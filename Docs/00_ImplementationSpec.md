# 00 — Implementation Spec (UE C++ classes & responsibilities)

> Every gameplay system from spec §10 mapped to concrete UE 5.8 classes (C++ name + parent),
> module placement, replication rules, and Blueprint responsibilities. Data shapes live in
> `01_DataSchemas.md`; networking detail in `02_NetworkDesign.md`. **Server-authoritative**
> throughout; clients predict movement and ability activation only where GAS supports it.

## Module map (`Source/Keldran*`)
| Module | Type | Owns |
|--------|------|------|
| `KeldranCore` | Runtime | GameInstance, subsystems, base types, GameplayTags, AssetManager |
| `KeldranCharacter` | Runtime | Pawns, PlayerController, movement, camera, MetaHuman setup |
| `KeldranAbilities` | Runtime | GAS: ASC owner glue, AttributeSet, abilities, effects |
| `KeldranInventory` | Runtime | Items, inventory, equipment, loot |
| `KeldranQuests` | Runtime | Quests, objectives tracking, journal, dialogue glue |
| `KeldranAI` | Runtime | Mob pawns, StateTree/BT, perception, spawning |
| `KeldranNet` | Runtime | Gateway client, session, login flow, (later) zone handoff |
| `KeldranUI` | Runtime | UMG widgets, HUD, windows, nameplates |
| `KeldranServer` | Runtime (server-focused) | Validation, persistence bridge, authority logic |
| `KeldranTests` | DeveloperTool | Automation specs, functional tests, Gauntlet controllers |

Dependencies flow downward; `KeldranCore` depends on nothing project-side. Avoid cycles.

---

## 1. Core (`KeldranCore`)
- **`UKeldranGameInstance : UGameInstance`** — process lifetime; holds the `UKeldranSessionSubsystem`
  reference; entry point for connecting to gateway then server.
- **`UKeldranAssetManager : UAssetManager`** — set in `DefaultEngine.ini`; registers Primary
  Asset types (ZoneDataAsset, etc.); validates soft refs on cook.
- **`AKeldranGameModeBase : AGameModeBase`** (server only) — base for zone game modes; spawns
  players, owns spawn director. Slice mode: **`AVaelsRestGameMode`**.
- **`AKeldranGameState : AGameStateBase`** — replicated world state (zone id, world time).
- **`AKeldranPlayerState : APlayerState`** — **owns the AbilitySystemComponent** for players
  (standard MMO pattern: ASC on PlayerState so it survives pawn death/respawn). Holds level/XP.
- **Native GameplayTags** declared in `KeldranGameplayTags.h` (Race.*, Class.*, Item.*,
  Damage.*, State.*, Input.Ability.*, Bark.*, Faction.*).

## 2. Character (`KeldranCharacter`)
- **`AKeldranCharacterBase : ACharacter, IAbilitySystemInterface`** — shared player/NPC base;
  `GetAbilitySystemComponent()` returns the PlayerState's (players) or its own (mobs).
- **`AKeldranPlayerCharacter : AKeldranCharacterBase`** — third-person; spring arm + camera;
  Enhanced Input; movement via `UCharacterMovementComponent` (server-authoritative with the
  built-in client prediction). On possess: init ASC from PlayerState, grant starting abilities
  from `DT_AttributeBases`.
- **`AKeldranPlayerController : APlayerController`** — input mapping contexts; target selection
  (tab-target); routes ability inputs by `Input.Ability.*` tag to ASC; owns local HUD.
- **`UKeldranInputConfig : UDataAsset`** — maps Enhanced Input actions → ability input tags.
- **Camera/MetaHuman:** MetaHuman skeletal mesh + Mixamo-retargeted anim BP. Appearance choices
  (slice: minimal) stored on PlayerState and persisted.

## 3. Abilities / GAS (`KeldranAbilities`)
- **`UKeldranAbilitySystemComponent : UAbilitySystemComponent`** — replication mode **Mixed**
  for players, **Minimal** for AI; input-tag → ability activation helper.
- **`UKeldranAttributeSet : UAttributeSet`** — attributes per `01_DataSchemas.md` §1;
  `PreAttributeChange` clamps; `PostGameplayEffectExecute` handles damage→health, death.
- **`UKeldranGameplayAbility : UGameplayAbility`** — base; reads its `FAbilityRow` (cost,
  cooldown, GCD, effects) so most abilities are **data-driven instances**, not bespoke classes.
  Concrete subclasses only where logic is special:
  - `UGA_BasicAttack`, `UGA_ShieldBash` (stun), `UGA_DefensiveStance` (buff) — slice Warden set.
- **`UKeldranGameplayEffect`** — generic GE configured from `DT_GameplayEffects`; a
  `UGameplayEffectExecutionCalculation` (`UKeldranDamageExecution`) computes damage from
  AttackPower/STR vs Armor. Cooldown/cost GEs generated from ability rows.
- **Replication:** GAS replicates attributes + active effects; ability activation is predicted
  client-side and confirmed/rejected by the server.

## 4. Inventory / Equipment / Loot (`KeldranInventory`)
- **`UKeldranInventoryComponent : UActorComponent`** (on PlayerState or Character) — replicated
  `TArray<FInventoryItem>` (`{FName ItemRow; int32 Qty; FGuid InstanceId;}`); add/remove/stack
  rules server-side; `OnRep` updates UI. **All mutations are server RPCs**, never client-trusted.
