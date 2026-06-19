"""Build-time text generation via OpenRouter (quest text, lore, bark pools).

Outputs JSON under Content/Data/Generated/Text/<id>.json for later DataTable import.
Free models cost $0; cached + originality-linted. See Docs/03_AIContentPipeline.md §3.
"""
from __future__ import annotations

import json
import sys
from pathlib import Path

import httpx
import yaml

import common as c

MODEL = __import__("os").environ.get("OPENROUTER_MODEL", "google/gemma-4-31b-it:free")
URL = "https://openrouter.ai/api/v1/chat/completions"
OUT_DIR = c.REPO_ROOT / "Content" / "Data" / "Generated" / "Text"


def _specs() -> list[dict]:
    items = []
    for f in sorted((c.PROMPTS_DIR / "text").glob("*.yaml")):
        doc = yaml.safe_load(f.read_text(encoding="utf-8"))
        for it in doc.get("items", []):
            it["_version"] = str(doc.get("version", "1"))
            items.append(it)
    return items


def run() -> int:
    c.banner("gen_text (OpenRouter)")
    c.ensure_dirs()
    key = None if c.DRYRUN else c.require_key("OPENROUTER_API_KEY")
    made = cached = 0
    for it in _specs():
        prompt = it["prompt"].strip()
        c.lint_originality(prompt + " " + it.get("system", ""), f"text:{it['id']}")
        ck = c.cache_key("text", MODEL, it["_version"], prompt, {"id": it["id"]})
        if c.is_cached(ck, "json"):
            c.log(f"cached  {it['id']}")
            cached += 1
            continue
        if c.DRYRUN:
            c.log(f"PLAN    {it['id']} (model {MODEL}, est $0.00 free)")
            c.reserve_spend(0.0, it["id"])
            continue
        c.reserve_spend(0.0, it["id"])  # free model
        messages = []
        if it.get("system"):
            messages.append({"role": "system", "content": it["system"].strip()})
        messages.append({"role": "user", "content": prompt})
        try:
            r = httpx.post(
                URL, headers={"Authorization": f"Bearer {key}"},
                json={"model": MODEL, "messages": messages, "max_tokens": 400},
                timeout=40,
            )
            r.raise_for_status()
            content = r.json()["choices"][0]["message"]["content"]
        except Exception as e:  # noqa: BLE001
            c.log(f"ERROR   {it['id']}: {e}")
            return 1
        c.lint_originality(content, f"text-output:{it['id']}")
        c.cached_path(ck, "json").write_text(content, encoding="utf-8")
        OUT_DIR.mkdir(parents=True, exist_ok=True)
        (OUT_DIR / f"{it['id']}.json").write_text(content, encoding="utf-8")
        c.manifest_append({
            "type": "text", "id": it["id"], "model": MODEL, "hash": ck,
            "out": f"Content/Data/Generated/Text/{it['id']}.json", "license": "owned-generated",
        })
        c.log(f"made    {it['id']}")
        made += 1
    c.log(f"done: {made} generated, {cached} cached, spend ${c.spent():.2f}")
    return 0


if __name__ == "__main__":
    sys.exit(run())
