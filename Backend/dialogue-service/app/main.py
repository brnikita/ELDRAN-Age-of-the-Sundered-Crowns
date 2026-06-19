"""KELDRAN NPC Dialogue Service (Docs/02_NetworkDesign.md §6).

Called ONLY by the dedicated game server (never the client). Enforces per-NPC persona,
injects compact world context, caches responses in Redis, rate-limits per account, applies
basic guardrails, and ALWAYS falls back to an authored line on any failure.
"""
import hashlib
import os
import time

import httpx
import redis.asyncio as aioredis
from fastapi import FastAPI, HTTPException
from pydantic import BaseModel

from .personas import WORLD_CONTEXT, get_persona

OPENROUTER_API_KEY = os.environ.get("OPENROUTER_API_KEY", "").strip()
OPENROUTER_MODEL = os.environ.get("OPENROUTER_MODEL", "google/gemma-4-31b-it:free")
RATE_LIMIT = int(os.environ.get("DIALOGUE_RATE_LIMIT_PER_MIN", "20"))
OPENROUTER_URL = "https://openrouter.ai/api/v1/chat/completions"

app = FastAPI(title="KELDRAN Dialogue Service")
_redis: aioredis.Redis | None = None


@app.on_event("startup")
async def _startup():
    global _redis
    _redis = aioredis.from_url(os.environ["REDIS_URL"], decode_responses=True)


@app.on_event("shutdown")
async def _shutdown():
    if _redis:
        await _redis.aclose()


class DialogueReq(BaseModel):
    npc_id: str
    account_id: str = "anon"
    player_utterance: str
    context_hints: str = ""


def _cache_key(npc_id: str, utterance: str) -> str:
    norm = utterance.strip().lower()
    h = hashlib.sha256(f"{npc_id}|{norm}".encode()).hexdigest()[:24]
    return f"dlg:{h}"


async def _rate_limited(account_id: str) -> bool:
    window = int(time.time() // 60)
    key = f"dlgrate:{account_id}:{window}"
    n = await _redis.incr(key)
    if n == 1:
        await _redis.expire(key, 60)
    return n > RATE_LIMIT


def _guardrail(text: str) -> str:
    # Trim, cap length, strip accidental role prefixes.
    text = text.strip().strip('"')
    for p in ("Doran:", "Doran Vale:", "Assistant:"):
        if text.startswith(p):
            text = text[len(p):].strip()
    return text[:400]


@app.get("/health")
async def health():
    try:
        await _redis.ping()
    except Exception as e:  # noqa: BLE001
        raise HTTPException(503, f"redis down: {e}")
    return {"status": "ok", "llm_configured": bool(OPENROUTER_API_KEY)}


@app.post("/dialogue")
async def dialogue(req: DialogueReq):
    persona = get_persona(req.npc_id)
    if persona is None:
        raise HTTPException(404, f"no persona for {req.npc_id}")

    fallback = persona["fallback"][hash(req.player_utterance) % len(persona["fallback"])]

    # 1) cache
    ck = _cache_key(req.npc_id, req.player_utterance)
    cached = await _redis.get(ck)
    if cached:
        return {"line": cached, "source": "cache"}

    # 2) rate limit -> authored fallback (not an error; keeps NPC responsive)
    if await _rate_limited(req.account_id):
        return {"line": fallback, "source": "fallback_ratelimited"}

    # 3) no key -> authored fallback
    if not OPENROUTER_API_KEY:
        return {"line": fallback, "source": "fallback_nokey"}

    # 4) call OpenRouter, fall back on any failure
    try:
        messages = [
            {"role": "system", "content": f"{persona['system']}\n\n{WORLD_CONTEXT}"},
        ]
        if req.context_hints:
            messages.append({"role": "system", "content": f"Context: {req.context_hints}"})
        messages.append({"role": "user", "content": req.player_utterance})
        async with httpx.AsyncClient(timeout=8.0) as client:
            resp = await client.post(
                OPENROUTER_URL,
                headers={"Authorization": f"Bearer {OPENROUTER_API_KEY}"},
                json={"model": OPENROUTER_MODEL, "messages": messages, "max_tokens": 120},
            )
            resp.raise_for_status()
            line = _guardrail(resp.json()["choices"][0]["message"]["content"])
        if not line:
            return {"line": fallback, "source": "fallback_empty"}
        await _redis.set(ck, line, ex=86400)  # cache 24h
        return {"line": line, "source": "llm"}
    except Exception:  # noqa: BLE001 — any failure => authored fallback
        return {"line": fallback, "source": "fallback_error"}
