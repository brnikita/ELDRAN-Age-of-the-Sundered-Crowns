# 03 — AI Content Pipeline

> Two layers: **build-time generation** (scripts bake assets into `/Content`, cached by content
> hash, idempotent via `make generate`) and **runtime services** (live NPC dialogue). Keys come
> from env/secrets, **never hardcoded**. Every generated asset is recorded in
> `04_AssetManifest.md` and a per-run manifest in `Docs/gen/`. A configurable spend/quota cap is
> enforced; generators **skip already-cached items**.

## 0. Safety / cost rules (apply to every generator)
- Read keys from env only: `OPENAI_API_KEY`, `ELEVENLABS_API_KEY`, `OPENROUTER_API_KEY`.
- **Spend cap:** each generator reads `GEN_SPEND_CAP_USD` (and provider sub-caps); estimates
  cost before a call; aborts the run with a clear message if the cap would be exceeded.
- **Idempotent caching:** the cache key is a SHA-256 of `(generator, model, version, prompt,
  params)`. If `Docs/gen/cache/<hash>` exists, skip the API call and reuse the artifact.
- **Manifest:** every produced asset appends a row to `Docs/gen/manifest-<date>.jsonl` and is
  reflected in `04_AssetManifest.md` (source=generated, license=owned-generated, path, hash).
- **Originality guard:** prompts must follow the `06_WorldBible.md` §9 style guide; a prompt
  lint step rejects banned terms (Tolkien/Blizzard/WoW) before any call.
- **Dry run:** `make generate DRYRUN=1` prints planned calls + cost estimate, makes no calls.
- Never call any of these from the **game client**. Build-time = developer machine/CI; runtime
  dialogue = dedicated server only.

## 1. GPT Image 2 — 2D art (`Tools/gen/gen_images.py`)
- **Model id:** `gpt-image-2` (pin snapshot `gpt-image-2-2026-04-21`).
- **Endpoints:** `v1/images/generations`, `v1/images/edits`, `v1/responses`. Up to ~4K; strong
  text rendering.
- **Use for (2D ONLY — never 3D meshes):** ability/item/status icons, UI frames & decals,
  loading screens, map/cartography art, concept art, tileable 2D textures/decals.
- **Outputs:** `Content/UI/Icons/`, `Content/UI/Frames/`, `Content/Textures/<set>/`,
  `Content/UI/Loading/`. PNGs imported as `UTexture2D` via the Python Editor commandlet (§5).
- **Prompt spec lives in** `Tools/gen/prompts/images/*.yaml` (one entry per asset: id, prompt,
  size, transparency, style tags). Icons share a style preamble for visual coherence.
- **Slice asset list:** icons for the 3 Warden abilities, the slice items (worn-shortsword,
  worn-shield, potion, blue weapon, blue accessory, quest items), buff/debuff icons
  (stun, defensive-stance), a Vael's Rest loading screen, basic UI frame set.

## 2. ElevenLabs — audio (`Tools/gen/gen_audio.py`)
- **APIs:** REST + Python SDK. TTS, **Text to Dialogue** (multi-speaker), **Sound Effects**
  (`eleven_text_to_sound_v2`), **Eleven Music v2** (text-to-music; cleared for commercial/gaming).
- **Use for:** NPC VO & barks, cinematic dialogue, combat/ability SFX, ambient loops, UI sounds,
  per-zone adaptive soundtrack + combat music.
- **Outputs:** `Content/Audio/VO/`, `Content/Audio/SFX/`, `Content/Audio/Music/`; imported as
  `USoundWave` and wired into **MetaSounds** (`Content/Audio/Meta/`).
- **Prompt spec:** `Tools/gen/prompts/audio/*.yaml` (id, type tts|sfx|music|dialogue, voice id,
  text/prompt, loop flag, duration).
- **Slice asset list:** VO/barks for the 4 NPCs + 4 creatures (aggro/death/idle/greeting),
  ability SFX (basic attack, shield bash, defensive stance), ambient woodland loop + a "near the
  Rift" tense layer, light combat music for Vael's Rest, a handful of UI sounds.

## 3. OpenRouter — LLM dialogue & text (`Tools/gen/gen_text.py` + runtime service)
**(a) Build-time** (`gen_text.py`): generate quest text, lore snippets, vendor flavor, and
**bark pools** (`DT_Barks`). Outputs JSON → imported into DataTables. Cached + manifested.
Always run through the originality lint. Default to a capable-but-cheap model; pin the model id
in the prompt spec.

**(b) Runtime "living NPC"** (`Backend/dialogue-service/`, see `02_NetworkDesign.md` §6):
- Default **free OpenRouter model** for ambient NPCs (slice: only Doran Vale is live).
- Enforces per-NPC persona/system prompt, injects a compact world-bible context block, caches
  responses (Redis), rate-limits per account, applies guardrails, and **falls back to authored
  lines** on timeout/quota/error.
- Prompt spec: `Backend/dialogue-service/personas/*.yaml`.

## 4. Free-asset acquisition helpers (3D & supplements)
GPT Image 2 cannot make 3D meshes. 3D/realistic assets come from the free libraries in
`08`/`04_AssetManifest.md` (MetaHuman, Mixamo, Quixel/Fab, Poly Haven, ambientCG, Sketchfab
CC0, Freesound, Incompetech). Acquisition is partly manual (logins/EULA) + scripted import:
- `Tools/gen/import_assets.py` — batch-imports downloaded source files into `/Content`, sets up
  materials at a unified scale, and records license metadata into `04_AssetManifest.md`.
- MetaHuman + Mixamo retarget is editor-driven (MCP/Python) in M3; tracked as M3 tasks.

## 5. Execution mechanics
- **Driver:** `make generate` runs the three generators in order (text → images → audio), then
  the importer (`Python Editor Script Plugin` commandlet) to bring artifacts into `/Content`:
  ```
  UnrealEditor-Cmd Keldran.uproject -run=pythonscript -script="Tools/gen/import_runner.py" \
    -unattended -nullrhi -nosound -stdout
  ```
- **Idempotency check:** `make generate` re-run with no spec changes makes **zero** API calls
  and changes no files (verified by an M3 functional check).
- **CI:** generation is **not** run on every commit (cost); it runs on demand or on a dedicated
  "content" job. Generated assets are committed (or stored via Git LFS — decided in M1 task
  `m1-lfs`) so builds are reproducible without re-calling APIs.
- **Versioning:** bumping a generator's `version` field invalidates its cache slice and forces
  regeneration of just those assets.

## 6. Slice (M2) "wired" definition
For the slice, AI assets may start as **placeholders** and be replaced by generated assets in
M3. The M2 DoD does not require final art/audio — it requires the *systems* to consume
data-driven asset references (which resolve to placeholders or generated assets identically).
The M3 DoD is the first time `make generate` must add a real new icon/SFX/VO/music + an animated
MetaHuman in-game.