- **`UKeldranEquipmentComponent : UActorComponent`** — equipped items per `Equip.Slot.*`;
  applies/removes the item's `StatModifiers` as a GAS GameplayEffect on equip/unequip; replicates
  visible mesh changes.
- **`UKeldranLootComponent`** + **`ULootService`** — on mob death the server rolls the mob's
  `DT_LootTables` row (seeded, deterministic for tests), spawns an **`AKeldranLootContainer`**
  (interactable corpse/chest) whose contents are server-owned; loot pickup is a server RPC.
  Quest drops are guaranteed while the relevant quest objective is active (loot math unit-tested).

## 5. Quests / Dialogue (`KeldranQuests`)
- **`UKeldranQuestComponent : UActorComponent`** (per player) — tracks active/completed quests,
  per-objective progress; replicated to owner only. Subscribes to gameplay events (mob killed,
  item collected, area reached) via a lightweight **`UQuestEventBus`** (GameInstance subsystem).
- **`UKeldranQuestSubsystem : UWorldSubsystem`** (server) — accepts/advances/turn-in; validates
  prereqs/level; grants rewards via Inventory + ASC (XP). Persists progress (see Net/Server).
- **Reach objectives:** `AKeldranAreaMarker : ATriggerVolume` tagged with a location `FName`;
  overlap on a player with that objective fires the event.
- **Dialogue:** `UKeldranDialogueComponent` on NPCs; authored lines from `DT_Quests`/`DT_Barks`.
  Living NPC (Doran Vale) calls **`UDialogueServiceClient`** (in `KeldranNet`) → backend; the
  game client never calls the LLM directly.

## 6. AI / Mobs (`KeldranAI`)
- **`AKeldranMobCharacter : AKeldranCharacterBase`** — owns its own ASC (Minimal replication);
  initialized from `DT_Mobs` (mesh, anim BP, attributes, abilities, loot table).
- **`AKeldranAIController : AAIController`** — runs the mob's StateTree (preferred in 5.8) or
  Behavior Tree; `UAIPerceptionComponent` for sight; aggro/leash from row radii.
- **`UKeldranSpawnDirector : UWorldSubsystem`** (server) — spawns/respawns mobs per
  `UZoneDataAsset.SpawnTables`; throttles for relevancy.
- **AI states (slice):** Idle → Patrol → Investigate → Combat(approach/ability/reposition) →
  Flee(low-HP swarmers) → Leash/Return. Boss adds a telegraphed-slam + summon-adds state.

## 7. Networking glue (`KeldranNet`) — detail in `02_NetworkDesign.md`
- **`UKeldranSessionSubsystem : UGameInstanceSubsystem`** — login flow: POST credentials to
  **Login/Account Gateway** → receive `{sessionToken, shardAddress}` → `ClientTravel` to server
  with token as URL option → server validates token with gateway/Redis before admitting.
- **`UGatewayClient`** — HTTP client (UE `HttpModule`) to the gateway.
- **`UDialogueServiceClient`** — HTTP client to the NPC Dialogue Service (server-side use).
- **`AKeldranGameSession : AGameSession`** (server) — `ApproveLogin`/`PreLogin` validates token.

## 8. UI (`KeldranUI`)
- **`AKeldranHUD : AHUD`** + **`UKeldranHUDWidget : UUserWidget`** — resource bars (Health/
  Mana/Stamina), action bar (abilities by input tag), minimap, quest tracker, chat box.
- Windows: `UInventoryWidget`, `UEquipmentWidget`, `UJournalWidget`, `UVendorWidget`,
  `UCharacterCreateWidget`, `UDialogueWidget`.
- **`UKeldranNameplateWidget`** — world-space nameplate/health over targets & mobs.
- Chat: `UKeldranChatComponent` (PlayerState) → server-routed zone chat (server RPC + multicast
  to relevant clients). No client-trusted broadcast.
- **BP responsibility:** widget layout/animation/binding in UMG; C++ exposes data + events.

## 9. Server / Persistence (`KeldranServer`)
- **`AKeldranServerGameMode`** mix-ins / **`UPersistenceSubsystem : UGameInstanceSubsystem`**
  (server) — bridges game state ↔ backend: on login load character (attributes, inventory,
  equipment, quest progress, position) from Postgres via the gateway/persistence API; autosave
  on interval + on logout. **Authoritative validation** of all client requests lives here and in
  each component's server RPCs.
- **`UCharacterPersistencePayload`** — serializable snapshot used for save/load and the
  relog-persistence Gauntlet assertion.

## 10. Blueprint vs C++ split (policy)
- **C++:** all systems, components, replication, GAS abilities/effects/attributes, data structs,
  authoritative logic, subsystems.
- **Blueprint:** UMG widget trees + bindings, anim blueprints/state machines, level scripting,
  cosmetic VFX/SFX hookup, designer-tunable BP child classes of C++ bases. No gameplay rules in BP.

## 11. Slice (M2) class checklist (what must exist & compile)
GameInstance/AssetManager/GameMode(VaelsRest)/GameState/PlayerState(+ASC); PlayerCharacter +
Controller + InputConfig; KeldranASC + AttributeSet + 3 Warden abilities + DamageExecution;
Inventory/Equipment/Loot components + LootService + LootContainer; QuestComponent +
QuestSubsystem + AreaMarker + DialogueComponent; MobCharacter + AIController + SpawnDirector +
StateTree(s); SessionSubsystem + GatewayClient + DialogueServiceClient + GameSession;
HUD + core widgets + Nameplate + ChatComponent; PersistenceSubsystem + payload. All listed in
`05_TaskGraph.md` with per-task DoD.
