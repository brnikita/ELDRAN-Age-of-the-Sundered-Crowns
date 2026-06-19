# DEVELOPMENT SPECIFICATION — "KELDRAN: Age of the Sundered Crowns"

**Type:** Open-world MMORPG (PvE-first, optional PvP), realistic high-fantasy.
**Engine:** Unreal Engine 5.8 (C++ core + Blueprints + GAS).
**Builder:** Claude Code (Opus 4.8) driving the Unreal Editor via the Unreal MCP plugin.
**Visual target:** photorealistic / realistic-stylized; real skeletal characters with full animations (no voxel/low-poly look).
**AI content stack:** ElevenLabs (voice/SFX/music), OpenRouter (LLM dialogue), GPT Image 2 (2D art/icons/textures).
**Assets:** free/open sources only (MetaHuman, Mixamo, Quixel Megascans/Fab free tier, Poly Haven, ambientCG, Sketchfab CC0, Freesound).
**Originality constraint (hard rule for all generated/sourced content):** no Tolkien or Blizzard/WoW names, terms, iconography, or assets; all proper nouns and assets are original or licensed.

---

## 1. How to use this document
- **Phase 0 is the agent's first task:** expand this spec into detailed design docs + a task graph (§12) before writing gameplay code.
- **Source of truth during the build:** `Docs/05_TaskGraph.md` (backlog) + `Docs/PROGRESS.md` (ledger). The agent always picks the next unblocked task, implements it, builds, tests, commits, updates the ledger.
- **Completion is defined only by passing a milestone's Definition of Done (§14).**
- Ship order is strict: M0 → M1 → M2 (vertical slice) → onward. Art/audio generation may run in parallel.

---

## 2. World & locations
**World:** **Keldran**, a fractured continent. The **Seven-Throne Dominion** once united seven peoples under the **Crowns of Concord**. A cataclysm — the **Sundering** — shattered the Crowns into **Shards** and tore open **Rifts**, unstable portals into pocket-worlds. The **Hollow Legion**, led by the **Hollow King** (a former Crown-keeper hollowed by the Sundering), rises from the ruins. Players collect Shards; factions war over whether to rebuild the Dominion.

The Rifts justify many distinct biomes/worlds for visual variety and travel.

| # | Location | Type | Biome | Tier |
|---|----------|------|-------|------|
| L1 | **Vael's Rest** | Covenant start zone | temperate woodland | 1–10 |
| L2 | **Emberhold Flats** | Free Pact start zone | arid mesas | 1–10 |
| L3 | **Lumengarde** | Capital hub | grand stone city (neutral) | all |
| L4 | **The Whispering Mire** | Mid zone | bioluminescent swamp | 10–20 |
| L5 | **Haelfrost** | Rift-world | frozen tundra/caves | 20–30 |
| L6 | **Cindreach** | Rift-world | volcanic ash | 25–35 |
| L7 | **Verdance** | Rift-world | overgrown jungle ruins | 30–40 |
| L8 | **The Hollow** | Endgame Rift-world | void/spectral | 40+ |
| D1 | **The Sunken Vault of Crowns** | 5-player dungeon | flooded crypt | 15 |

Vertical slice = **L1 only**; later milestones add the rest.

---

## 3. Races & factions
**Races:** **Aelin** (long-lived mages), **Grunvaldi** (mountain artisans), **Bryn** (beastkin), **Freeborn** (humans), **Ashborn** (redemption-seeking former Legion captives), **Claykin** (small folk/constructs).
**Factions:** **Covenant of the Seven Crowns** (order/restoration) vs **Free Pact of Ash** (frontier freedom). Shared PvE threat: the **Hollow Legion**.

---

