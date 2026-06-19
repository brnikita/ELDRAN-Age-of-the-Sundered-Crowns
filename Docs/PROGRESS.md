# PROGRESS — KELDRAN build ledger

> The run's memory. One entry per task per the format below (spec §13). Newest at the bottom.
> The agent picks the next unblocked task from `05_TaskGraph.md`, implements, builds, tests,
> commits, and appends here. A milestone is done only when its Definition of Done passes.

**Entry format**
```
## <ISO timestamp> | <task id> | <DONE|BLOCKED|WIP>
- change:
- build: <pass/fail/n-a>   tests: <pass/fail/n-a>   gauntlet: <pass/fail/n-a>
- commit: <hash>
- notes/blockers:
- resume here: <next task id>
```

---

## 2026-06-19 | M0-1..M0-7 | DONE
- change: authored Docs 06 (World Bible), 01 (Data Schemas), 00 (Implementation Spec),
  02 (Network Design), 03 (AI Content Pipeline), 04 (Asset Manifest), 07 (Test Plan).
- build: n-a   tests: n-a   gauntlet: n-a
- commit: <pending M0-9>
- notes/blockers: none. Docs scoped so the M2 vertical slice has no blocking TBD.
- resume here: M0-8 (task graph + ledger) → M0-9 (consistency pass + tag m0)

## 2026-06-19 | M0-8 | DONE
- change: authored Docs 05 (Task Graph) with per-task DoDs through M3 and M4–M7 epics; created
  this ledger.
- build: n-a   tests: n-a   gauntlet: n-a
- commit: <pending M0-9>
- notes/blockers: none.
- resume here: M0-9 (consistency pass + commit + tag m0)

