# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this project is

**KELDRAN: Age of the Sundered Crowns** — an open-world, PvE-first MMORPG built in **Unreal Engine 5.8** (C++ core + Blueprints + GAS). This repo is being built *by* an autonomous Claude Code agent driving the Unreal Editor through the **Unreal MCP plugin**, with AI-generated content (ElevenLabs audio, OpenRouter LLM dialogue, GPT Image 2 for 2D art) and free/open 3D assets (MetaHuman, Mixamo, Quixel/Fab, Poly Haven, ambientCG, Sketchfab CC0).

**Current state:** greenfield. The only file is `keldran_dev_spec_en.md` — the full development specification and the authoritative source for everything below. **Read it before doing substantive work.** The codebase, `/Docs`, and project structure do not exist yet; the first deliverable (M0) is to *create* them.

## The work model — read this first

This is not a normal "make a change" repo. It runs as an **autonomous milestone-driven build**:

1. **Source of truth during the build** is `Docs/05_TaskGraph.md` (dependency-ordered backlog, each task with a Definition of Done) plus `Docs/PROGRESS.md` (the ledger). These do not exist yet — M0 creates them.
2. **Per-task loop:** pick the next *unblocked* task → implement → `build_project` → run automation/functional tests → if green, `git commit` (with task id) → append an entry to `Docs/PROGRESS.md` → next task.
3. **On failure:** capture the error, attempt up to N fixes using `lookup_docs`; if still red, mark the task `BLOCKED` with notes and move on. **No infinite retry loops.**
4. **A milestone is "done" only when its Definition of Done passes** (spec §14), including its Gauntlet test where one is specified. Tag a git checkpoint when green.
5. **Stop & summarize** when a milestone's DoD passes and the next needs a human prerequisite, when all tasks are `DONE`/`BLOCKED`, or when a spend/quota cap is hit. Always leave a "resume here" note.

Ship order is strict: **M0 → M1 → M2 (vertical slice) → M3 …** (spec §14). Art/audio generation may run in parallel. The vertical slice is **L1 (Vael's Rest) only**.

`Docs/PROGRESS.md` entry format (spec §13):
```
## <ISO timestamp> | <task id> | <DONE|BLOCKED|WIP>
- change:
- build: <pass/fail>   tests: <pass/fail>   gauntlet: <pass/fail/n-a>
- commit: <hash>
- notes/blockers:
- resume here: <next task id>
```

## First task: M0 (no gameplay code yet)

The agent's first deliverable is to expand the spec into detailed design docs under `/Docs` (spec §12):
`00_ImplementationSpec.md`, `01_DataSchemas.md`, `02_NetworkDesign.md`, `03_AIContentPipeline.md`, `04_AssetManifest.md`, `05_TaskGraph.md`, `06_WorldBible.md`, `07_TestPlan.md`.
**M0 exit:** all docs exist, are mutually consistent, and `05_TaskGraph.md` has no blocking TBD for the vertical slice.

## MCP / Unreal Editor workflow (spec §11)

When driving the editor via Unreal MCP:
1. Check editor connection (`status`) before any editor operation.
2. Consult `lookup_class` / `lookup_docs` **before** writing UE C++ or Blueprint code.
3. After C++ edits: `build_project` → run automation tests → read errors carefully before proceeding.
4. Respect UE object-path formats.
5. Work in small steps — one module/feature per iteration, each compiled, tested, and committed.

## Headless build & test commands (spec §6)

All test layers are runnable headless to fit the autonomous loop:
```bash
# Run automation/functional tests, no GPU (-nullrhi), exit when the queue empties:
UnrealEditor-Cmd <Project>.uproject -game -unattended -nullrhi -nosound -nosplash -stdout \
  -ExecCmds="Automation RunTests <Filter>;quit" -TestExit="Automation Test Queue Empty"

# Build + cook + run via the Automation Tool:
RunUAT BuildCookRun -project=<Project>.uproject -noP4 -build -cook -stage ...

# Multiplayer (Gauntlet) pass on a cooked build:
RunUAT RunGauntlet -project=<Project>.uproject -build=<CookedBuildPath> -test=<GauntletTest>
```
Three test layers: (a) C++ automation + Blueprint **Functional Tests** in maps prefixed `FTEST_` (including data-integrity tests that assert every DataTable row references valid assets); (b) functional/integration tests on a loaded map; (c) **Gauntlet** multiplayer sessions (dedicated server + N clients puppeteered by a `UGauntletTestController`). **A red test is a blocking failure in the autonomous loop.**

## Architecture essentials

- **Engine runtime:** GAS for all combat/abilities/attributes; Enhanced Input; StateTree/Behavior Trees for AI; PCG + World Partition for streamed worlds; MetaSounds for audio.
- **Networking:** UE **Dedicated Server** builds, **server-authoritative**, client prediction for movement, Replication Graph for relevancy. One process per zone; sharding via a World Service comes later. MVP topology = one dedicated server + one gateway + one Postgres + one Redis, no sharding.
- **Backend (outside UE, in `docker-compose`):** Login/Account Gateway (issues session token + shard address) → PostgreSQL (accounts/characters/inventory/progression); Redis (sessions/presence/caches); NPC Dialogue Service → OpenRouter.
- **C++ modules** live under `Source/Keldran*` split by domain (Core, Character, Abilities, Inventory, Quests, AI, Net, UI, Server, Tests). See spec §9 for the full layout.

## Hard rules (do not violate)

- **Originality:** **No Tolkien or Blizzard/WoW** names, terms, iconography, or assets. All proper nouns and assets must be original or licensed. The spec's lore (Keldran, the Sundering, Shards, Rifts, the Hollow Legion, etc.) is the canonical, original vocabulary — use it.
- **Data-driven:** items, abilities, mobs, quests, loot, NPCs, vendors, zones live in **DataTables / DataAssets**, not in code.
- **Server authority:** never trust the client. Networking + sharding is the hardest part — cover it with Gauntlet tests early.
- **AI content pipeline:** API keys come from env/secrets, **never hardcoded**. Generators are idempotent, cached by content hash (`make generate`), and skip already-cached items. Every generated asset is recorded in `Docs/04_AssetManifest.md` / `Docs/gen/` manifests. **Never call an LLM/TTS service from the game client** — runtime NPC dialogue goes through the backend Dialogue Service, which caches, rate-limits, guards, and falls back to authored lines on timeout/quota.
- **Assets:** free/open sources only; track a license for **every** external asset in `Docs/04_AssetManifest.md` and build a credits screen. Run the §8 coherence pass (unify scale, materials, lighting) so mixed-source assets read as one world.
- **GPT Image 2 is 2D only** (icons, UI, textures, concept art) — never for 3D meshes.