## 4. Tech stack & architecture
- **Runtime:** UE 5.8, C++ core + Blueprints; **Gameplay Ability System (GAS)** for combat/abilities/attributes; **Enhanced Input**; **StateTree/Behavior Trees** for AI; **PCG + World Partition** for large streamed worlds; **MetaSounds** for audio.
- **Networking:** UE **Dedicated Server** builds; server-authoritative; client prediction for movement; **Replication Graph** for relevancy scaling. One process per zone; later, sharding via a World Service.
- **Backend (outside UE), packaged in `docker-compose`:**
  - **Login/Account Gateway** (HTTP/gRPC) → issues session token + shard address.
  - **PostgreSQL** — accounts, characters, inventory, progression.
  - **Redis** — sessions, presence, caches.
  - **NPC Dialogue Service** → OpenRouter (see §7).
- **Full target topology:**
```
[UE Client] --HTTPS--> [Login/Account Gateway] --> [PostgreSQL]
     | token + shard addr
     v
[Dedicated Game Server (zone shard)] <--replication--> [Clients]
     |--> [Char DB / Redis]
     |--> [NPC Dialogue Service] --> [OpenRouter]
[World Service] coordinates zone handoff & sharding
```
- **MVP topology:** one dedicated server + one gateway + one Postgres + one Redis; no sharding.

---

## 5. Required Unreal plugins & agentic-dev toolchain
Enable in the `.uproject`. These give the agent control of the editor, scriptable automation, and headless build/test.

**Agent control & editor automation**
- **Model Context Protocol (Unreal MCP)** — embeds an MCP server in the editor (binds `http://127.0.0.1:8000/mcp`); lets Claude Code spawn actors, build Blueprints, edit materials, author UMG, run automation tests. Enable **Auto Start Server**.
- **Toolset Registry** — dependency of Unreal MCP (auto-enabled).
- **Python Editor Script Plugin** — run Python in the editor and headless **commandlet** mode (asset import/creation, batch ops); backbone of the generation/asset pipeline.
- **Editor Scripting Utilities** — exposes editor operations to Python/Blueprint automation.
- **Remote Control API** — HTTP/WebSocket control surface; used by community MCP servers (e.g. `mcp-unreal`) for headless builds/tests if added alongside the official plugin.

**Testing**
- **Gauntlet** — orchestrates full sessions, including multiplayer (server + N clients), and validates results; add a `UGauntletTestController` to puppeteer instances. (Gauntlet runs a build; it does not build it.)
- **Functional Testing Editor** — author Blueprint functional/map tests.

**Gameplay**
- **Gameplay Abilities (GAS)**, **GameplayTags**, **GameplayTasks** — combat/abilities/attributes.
- **Enhanced Input** — input mapping.
- **World Partition + PCG** (built-in) — large worlds & procedural population.

**Content**
- **MetaHuman** plugin + **Quixel Bridge / Fab** integration — realistic characters and photoreal assets.
- **Online Subsystem** (+ chosen backend implementation) — sessions/login glue.

> Recommendation: use the **official Unreal MCP** plugin as the primary control surface and **`mcp-unreal`** (via Remote Control API) as a secondary headless build/test server, so the agent can build, run automation tests, and run Gauntlet without a human in the editor.

---

## 6. Automated testing strategy (how the agent verifies its own output)
Three layers, all runnable headless so they fit the autonomous loop:

**(a) Unit / automation tests** — C++ tests via `IMPLEMENT_SIMPLE_AUTOMATION_TEST` / Automation Spec, plus Blueprint **Functional Tests** in maps prefixed `FTEST_`. Cover: ability math (damage/cost/cooldown), inventory rules, loot-table math, and **data-integrity tests** that assert every DataTable row references valid assets (no missing icon/ability/mesh).

**(b) Functional / integration tests** — load a test map, spawn actors, drive latent commands, assert outcomes: an ability applies the right GameplayEffect; a mob dies and drops loot per table; a quest objective increments and turns in; equipment changes attributes.

**(c) Gauntlet multiplayer sessions** — spin up a dedicated server + N clients; a `UGauntletTestController` puppeteers clients to connect, move, attack, loot, and relog; assert replication correctness, persistence (progress survives relog), and capture performance via the CSV profiler.

