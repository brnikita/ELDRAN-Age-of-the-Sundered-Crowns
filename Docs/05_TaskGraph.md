# 05 — Task Graph (the build backlog)

> **This is the run's source of truth.** The agent always picks the next *unblocked* task,
> implements it, builds, tests, commits (task id in message), and appends to `PROGRESS.md`.
> Each task has a **Definition of Done (DoD)** that is concrete and testable. No task for the
> vertical slice (through M2) contains a blocking TBD — that is the M0 exit condition.

## Legend
- **Status:** `TODO` | `WIP` | `DONE` | `BLOCKED`
- **Dep:** task ids that must be DONE first.
- Tasks are ordered; within a milestone, respect deps but otherwise take lowest id first.
- A task is DONE only when its DoD passes (build green + listed tests green where applicable).

---

## M0 — Spec (this milestone)
| ID | Task | Dep | DoD | Status |
|----|------|-----|-----|--------|
| M0-1 | World Bible | — | `Docs/06_WorldBible.md` complete; slice content fully authored | DONE |
| M0-2 | Data schemas | — | `Docs/01_DataSchemas.md` complete; slice tables enumerated | DONE |
| M0-3 | Implementation spec | M0-2 | `Docs/00_ImplementationSpec.md` complete; slice classes listed | DONE |
| M0-4 | Network design | M0-3 | `Docs/02_NetworkDesign.md` complete; authority matrix + RPCs | DONE |
| M0-5 | AI content pipeline | — | `Docs/03_AIContentPipeline.md` complete | DONE |
| M0-6 | Asset manifest | M0-1 | `Docs/04_AssetManifest.md` complete; sources+licenses | DONE |
| M0-7 | Test plan | M0-3,M0-4 | `Docs/07_TestPlan.md` complete; per-milestone matrix | DONE |
| M0-8 | Task graph + ledger | all M0 | this file + `PROGRESS.md`; no blocking TBD for slice | WIP |
| M0-9 | Consistency pass + tag | M0-8 | docs cross-checked; commit; tag `m0` | TODO |

---

## M1 — Skeleton + backend + pipeline wiring
*Goal: empty client+server connect; backend online; build + automation tests green; CI runs;
generators produce one cached artifact each.* **Gate: requires Phase A environment verified.**

| ID | Task | Dep | DoD |
|----|------|-----|-----|
| M1-1 | Create `Keldran.uproject` + enable §5 plugins; verify MCP `status` | env | editor opens; MCP connected; plugins enabled |
| M1-2 | Create `Source/Keldran*` C++ modules (stubs) + build targets (Editor/Client/Server) | M1-1 | all targets compile clean (`Keldran.Smoke.Compile`) |
| M1-3 | Core: GameInstance, AssetManager, GameMode/State, PlayerState(+ASC), GameplayTags | M1-2 | compiles; PIE launches empty map |
| M1-4 | Net skeleton: SessionSubsystem, GatewayClient, GameSession PreLogin token check | M1-3 | `Keldran.Net.EmptyConnect` (1s+1c) green |
| M1-5 | Backend scaffold: `Backend/docker-compose.yml` (Postgres, Redis, gateway, dialogue-service) | — | `docker compose up` → all healthy (`Keldran.Backend.Health`) |
| M1-6 | DB migrations (accounts/characters/inventory/equipment/quest_progress/currency) | M1-5 | migrations apply; schema matches `02` §5 |
| M1-7 | Gateway: auth (login/verify), characters CRUD, persistence API | M1-6 | `Keldran.Backend.Auth` green |
| M1-8 | NPC Dialogue Service: persona, cache, rate-limit, guardrails, authored fallback | M1-5 | unit test: fallback on timeout; cache hit on repeat |
| M1-9 | Wire client login → gateway → ClientTravel → server token verify (end-to-end) | M1-4,M1-7 | real login path connects a client to the server |
| M1-10 | Gen pipeline: `Tools/gen/*` + `make generate` + cache + manifest + spend cap + originality lint | env(keys) | `Keldran.Gen.Smoke` green; dry-run prints cost |
| M1-11 | Test harness: `KeldranTests` module, `Keldran.Data.Integrity`, `Keldran.Content.Originality`, Gauntlet base controller | M1-2 | tests run headless; integrity/originality green |
| M1-12 | CI: self-hosted runner pipeline (compile→tests→Gauntlet smoke→report) | M1-11 | CI green on a commit |
| M1-13 | Decide asset storage (Git LFS vs committed) + `.gitattributes`/`.gitignore` for UE | M1-1 | large/binary handling configured; repo stays clean |
| M1-14 | Milestone gate: run M1 DoD, tag `m1` | all M1 | all M1 tests green; tag created |

---

## M2 — Vertical slice (Vael's Rest)
*Goal: the Gauntlet slice test passes (2 clients: connect, see each other, kill mobs, quest,
loot/equip, relog with progress saved).* Built data-driven on M1.

