"""Shared infrastructure for the KELDRAN build-time generators (Docs/03_AIContentPipeline.md).

Responsibilities:
- Load secrets from the gitignored root .env (single source of truth).
- Content-hash caching: skip API calls when an identical artifact already exists.
- Spend cap: estimate cost before each call; abort the run before exceeding GEN_SPEND_CAP_USD.
- Originality lint: reject prompts containing banned (Tolkien/Blizzard/WoW) terms.
- Manifest: append every produced artifact to Docs/gen/manifest-<date>.jsonl.
- Dry run (DRYRUN=1): print planned calls + cost estimate, make no API calls, write nothing.
"""
from __future__ import annotations

import hashlib
import json
import os
import sys
from datetime import date
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
GEN_DIR = REPO_ROOT / "Docs" / "gen"
CACHE_DIR = GEN_DIR / "cache"
PROMPTS_DIR = Path(__file__).resolve().parent / "prompts"

DRYRUN = os.environ.get("DRYRUN", "0") == "1"

# Banned terms — keep generated content original (Docs/06_WorldBible.md §9).
BANNED_TERMS = [
    "azeroth", "middle-earth", "middle earth", "mordor", "gandalf", "frodo", "sauron",
    "hobbit", "warcraft", "stormwind", "orgrimmar", "azshara", "lich king", "arthas",
    "thrall", "sylvanas", "silvermoon", "elf of rivendell", "rivendell", "gondor",
    "isengard", "shire", "nazgul", "balrog", "uruk-hai", "uruk hai",
]


def _load_dotenv() -> None:
    env_path = REPO_ROOT / ".env"
    if not env_path.exists():
        return
    for line in env_path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue
        k, _, v = line.partition("=")
        os.environ.setdefault(k.strip(), v.strip())


_load_dotenv()

SPEND_CAP = float(os.environ.get("GEN_SPEND_CAP_USD", "10"))
_spent = 0.0


def require_key(name: str) -> str:
    val = os.environ.get(name, "").strip()
    if not val:
        sys.exit(f"[gen] ERROR: {name} not set (add it to the root .env). Aborting.")
    return val


class SpendExceeded(RuntimeError):
    pass


def reserve_spend(usd: float, label: str) -> None:
    """Account for an upcoming call; abort the whole run if it would exceed the cap."""
    global _spent
    if _spent + usd > SPEND_CAP:
        raise SpendExceeded(
            f"[gen] spend cap ${SPEND_CAP:.2f} would be exceeded by '{label}' "
            f"(spent ${_spent:.2f} + ${usd:.2f}). Aborting."
        )
    _spent += usd


def spent() -> float:
    return _spent


def lint_originality(text: str, where: str) -> None:
    low = text.lower()
    for term in BANNED_TERMS:
        if term in low:
            sys.exit(f"[gen] ORIGINALITY VIOLATION in {where}: banned term '{term}'. Aborting.")


def cache_key(generator: str, model: str, version: str, prompt: str, params: dict) -> str:
    blob = json.dumps(
        {"g": generator, "m": model, "v": version, "p": prompt, "x": params},
        sort_keys=True,
    )
    return hashlib.sha256(blob.encode()).hexdigest()[:32]


def cached_path(key: str, ext: str) -> Path:
    return CACHE_DIR / f"{key}.{ext}"


def is_cached(key: str, ext: str) -> bool:
    return cached_path(key, ext).exists()


def manifest_append(record: dict) -> None:
    if DRYRUN:
        return
    GEN_DIR.mkdir(parents=True, exist_ok=True)
    path = GEN_DIR / f"manifest-{date.today().isoformat()}.jsonl"
    with path.open("a", encoding="utf-8") as f:
        f.write(json.dumps(record) + "\n")


def ensure_dirs() -> None:
    if not DRYRUN:
        CACHE_DIR.mkdir(parents=True, exist_ok=True)


def log(msg: str) -> None:
    print(f"[gen] {msg}", flush=True)


def banner(generator: str) -> None:
    mode = "DRY RUN (no API calls)" if DRYRUN else "LIVE"
    log(f"=== {generator} | {mode} | spend cap ${SPEND_CAP:.2f} ===")