**Headless commands the agent uses in its loop**
```
# Run automation/functional tests, no GPU needed (-nullrhi), exit when queue empties:
UnrealEditor-Cmd <Project>.uproject -game -unattended -nullrhi -nosound -nosplash -stdout \
  -ExecCmds="Automation RunTests <Filter>;quit" -TestExit="Automation Test Queue Empty"

# Build + cook + run tests via the Automation Tool:
RunUAT BuildCookRun -project=<Project>.uproject -noP4 -build -cook -stage ...

# Multiplayer test pass on a cooked build:
RunUAT RunGauntlet -project=<Project>.uproject -build=<CookedBuildPath> -test=<GauntletTest>
```

**CI (self-hosted runner, per commit):** compile → automation + functional tests → Gauntlet smoke (server + 2 clients) → publish report. Treat a red test as a blocking failure in the autonomous loop (§13).

---

## 7. AI generation pipeline
Two layers: **build-time generation** (scripts bake assets into `/Content`, cached by content hash, idempotent via `make generate`) and **runtime services** (live NPC dialogue). Keys come from env/secrets, never hardcoded. Every generated asset is recorded in `Docs/04_AssetManifest.md`.

**GPT Image 2 — 2D art.** Model id `gpt-image-2` (pin snapshot `gpt-image-2-2026-04-21`); endpoints `v1/images/generations`, `v1/images/edits`, `v1/responses`; up to ~4K, strong text rendering. Use for ability/item/status icons, UI frames & decals, loading screens, map/cartography art, concept art, and tileable 2D textures/decals. **2D only — not for 3D meshes.** Script: `Tools/gen/gen_images.py` → `/Content/UI/Icons`, `/Content/Textures/...`.

**ElevenLabs — audio.** REST + Python/TS SDK: TTS (10k+ voices), **Text to Dialogue** (multi-speaker scenes), **Sound Effects** (`eleven_text_to_sound_v2`), **Eleven Music v2** (text-to-music, section-by-section, mid-track genre transitions; cleared for commercial/gaming). Use for NPC VO & barks, cinematic dialogue, combat/ability SFX, ambient loops, UI sounds, and per-zone adaptive soundtracks + combat music. Script: `Tools/gen/gen_audio.py` → `/Content/Audio/{VO,SFX,Music}` → wire into MetaSounds.

**OpenRouter — LLM dialogue.** (a) Build-time generation of quest text, lore, vendor flavor, bark pools; (b) optional runtime "living NPC" chat via the **NPC Dialogue Service**, which enforces a per-NPC persona/system prompt, caches responses, rate-limits, applies guardrails, injects compact world-bible context, and **falls back to authored lines** on timeout/quota. Default to a free OpenRouter model for ambient NPCs. Never call the LLM from the game client. Scripts: `Tools/gen/gen_text.py` (build-time) + `Backend/dialogue-service/` (runtime).

**Governance:** manifests in `/Docs/gen/`; outputs hashed; a configurable spend/quota cap; generators skip already-cached items.

---

## 8. Asset acquisition (free/open sources)
Realistic skeletal characters + photoreal environments from free libraries. Track a license for **every** asset in `Docs/04_AssetManifest.md`; build a credits screen.

| Need | Source | License | Notes |
|------|--------|---------|-------|
| Realistic humanoid characters | **MetaHuman** | Free for UE projects | Drives the playable races |
| Character animations | **Adobe Mixamo** | Free, royalty-free | Auto-rig + library; retarget to UE5/MetaHuman skeleton |
| Photoreal environments/props/surfaces/vegetation | **Quixel Megascans on Fab** | Fab free tier + legacy library (Epic Content License) | Use the free subset; claimed assets are yours permanently |
| CC0 HDRIs/textures/models | **Poly Haven** | CC0 | Lighting, PBR materials, props |
| CC0 PBR materials | **ambientCG** | CC0 | Tileable surfaces |
| CC0/downloadable 3D | **Sketchfab** (filter CC0) | CC0 / CC-BY (per asset) | Fill gaps |
| Free SFX | **Freesound** | CC (per asset) | Supplement ElevenLabs SFX |
| Free music | **Incompetech** | CC-BY | Supplement ElevenLabs Music |