| ID | Task | Dep | DoD |
|----|------|-----|-----|
| M2-1 | GAS: `UKeldranAttributeSet` + clamps + death handling | M1-3 | `Keldran.Abilities.Damage` (death at 0) green |
| M2-2 | GAS: ASC setup, data-driven `UKeldranGameplayAbility`, cost/cooldown/GCD | M2-1 | `Keldran.Abilities.CostCooldown` green |
| M2-3 | Damage execution calc (STR/AP vs Armor) + `DT_GameplayEffects` | M2-2 | `Keldran.Abilities.Damage` green |
| M2-4 | Warden abilities: basic-attack, shield-bash (stun), defensive-stance (buff) | M2-3 | abilities work in `FTEST_Combat` |
| M2-5 | PlayerCharacter + Controller + Enhanced Input + 3rd-person camera + movement | M1-3 | move/look/target work in PIE; server-authoritative |
| M2-6 | Tab-target + nameplates (`UKeldranNameplateWidget`) | M2-5 | target select + nameplate health replicate |
| M2-7 | Inventory component + stack rules + server RPCs | M1-3 | `Keldran.Inventory.StackRules` green |
| M2-8 | Equipment component + stat GE apply/remove + visible mesh | M2-7,M2-2 | `Keldran.Equipment.StatApply`, `FTEST_Equip` green |
| M2-9 | Loot: LootComponent, LootService (seeded), LootContainer, quest-drop guarantee | M2-7 | `Keldran.Loot.Tables` green |
| M2-10 | Mobs: `AKeldranMobCharacter` from `DT_Mobs` + own ASC | M2-1 | mob spawns with attributes/abilities/loot |
| M2-11 | AI: AIController + StateTree (idle/patrol/combat/flee/leash) + perception + SpawnDirector | M2-10 | slice mobs behave per `00` §6 in `FTEST_Combat` |
| M2-12 | Quests: QuestComponent + QuestSubsystem + event bus + AreaMarker (reach) | M1-3 | `Keldran.Quests.Lifecycle` green |
| M2-13 | Dialogue: DialogueComponent (authored) + DialogueServiceClient (Doran live + fallback) | M1-8,M2-12 | `FTEST_Dialogue` green (fallback path) |
| M2-14 | Vendor: VendorComponent + buy/sell + `DT_Vendors` | M2-7 | `Keldran.Vendor.BuySell` green |
| M2-15 | HUD + windows (inventory/equipment/journal/vendor/dialogue/char-create) + chat | M2-5..M2-14 | all windows function; zone chat routes server-side |
| M2-16 | Character create + appearance + persistence load/save (`UPersistenceSubsystem`) | M1-7,M2-15 | create → DB; relog restores state |
| M2-17 | Author slice DataTables (all rows in `01` §"Slice authoritative contents") | M2-1..M2-14 | `Keldran.Data.Integrity` green on full slice tables |
| M2-18 | Build L1 Vael's Rest map (World Partition + PCG) + placeholders + spawns + markers | M2-11,M2-17 | zone loads; mobs spawn; reach markers fire |
| M2-19 | Functional maps: `FTEST_Combat/QuestLoop/Equip/Dialogue` | M2-4..M2-14 | all `FTEST_*` green |
| M2-20 | Gauntlet slice controller + test (the M2 DoD scenario) | all above | `Keldran.Gauntlet.Slice` green |
| M2-21 | Milestone gate: cook, run Gauntlet on cooked build, tag `m2` | M2-20 | slice DoD passes on cooked build; tag created |

---

## M3 — Content pipeline depth (push-while-green begins here)
| ID | Task | Dep | DoD |
|----|------|-----|-----|
| M3-1 | MetaHuman Freeborn Warden + Mixamo retarget pipeline (MCP/Python) | M2-21 | `Keldran.Content.MetaHuman` green (animated in-game) |
| M3-2 | Generate slice icons (GPT Image 2) + import + wire to data | M2-21 | new icons in-use; integrity green |
| M3-3 | Generate slice SFX/VO/music (ElevenLabs) + MetaSounds wiring | M2-21 | `make generate` adds audio; plays in-game |
| M3-4 | Generate bark pools + quest text (OpenRouter) + import | M2-21 | barks/quest text data-driven; originality green |
| M3-5 | Idempotency + coherence pass + credits screen from manifest | M3-1..M3-4 | `Keldran.Gen.Idempotent` green; coherence checklist done |
| M3-6 | Milestone gate: M3 DoD, tag `m3` | M3-1..M3-5 | `make generate` adds icon+SFX+VO+music + animated MetaHuman |

---

## M4–M7 (epics; expand into tasks when reached, following M0 discipline)
| ID | Epic | DoD (spec §14) |
|----|------|-----|
| M4 | Parties(5), group loot/XP, LFG, guilds+chat+bank, leveling, talents/specs, more classes, travel, L2–L3 | group quests + leveling + capital hub pass functional + Gauntlet |
| M5 | 5-player dungeon D1, gathering+crafting, player economy (Shards), rift-worlds L4–L8 | ≥2 rift-worlds + D1 playable & tested |
| M6 | Raids (10–20), reputations, seasons, world bosses, optional PvP (duels/BG/arena), AH | one raid + reputation + one PvP mode pass tests |
| M7 | Sharding (World Service), anti-cheat, DevOps | survives scripted Gauntlet load test across ≥2 shards |

> Each M4–M7 epic must be broken into concrete tasks **with DoDs in this file** before its code
> is written (same rule that made M0–M2 buildable). Expanding an epic is itself the first task
> of that milestone.

---

## Autonomous loop reminder (spec §13)
1. Pick next unblocked task (lowest id, deps DONE).
2. Implement → `build_project` → run the task's tests → if green: `git commit` (task id) →
   append `PROGRESS.md` → next.
3. On failure: capture error; up to N fixes via `lookup_docs`; else mark `BLOCKED` with notes,
   move on. No infinite loops.
4. At milestone boundary: run DoD incl. Gauntlet; tag checkpoint if green.
5. Stop & summarize when a milestone passes and the next needs a human prerequisite, all tasks
   are DONE/BLOCKED, or a spend cap is hit. Always leave a "resume here" note in `PROGRESS.md`.
