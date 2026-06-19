"""2D art generation via GPT Image 2 (OpenAI). Icons/frames/textures only — never 3D.

Outputs PNG to the spec's `out` path under Content/ and caches by content hash.
See Docs/03_AIContentPipeline.md §1.
"""
from __future__ import annotations

import base64
import os
import sys

import httpx
import yaml

import common as c

MODEL = os.environ.get("GEN_IMAGE_MODEL", "gpt-image-2")
URL = "https://api.openai.com/v1/images/generations"
# Rough per-image cost estimate for the spend cap (override with GEN_IMAGE_COST_USD).
COST_PER_IMAGE = float(os.environ.get("GEN_IMAGE_COST_USD", "0.04"))


def _specs() -> list[dict]:
    items = []
    for f in sorted((c.PROMPTS_DIR / "images").glob("*.yaml")):
        doc = yaml.safe_load(f.read_text(encoding="utf-8"))
        preamble = (doc.get("style_preamble") or "").strip()
        for it in doc.get("items", []):
            it["_version"] = str(doc.get("version", "1"))
            it["_preamble"] = preamble
            items.append(it)
    return items


def _write_png(path, data: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(data)


def run() -> int:
    c.banner("gen_images (GPT Image 2)")
    c.ensure_dirs()
    key = None if c.DRYRUN else c.require_key("OPENAI_API_KEY")
    made = cached = 0
    for it in _specs():
        full_prompt = (it["_preamble"] + "\n" + it["prompt"].strip()).strip()
        c.lint_originality(full_prompt, f"image:{it['id']}")
        size = it.get("size", "1024x1024")
        ck = c.cache_key("image", MODEL, it["_version"], full_prompt, {"size": size})
        out_path = c.REPO_ROOT / it["out"]
        if c.is_cached(ck, "png"):
            _write_png(out_path, c.cached_path(ck, "png").read_bytes())
            c.log(f"cached  {it['id']} -> {it['out']}")
            cached += 1
            continue
        if c.DRYRUN:
            c.log(f"PLAN    {it['id']} ({MODEL} {size}, est ${COST_PER_IMAGE:.2f}) -> {it['out']}")
            c.reserve_spend(COST_PER_IMAGE, it["id"])
            continue
        c.reserve_spend(COST_PER_IMAGE, it["id"])
        try:
            r = httpx.post(
                URL, headers={"Authorization": f"Bearer {key}"},
                json={"model": MODEL, "prompt": full_prompt, "size": size, "n": 1},
                timeout=180,
            )
            r.raise_for_status()
            b64 = r.json()["data"][0]["b64_json"]
            data = base64.b64decode(b64)
        except Exception as e:  # noqa: BLE001
            body = getattr(getattr(e, "response", None), "text", "")
            c.log(f"ERROR   {it['id']}: {e} {body[:300]}")
            return 1
        c.cached_path(ck, "png").write_bytes(data)
        _write_png(out_path, data)
        c.manifest_append({
            "type": "image", "id": it["id"], "model": MODEL, "hash": ck,
            "out": it["out"], "license": "owned-generated",
        })
        c.log(f"made    {it['id']} -> {it['out']}")
        made += 1
    c.log(f"done: {made} generated, {cached} cached, spend ${c.spent():.2f}")
    return 0


if __name__ == "__main__":
    sys.exit(run())