**Coherence pass:** unify scale, material setup, and a shared post-process/lighting profile so mixed-source assets read as one world.

---

## 9. Project structure
```
Keldran/
  Source/
    KeldranCore/        # GameInstance, subsystems, base types
    KeldranCharacter/   # pawns, controllers, movement, MetaHuman setup
    KeldranAbilities/   # GAS: abilities, attribute sets, effects
    KeldranInventory/   # items, inventory, equipment
    KeldranQuests/      # quests, dialogue, journal
    KeldranAI/          # mobs, StateTree/BT, perception
    KeldranNet/         # replication, gateway client, sessions, handoff
    KeldranUI/          # UMG/Slate, HUD, windows
    KeldranServer/      # server logic, validation, persistence
    KeldranTests/       # automation/functional tests + Gauntlet controllers
  Content/
    Maps/  ( + FTEST_*  test maps )
    Characters/ Abilities/ Audio/{VO,SFX,Music}/ Textures/ UI/ Data/
  Config/               # Default*.ini
  Tools/gen/            # gen_images.py, gen_audio.py, gen_text.py
  Backend/              # gateway, dialogue-service, db migrations, docker-compose
  Docs/                 # Phase 0 outputs (00..06), PROGRESS.md, gen/ manifests
  CI/                   # pipeline scripts (build, test, gauntlet)
  CLAUDE.md             # agent instructions
```
**Data-driven:** items/abilities/mobs/quests/loot live in **DataTables/DataAssets**, not code.

---

## 10. Gameplay systems
**MVP core:** character create (race/class/appearance) + DB persistence; third-person camera; server-authoritative movement with prediction; **GAS combat** (tab-target; attributes Health/Mana/Stamina/STR/AGI/INT/Armor; abilities, GCD, cooldowns, GameplayEffects for buffs/debuffs); targeting + nameplates; inventory & equipment affecting attributes; loot tables; quests (kill/collect/reach) + journal + rewards; NPCs, dialogue, vendors; HUD (resources, action bar, minimap, journal, inventory, chat); zone chat.

**Phase 2:** parties (5), group loot/XP, looking-for-group; guilds + chat/bank; leveling, talents/specs; multiple classes; travel points.
**Phase 3:** 5-player instanced dungeons; gathering + crafting; player economy (Shards as high-tier currency).
**Phase 4+:** raids (10–20); reputations; seasons; world bosses; optional PvP (duels, battlegrounds, arenas); auction house.

---

## 11. MCP workflow (CLAUDE.md must instruct the agent to)
1. Check editor connection/`status` before editor ops.
2. Consult `lookup_class`/`lookup_docs` before writing UE C++/Blueprint.
3. After C++ edits: `build_project` → run automation tests (§6) → read errors carefully.
4. Respect UE object-path formats.
5. Keep everything data-driven.
6. Work in small steps — one module/feature per iteration, each compiled, tested, committed.

---

## 12. Phase 0 (M0) — agent's first deliverable: the detailed spec (no code yet)
Produce under `/Docs`:
- `00_ImplementationSpec.md` — every system in §10 as concrete UE classes (C++ names + parents), schemas, replication rules, Blueprint responsibilities.
- `01_DataSchemas.md` — full DataTable/DataAsset row structures (items, abilities, mobs, quests, loot, NPCs, vendors, zones).
- `02_NetworkDesign.md` — server-authority matrix, RPC list, persistence model, login/handoff flow.
- `03_AIContentPipeline.md` — generation scripts, prompts, model ids, caching/versioning, output paths.
- `04_AssetManifest.md` — every external asset: source, license, target path.
- `05_TaskGraph.md` — dependency-ordered backlog; each task has a Definition of Done.
- `06_WorldBible.md` — expanded lore, zone-by-zone design, quest outlines, NPC roster.
- `07_TestPlan.md` — the automation/functional/Gauntlet tests per milestone and their pass criteria.