## 2026-06-19 | M0-9 | DONE
- change: consistency pass — verified all 8 `/Docs` present and slice identifiers (creatures,
  NPCs, Shard Fragment of Vael, Freeborn Warden, Vael's Rest) cross-reference cleanly across
  every doc (58 coherent occurrences). Committed `/Docs` and tagged `m0`.
- build: n-a   tests: n-a (M0 has no code; doc-presence + consistency checked manually)
- commit: see tag m0
- notes/blockers: **M0 EXIT MET** — all docs exist, are consistent, and `05_TaskGraph.md` has
  no blocking TBD for the vertical slice.
- resume here: M1 — **gated on Phase A** (user installs UE 5.8 + plugins + MCP + Epic/Adobe
  logins + API keys). Backend tasks (M1-5/6/7) and gen pipeline (M1-10) can start once Docker
  + API keys are available, before the editor is fully set up.

## 2026-06-19 | M1-5,M1-6,M1-7,M1-8 | DONE
- change: built the backend stack ahead of the UE work (doesn't need the editor). `Backend/`
  with `docker-compose.yml` (Postgres 16, Redis 7, FastAPI gateway, FastAPI dialogue-service),
  `db/migrations/001_init.sql` (schema per `02` §5), gateway (auth/login/verify, characters
  CRUD, play handoff w/ single-use tickets, persistence load/save), dialogue-service (persona,
  Redis cache, per-account rate-limit, guardrails, authored fallback, OpenRouter live). Secrets
  via gitignored root `.env`; `.gitignore` + `Backend/.env.example` added.
- build: pass (4 images build)   tests: pass (manual verification below)   gauntlet: n-a
- commit: <this commit>
- verification:
  - all 4 containers HEALTHY; gateway `/health` + dialogue `/health` 200.
  - auth: register→login→bad-login 401→create char→list→play(shard+ticket)→verify(consume)→
    reuse-ticket 401 (single-use). ALL PASS.
  - persistence: saved a level-5 snapshot (xp/coin/shards/inventory/quest) and reloaded it
    intact → relog-persistence foundation proven.
  - dialogue: live LLM reply in-character + on-setting; repeat hits cache; unknown NPC 404;
    fallback path confirmed earlier.
- fixes during build: (1) passlib+bcrypt 4.x 72-byte self-test crash → switched to
  `pbkdf2_sha256`. (2) OpenRouter `llama-3.1-8b-instruct:free` retired & `llama-3.3-70b:free`
  upstream-429 → default model now `google/gemma-4-31b-it:free` (verified reliable).
- notes: OpenAI + ElevenLabs + OpenRouter keys all present in root `.env` (gitignored). OpenAI/
  ElevenLabs are for M3 content gen (not exercised yet).
- resume here: M1 UE side — **gated on Phase A editor setup** (Unreal MCP server reachable at
  127.0.0.1:8000/mcp). Remaining standalone backend-adjacent task: M1-10 (gen pipeline).

## 2026-06-20 | M1-10 | DONE
- change: built the AI generation pipeline `Tools/gen/` — `common.py` (root-.env secrets,
  content-hash cache, spend cap, originality lint, JSONL manifest, dry-run), `gen_text.py`
  (OpenRouter), `gen_images.py` (GPT Image 2), `gen_audio.py` (ElevenLabs) + prompt specs +
  root `Makefile` (`make generate` / `generate-dry`).
- build: n-a   tests: pass (live smoke + idempotency)   gauntlet: n-a
- commit: <this commit>
- verification: dry-run estimated $0.05; live smoke produced 3 real artifacts — bark JSON
  (free), Doran VO mp3 (ElevenLabs, $0.01), shield-bash icon PNG (GPT Image 2, $0.04); all in
  manifest with hashes + license; re-run = cached, $0 (idempotent).
- fixes: ElevenLabs key lacks `voices_read` → use default premade voice id (no /voices call).
- notes: generated binary media gitignored until M1-13 sets up Git LFS (regenerable from
  committed prompt specs). OpenAI/ElevenLabs/OpenRouter keys all confirmed working.
- resume here: UE editor side — gated on Phase A (user creating Keldran C++ project + MCP).

## BLOCKER (needs the user — GUI, cannot self-authorize)
- **Create the Keldran C++ project in UE 5.8 + enable the built-in Unreal MCP plugin**, then
  run `ModelContextProtocol.GenerateClientConfig ClaudeCode` (writes `.mcp.json`) so Claude
  Code can drive the editor. Until then M1-1..M1-4, M1-9, M1-11, M1-12 cannot proceed.
- **Hardware:** user's GPU is a GTX 750 — below UE 5.8 reqs for the photorealistic target.
  GPU-free work (C++ compile, headless `-nullrhi` tests, cooking, backend, gen) is unaffected;
  rendered editor/playtest/Gauntlet need an RTX-class 12GB+ GPU. Awaiting user's GPU decision.

## 2026-06-20 | M1-1,M1-2 (partial),M1-13 | DONE
- change: found engine at `B:\Programs\Epic Games\UE_5.8`. Hand-authored the UE C++ project
  skeleton at `Keldran/` (so no Project Browser needed): `Keldran.uproject` (+plugins),
  Game/Editor/Server `.Target.cs`, primary `Keldran` module (Build.cs/.h/.cpp), Config inis,
  repo-root `.mcp.json` (HTTP http://127.0.0.1:8000/mcp). Set up Git LFS (`.gitattributes` for
  .uasset/.umap + media; hooks installed) and un-ignored generated media.
- build: PASS — `Build.bat Keldran Win64 Development` => Result: Succeeded, Keldran.exe (345 MB)
  produced. Toolchain: MSVC 14.50 (Build Tools 2026) + Win SDK 22621. Proves skeleton + the
  user's toolchain work for UE 5.8. (Editor target NOT yet built — needs .NET Framework SDK.)
- tests: n-a   gauntlet: n-a   commit: <this commit>
- notes: RAM (16 GB) capped UBT to 1 parallel action → slow builds; user should add RAM to 32 GB.
- resume here: M1-2 module split (KeldranCore/etc.) — can author + compile headless (game target,
  GPU-free) without the user. Editor target + MCP gated on .NET FW SDK + the one editor click.

## BLOCKERS (need the user)
1. **.NET Framework 4.8 SDK** — add via Visual Studio Installer → Build Tools 2026 → Modify →
   Individual components → ".NET Framework 4.8 SDK" (+ targeting pack) → Modify. Needed for the
   Editor target (Swarm/Lightmass) → required to open the project + run Unreal MCP.
2. **Enable Unreal MCP** in the editor (built-in 5.8): Edit>Plugins → "Unreal MCP" → Enabled;
   Editor Preferences → Model Context Protocol → Auto Start Server on. (.mcp.json already at root.)
3. **GPU + PSU**: GTX 750 → plan RTX 3060 12GB (PCIe x16, best fit for the X99/PCIe-3.0 board)
   or RTX 5060 Ti 16GB. Awaiting PSU wattage + PCIe 8-pin confirmation. Also add RAM to 32 GB.

## 2026-06-20 | M1-2,M1-3,M1-4 | DONE
- M1-2: 10 domain modules authored + compiled (committed 5edb28d).
- M1-3: KeldranCore classes — GameInstance, AssetManager, GameMode/State, PlayerState
  (replicated Level/XP; ASC deferred to M2), native GameplayTags. Wired GameInstance +
  AssetManager in DefaultEngine.ini. Added NetCore dep.
- M1-4: KeldranNet — GatewayClient (async HTTP login), SessionSubsystem (login orchestration),
  GameSession (ApproveLogin ticket gate; full /auth/verify in M1-9).
- build: PASS (game target, 155s, no warnings). tests: n-a (PIE/editor gated). gauntlet: n-a.
- **MEMORY FIX (important):** 16 GB RAM made UBA thrash (kill-retry loop on commit-limit). Added
  `%APPDATA%/Unreal Engine/UnrealBuildTool/BuildConfiguration.xml` disabling UBA + capping
  MaxProcessorCount=2. Builds now reliable (slower). Remove cap after 32 GB RAM upgrade.
  Workflow: `docker compose stop` before builds to free RAM; `wsl --shutdown` if commit saturates.
- fixes: BlueprintCallable can't take multicast delegate param → single-cast DYNAMIC_DELEGATE;
  PreLogin is on GameMode not GameSession → use AGameSession::ApproveLogin.
- resume here: M1-9 (wire client login→/play→ClientTravel + server /auth/verify) and M1-11 (test
  harness) — both authorable headless. M1 editor DoD (empty client+server connect, PIE) still
  needs the .NET FW SDK + MCP enable.

## NEXT (GPU-free, autonomous)
- M1-9: complete the gateway client (characters/play/persistence calls) + server-side ticket
  verify in ApproveLogin; wire SessionSubsystem to ClientTravel.
- M1-11: KeldranTests automation tests (compile headless; full run needs editor/-nullrhi).
- Then M1 is **gated on Phase A** (user installs UE 5.8 + plugins + MCP + accounts + API keys).
  M1-5/M1-6/M1-7 (backend, Docker) and M1-10 (gen pipeline) can begin as soon as API keys +
  Docker are available, even before the editor is fully set up. M1-1..M1-4, M1-9, M1-11..M1-12
  need the UE environment verified.
