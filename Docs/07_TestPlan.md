# 07 — Test Plan

> Three layers, all runnable **headless** so they fit the autonomous loop (spec §6). A red test
> is a **blocking failure** in the loop (§13). Tests are written alongside the feature, not after.

## Layers
- **(a) Unit / automation** — C++ `IMPLEMENT_SIMPLE_AUTOMATION_TEST` / Automation Spec, plus
  Blueprint **Functional Tests** in maps prefixed `FTEST_`. No GPU needed.
- **(b) Functional / integration** — load a test map, spawn actors, drive latent commands,
  assert outcomes.
- **(c) Gauntlet multiplayer** — dedicated server + N clients, a `UGauntletTestController`
  puppeteers them; assert replication, persistence, and capture perf (CSV profiler).

## Headless commands
```bash
# Automation/functional, no GPU, exit when queue empties:
UnrealEditor-Cmd Keldran.uproject -game -unattended -nullrhi -nosound -nosplash -stdout \
  -ExecCmds="Automation RunTests <Filter>;quit" -TestExit="Automation Test Queue Empty"

# Build + cook:
RunUAT BuildCookRun -project=Keldran.uproject -noP4 -build -cook -stage -server -client ...

# Multiplayer pass on cooked build:
RunUAT RunGauntlet -project=Keldran.uproject -build=<CookedBuildPath> -test=<GauntletTest>
```
Test filter prefix: `Keldran.` (e.g. `Keldran.Abilities.CostCooldown`).

---

## Always-on tests

### Data-integrity (automation) — `Keldran.Data.Integrity`
For every row in every shipping DataTable, assert all soft asset refs resolve (icon, mesh,
anim BP, ability class, effect class) and all cross-table refs exist (mob→loot table,
quest→npc, vendor→item, attribute base→ability/item). **This is the guardrail that keeps the
data-driven design honest.** Runs in CI on every commit.

### Originality lint (tooling test) — `Keldran.Content.Originality`
Scans DataTable display text + generated content manifests for banned terms (Tolkien/
Blizzard/WoW). Fails the build on a hit.

---

## Per-milestone test matrix

### M0 — docs
Not code-tested. Exit check: all 8 docs exist, are internally consistent, and `05_TaskGraph.md`
has no blocking TBD for the slice. (Verified by the M0 consistency pass + a simple doc-presence
script in CI.)

### M1 — skeleton + backend
| Test | Layer | Pass criteria |
|------|-------|---------------|
| `Keldran.Smoke.Compile` | build | editor + dedicated server + client targets compile clean |
| `Keldran.Net.EmptyConnect` | Gauntlet | 1 server + 1 client connect, client spawns, no errors |
| `Keldran.Backend.Health` | integration (script) | `docker compose up` → gateway `/health`, Postgres, Redis, dialogue service all respond |
| `Keldran.Backend.Auth` | integration | login → token → verify roundtrip succeeds; bad token rejected |
| `Keldran.Data.Integrity` | automation | passes on the (empty/seed) tables |
| `Keldran.Gen.Smoke` | tooling | each generator makes one cheap cached artifact; re-run makes 0 API calls |
| CI pipeline | CI | compile → automation → Gauntlet smoke (server + 2 clients) → report, green |

**M1 DoD:** all above green; empty client+server connect; backend online; CI runs.

### M2 — vertical slice (the big one)
**Unit/automation:**
| Test | Pass criteria |
|------|---------------|
| `Keldran.Abilities.CostCooldown` | abilities deduct correct cost, respect GCD + cooldown, reject when insufficient |
| `Keldran.Abilities.Damage` | damage execution = expected from STR/AttackPower vs Armor; death at 0 HP |
| `Keldran.Inventory.StackRules` | add/remove/move/stack/split obey MaxStack; no dupes |
| `Keldran.Equipment.StatApply` | equip applies stat GE; unequip removes it; attributes match |
| `Keldran.Loot.Tables` | seeded roll matches expected distribution; quest drop guaranteed while quest active |
| `Keldran.Quests.Lifecycle` | accept→progress (kill/collect/reach)→turn-in increments + grants rewards; prereqs enforced |
| `Keldran.Vendor.BuySell` | price math, coin deduction/grant, stock rules correct |
| `Keldran.Data.Integrity` | passes on full slice tables |

**Functional (`FTEST_*` maps):**
- `FTEST_Combat`: spawn player + Brambleback, activate abilities, mob dies, loot spawns.
- `FTEST_QuestLoop`: accept "Thin the Thorns", kill 6, objective completes, turn in, reward granted.
- `FTEST_Equip`: loot a weapon, equip, attack power increases, hit harder.
- `FTEST_Dialogue`: talk to Doran Vale; service mocked-unavailable → authored fallback line shown.

**Gauntlet — `Keldran.Gauntlet.Slice` (the M2 Definition of Done):**
Dedicated server (Vael's Rest) + **2 clients**. The controller asserts, fully automatically:
1. Both clients connect and **see each other** (replicated pawns/nameplates).
2. Both **kill mobs** (abilities apply, mobs die, XP granted).
3. Both **take + turn in quests** (the L1 chain through "Thin the Thorns"/"Salvage the Skitters").
4. Both **loot & equip** (loot container, pickup, equip, stat change replicated).
5. Each client **relogs** (disconnect → reconnect) and **progress is saved** (quest state,
   inventory, equipment, level/XP, position restored from Postgres).
6. No replication errors; CSV profiler captured for perf baseline.

**M2 DoD:** the Gauntlet test passes green, plus all M2 unit/functional tests.

### M3 — content pipeline
| Test | Pass criteria |
|------|---------------|
| `Keldran.Gen.NewAsset` | `make generate` adds a new icon + SFX + VO + music; data-integrity still green |
| `Keldran.Content.MetaHuman` | an animated MetaHuman (Mixamo-retargeted) loads and plays in-game |
| `Keldran.Gen.Idempotent` | re-running `make generate` with no spec change = 0 API calls, 0 file changes |

### M4–M7 (summary; expanded when reached)
- **M4 Social/Progress:** functional + Gauntlet for party formation, group loot/XP split,
  guild create/chat/bank, leveling + talent application; L2–L3 traversal.
- **M5 Worlds:** ≥2 rift-worlds + the 5-player dungeon D1 playable; Gauntlet 5-client dungeon clear.
- **M6 Endgame:** one raid encounter, reputation gain, one PvP mode (duel/battleground) pass tests.
- **M7 Hardening:** scripted Gauntlet **load test across ≥2 shards**; anti-cheat checks; survives.

---

## CI policy (self-hosted runner, per commit)
`compile → Keldran.Data.Integrity + Keldran.Content.Originality → unit/automation →
functional → Gauntlet smoke (server + 2 clients) → publish report`. Red = blocking; the
autonomous loop must not commit past a red test. Generation jobs run on demand (cost), not
per-commit. Milestone DoD Gauntlet runs on the milestone branch before tagging.

## Determinism notes
- Loot/ability tests seed RNG so results are reproducible.
- Gauntlet uses fixed-tick where possible and tolerances on timing-sensitive asserts.
- Tests must not depend on generated final art/audio (placeholders are valid for logic tests).