**M0 exit:** all docs exist, are consistent, and `05_TaskGraph.md` has no blocking TBD for the vertical slice.

---

## 13. Autonomous run protocol
1. Pick the next unblocked task from `Docs/05_TaskGraph.md`.
2. **Per-task loop:** implement → `build_project` → run automation/functional tests (§6) → if green, `git commit` (task id) → append to `Docs/PROGRESS.md` → next task.
3. **On failure:** capture error; attempt up to N fixes using `lookup_docs`; if still red, mark `BLOCKED` with notes and move on (no infinite loops).
4. **At milestone boundaries:** run the milestone's Definition of Done incl. its Gauntlet pass; tag a git checkpoint if green.
5. **Stop & summarize** when: a milestone's DoD passes and the next needs a human prerequisite; all tasks are `DONE`/`BLOCKED`; or a spend/quota cap is hit. Always write a "resume here" note.

**`Docs/PROGRESS.md` entry format**
```
## <ISO timestamp> | <task id> | <DONE|BLOCKED|WIP>
- change:
- build: <pass/fail>   tests: <pass/fail>   gauntlet: <pass/fail/n-a>
- commit: <hash>
- notes/blockers:
- resume here: <next task id>
```

---

## 14. Milestones & Definitions of Done
| Milestone | Scope | Definition of Done (testable) |
|-----------|-------|-------------------------------|
| **M0 Spec** | §12 docs | All docs exist & consistent; TaskGraph has no blocking TBD |
| **M1 Skeleton** | project, modules, dedicated server, backend (`docker-compose up`), MCP, CLAUDE.md, CI, test harness | Empty client+server connect; backend online; build + automation tests green; CI runs |
| **M2 Vertical Slice** | L1, 1 race + 1 class, GAS combat, 2–3 mobs w/ AI, inventory+equip+loot, 3–5 quests, 1 vendor, HUD, persistence | **Gauntlet:** 2 clients join, see each other, kill mobs, take/turn-in quests, loot & equip, relog with progress saved — all asserted automatically |
| **M3 Content Pipeline** | §7 generators + §8 acquisition wired; MetaHuman+Mixamo character; zone audio | `make generate` adds a new icon/SFX/VO/music + an animated MetaHuman in-game |
| **M4 Social & Progress** | parties, guilds, leveling, talents, L2–L3 | Group quests + leveling + capital hub pass functional + Gauntlet tests |
| **M5 Worlds** | L5–L8 + D1 | ≥2 rift-worlds + the 5-player dungeon playable and tested |
| **M6 Endgame** | raids, reputations, optional PvP, AH | One raid + reputation + one PvP mode pass tests |
| **M7 Hardening** | sharding, anti-cheat, DevOps | Survives a scripted Gauntlet load test across ≥2 shards |

---

## 15. One-time setup prerequisites (do before the unattended run)
- Install UE 5.8 + C++ toolchain; enable all plugins in §5.
- Accounts/logins: Epic (MetaHuman/Fab/Megascans), Adobe (Mixamo).
- API keys in env/secrets: OpenAI (GPT Image 2), ElevenLabs, OpenRouter; set spend caps.
- Provision Docker (Postgres/Redis/services); set up the self-hosted CI runner.
- Record all asset/model licenses.

---

## 16. Technical risk checklist
- Networking + sharding is the hardest part — never trust the client; budget extra time; cover with Gauntlet tests early.
- Runtime LLM/TTS calls cost money + add latency — cache, pre-generate, prefer free models, always have authored fallbacks.
- Mixed free assets can look like an asset flip — enforce the §8 coherence pass (scale/material/lighting).
- Gauntlet needs cooked builds — keep the build step fast and reliable so the test loop stays tight.

---

*Start at M0 (write the detailed spec), then M1 → M2. Treat `Docs/05_TaskGraph.md` + `Docs/PROGRESS.md` as the run's memory; a milestone is done only when its tests pass.*
