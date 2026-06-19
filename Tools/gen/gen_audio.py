"""Audio generation via ElevenLabs (TTS now; SFX/Music specs supported via `kind`).

Outputs mp3 to the spec's `out` path under Content/Audio/ and caches by content hash.
See Docs/03_AIContentPipeline.md §2.
"""
from __future__ import annotations

import os
import sys

import httpx
import yaml

import common as c

TTS_MODEL = os.environ.get("ELEVEN_TTS_MODEL", "eleven_multilingual_v2")
SFX_URL = "https://api.elevenlabs.io/v1/sound-generation"
VOICES_URL = "https://api.elevenlabs.io/v1/voices"
# Rough cost estimates for the spend cap.
COST_TTS = float(os.environ.get("GEN_TTS_COST_USD", "0.01"))
COST_SFX = float(os.environ.get("GEN_SFX_COST_USD", "0.02"))
# Default premade ElevenLabs voice (George) — avoids needing the voices_read key permission.
DEFAULT_VOICE = os.environ.get("GEN_DEFAULT_VOICE_ID", "JBFqnCBsd6RMkjVDRZzb")


def _specs() -> list[dict]:
    items = []
    for f in sorted((c.PROMPTS_DIR / "audio").glob("*.yaml")):
        doc = yaml.safe_load(f.read_text(encoding="utf-8"))
        for it in doc.get("items", []):
            it["_version"] = str(doc.get("version", "1"))
            items.append(it)
    return items


def _first_voice(key: str) -> str:
    r = httpx.get(VOICES_URL, headers={"xi-api-key": key}, timeout=30)
    r.raise_for_status()
    voices = r.json().get("voices", [])
    if not voices:
        sys.exit("[gen] ElevenLabs returned no voices.")
    return voices[0]["voice_id"]


def _write(path, data: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(data)


def run() -> int:
    c.banner("gen_audio (ElevenLabs)")
    c.ensure_dirs()
    key = None if c.DRYRUN else c.require_key("ELEVENLABS_API_KEY")
    made = cached = 0
    voice_cache: str | None = None
    for it in _specs():
        kind = it.get("kind", "tts")
        text = (it.get("text") or it.get("prompt") or "").strip()
        c.lint_originality(text, f"audio:{it['id']}")
        cost = COST_TTS if kind == "tts" else COST_SFX
        ck = c.cache_key("audio", TTS_MODEL if kind == "tts" else "sfx", it["_version"], text,
                         {"id": it["id"], "kind": kind, "voice": it.get("voice", "auto")})
        out_path = c.REPO_ROOT / it["out"]
        if c.is_cached(ck, "mp3"):
            _write(out_path, c.cached_path(ck, "mp3").read_bytes())
            c.log(f"cached  {it['id']} -> {it['out']}")
            cached += 1
            continue
        if c.DRYRUN:
            c.log(f"PLAN    {it['id']} ({kind}, est ${cost:.2f}) -> {it['out']}")
            c.reserve_spend(cost, it["id"])
            continue
        c.reserve_spend(cost, it["id"])
        try:
            if kind == "tts":
                # Prefer explicit/default voice to avoid the voices_read permission;
                # only list voices if no default is configured.
                voice = it.get("voice") or DEFAULT_VOICE or voice_cache or _first_voice(key)
                voice_cache = voice
                r = httpx.post(
                    f"https://api.elevenlabs.io/v1/text-to-speech/{voice}",
                    headers={"xi-api-key": key, "Content-Type": "application/json"},
                    json={"text": text, "model_id": TTS_MODEL},
                    timeout=120,
                )
            else:  # sfx
                r = httpx.post(
                    SFX_URL, headers={"xi-api-key": key, "Content-Type": "application/json"},
                    json={"text": text}, timeout=120,
                )
            r.raise_for_status()
            data = r.content
        except Exception as e:  # noqa: BLE001
            body = getattr(getattr(e, "response", None), "text", "")
            c.log(f"ERROR   {it['id']}: {e} {body[:300]}")
            return 1
        c.cached_path(ck, "mp3").write_bytes(data)
        _write(out_path, data)
        c.manifest_append({
            "type": "audio", "id": it["id"], "kind": kind, "model": TTS_MODEL, "hash": ck,
            "out": it["out"], "license": "owned-generated",
        })
        c.log(f"made    {it['id']} -> {it['out']}")
        made += 1
    c.log(f"done: {made} generated, {cached} cached, spend ${c.spent():.2f}")
    return 0


if __name__ == "__main__":
    sys.exit(run())
