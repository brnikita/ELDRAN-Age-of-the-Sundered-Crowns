# 01 — Data Schemas (DataTables & DataAssets)

> **Hard rule:** items, abilities, mobs, quests, loot, NPCs, vendors, and zones are
> **data-driven** — defined in DataTables (row structs) or Primary DataAssets, never
> hardcoded in C++. C++ defines the *struct/asset shape and behavior*; designers/generators
> fill the *rows*. Every row that references an asset (icon, mesh, ability, effect) must
> resolve — enforced by the data-integrity automation test (see `07_TestPlan.md`).

## Conventions
- **Row struct** C++ types are `USTRUCT`s named `F<Thing>Row : public FTableRowBase`.
- **DataAsset** types are `UPrimaryDataAsset` subclasses named `U<Thing>DataAsset`.
- Identifiers are `FName` `RowName`s, globally unique within a table, lowercase-kebab style
  (e.g. `mob.brambleback`, `item.weapon.worn-shortsword`, `quest.thin-the-thorns`).
- Asset references use **soft pointers** (`TSoftObjectPtr` / `TSoftClassPtr`) so tables don't
  force-load everything; the data-integrity test validates they resolve.
- GameplayTags drive categorization (e.g. `Item.Type.Weapon`, `Damage.Type.Physical`).
- All numeric tunables (damage, cost, cooldown) live in rows, not code.

DataTable CSV/JSON assets live under `Content/Data/<Domain>/`. Authoritative tables for the
slice are listed per-section below.

---

## 1. Attributes (GAS) — `UKeldranAttributeSet`
Not a DataTable (it's a GAS `UAttributeSet`), but the schema of attributes is canonical here:

| Attribute | Type | Replicated | Notes |
|-----------|------|-----------|-------|
| Health / MaxHealth | float | yes | death at 0 |
| Mana / MaxMana | float | yes | caster resource |
| Stamina / MaxStamina | float | yes | melee resource (slice Warden) |
| Strength (STR) | float | yes | scales physical damage |
| Agility (AGI) | float | yes | scales crit/speed |
| Intellect (INT) | float | yes | scales spell damage |
| Armor | float | yes | mitigates physical |
| AttackPower (meta) | float | no | derived; transient |
| MoveSpeed | float | yes | base + modifiers |

Base values per race/class come from `DT_AttributeBases` (below).

---

## 2. `F AttributeBaseRow` → `DT_AttributeBases`
Keyed by `race.class` (e.g. `freeborn.warden`).
```
RowName: FName               // "freeborn.warden"
Race: FGameplayTag           // Race.Freeborn
Class: FGameplayTag          // Class.Warden
Health, Mana, Stamina: float
STR, AGI, INT, Armor: float
MoveSpeed: float
StartingAbilities: TArray<FName>  // -> DT_Abilities rows
StartingItems: TArray<FName>      // -> DT_Items rows
```

## 3. `FItemRow` → `DT_Items`
```
RowName: FName                 // "item.weapon.worn-shortsword"
DisplayName: FText
Description: FText
ItemType: FGameplayTag         // Item.Type.{Weapon,Armor,Consumable,Quest,Trinket,Reagent}
EquipSlot: FGameplayTag        // Equip.Slot.{MainHand,OffHand,Head,Chest,...,None}
Rarity: EItemRarity            // Common, Uncommon(blue=Rare), Rare, Epic
Icon: TSoftObjectPtr<UTexture2D>
Mesh: TSoftObjectPtr<USkeletalMesh>   // null for non-visual items
MaxStack: int32                // 1 for gear, N for stackables
StatModifiers: TArray<FAttributeModifier>  // {Attribute, Op:Add/Mul, Magnitude}
GrantedAbilities: TArray<FName>            // weapons can grant abilities
UseEffect: FName               // -> DT_GameplayEffects (consumables)
VendorValue: int32             // buy/sell base price
RequiredLevel: int32
GameplayTags: FGameplayTagContainer
```
`FAttributeModifier { FGameplayAttribute Attribute; EModifierOp Op; float Magnitude; }`

## 4. `FAbilityRow` → `DT_Abilities`
```
RowName: FName                 // "ability.warden.shield-bash"
DisplayName: FText
Description: FText
AbilityClass: TSoftClassPtr<UGameplayAbility>  // the GA_ class implementing it
Icon: TSoftObjectPtr<UTexture2D>
CostAttribute: FGameplayAttribute              // Stamina/Mana
CostMagnitude: float
Cooldown: float                                // seconds
TriggersGCD: bool
CastTime: float                                // 0 = instant
Range: float
ApplyEffects: TArray<FName>                    // -> DT_GameplayEffects
InputTag: FGameplayTag                         // Input.Ability.Slot1..N
RequiredLevel: int32
```

## 5. `FGameplayEffectRow` → `DT_GameplayEffects`
Describes data-driven effects (damage/heal/buff/debuff) instantiated by a generic effect class.
```
RowName: FName                 // "ge.damage.physical.basic"
EffectClass: TSoftClassPtr<UGameplayEffect>    // base GE class
DurationType: EGEDuration      // Instant, Duration, Infinite
Duration: float
Period: float                  // for DoT/HoT (0 = none)
Modifiers: TArray<FAttributeModifier>          // what it changes
GrantedTags: FGameplayTagContainer             // e.g. State.Stunned
StackingType: EGameplayEffectStackingType
MaxStacks: int32
Icon: TSoftObjectPtr<UTexture2D>               // buff/debuff bar icon
```

## 6. `FMobRow` → `DT_Mobs`
```
RowName: FName                 // "mob.brambleback"
DisplayName: FText
SkeletalMesh: TSoftObjectPtr<USkeletalMesh>
AnimBlueprint: TSoftClassPtr<UAnimInstance>
Level: int32
IsElite: bool                  // mini-boss = true
AttributeBases: FName          // -> DT_AttributeBases (or inline)
Abilities: TArray<FName>       // -> DT_Abilities
BehaviorTree: TSoftClassPtr<UBehaviorTree>   // or StateTree asset ref
AggroRadius: float
LeashRadius: float
MovementSpeed: float
LootTable: FName               // -> DT_LootTables
XPReward: int32
Faction: FGameplayTag          // Faction.Hostile.Wildlife / Faction.Hostile.Hollow
Nameplate: FText
```

## 7. Loot — `FLootTableRow` → `DT_LootTables`
A loot table is a row whose `Entries` are weighted drops.
```
RowName: FName                 // "loot.brambleback"
Entries: TArray<FLootEntry>
GuaranteedCoinMin, GuaranteedCoinMax: int32
```
`FLootEntry { FName ItemRow; float DropChance(0..1); int32 MinQty; int32 MaxQty; bool IsQuestDrop; }`
Loot math (roll order, no-dupe rules, quest-drop guarantee while quest active) is specified in
`00_ImplementationSpec.md` §Inventory/Loot and covered by a unit test.

## 8. Quests — `FQuestRow` → `DT_Quests`
```
RowName: FName                 // "quest.thin-the-thorns"
DisplayName: FText
SummaryText: FText             // journal
GiverNPC: FName                // -> DT_NPCs
TurnInNPC: FName               // -> DT_NPCs
Objectives: TArray<FQuestObjective>
PrereqQuests: TArray<FName>    // chain ordering
RewardItems: TArray<FQuestReward>   // {ItemRow, Qty}
RewardCoin: int32
RewardXP: int32
MinLevel: int32
DialogueText: FText            // offered/in-progress/complete lines (or -> bark pool id)
```
`FQuestObjective { EObjectiveType Type(Kill/Collect/Reach); FName TargetId; int32 Count; FText Desc; }`
- Kill: `TargetId` = mob row; Count = N.
- Collect: `TargetId` = item row; Count = N.
- Reach: `TargetId` = a `FName` location marker placed in the level (trigger volume tag).

## 9. NPCs — `FNPCRow` → `DT_NPCs`
```
RowName: FName                 // "npc.halek-orne"
DisplayName: FText
Race: FGameplayTag
SkeletalMesh: TSoftObjectPtr<USkeletalMesh>   // MetaHuman-derived
AnimBlueprint: TSoftClassPtr<UAnimInstance>
Role: ENPCRole                 // QuestGiver, Vendor, Flavor, QuestGiver+Vendor
VendorTable: FName             // -> DT_Vendors (if vendor), else NAME_None
Quests: TArray<FName>          // quests this NPC gives
BarkPool: FName                // -> DT_Barks (ambient lines)
IsLivingNPC: bool              // true only for Doran Vale (runtime LLM demo)
PersonaPrompt: FText           // system prompt seed for living NPCs
FallbackLines: TArray<FText>   // authored fallback if dialogue service unavailable
```

## 10. Vendors — `FVendorRow` → `DT_Vendors`
```
RowName: FName                 // "vendor.mother-wend"
Inventory: TArray<FVendorEntry>
BuybackEnabled: bool
PriceMultiplierBuy: float      // markup vs VendorValue
PriceMultiplierSell: float     // markdown vs VendorValue
```
`FVendorEntry { FName ItemRow; int32 Stock(-1=infinite); int32 RequiredLevel; }`

## 11. Zones — `UZoneDataAsset` (DataAsset, one per zone)
```
ZoneId: FName                  // "zone.vaels-rest"
DisplayName: FText
LevelMap: TSoftObjectPtr<UWorld>      // the World Partition map
RecommendedLevelMin/Max: int32
Faction: FGameplayTag                  // owning faction (Covenant for L1)
AmbientMusic: TSoftObjectPtr<UMetaSoundSource>
AmbientSoundscape: TSoftObjectPtr<UMetaSoundSource>
PostProcessProfile: TSoftObjectPtr<UMaterialInterface>  // coherence profile
SpawnTables: TArray<FName>             // -> DT_Mobs rows + spawn config
SafeZones: TArray<FName>               // trigger tags where combat disabled
```

## 12. Barks — `FBarkRow` → `DT_Barks`
```
RowName: FName                 // "bark.brambleback.aggro"
Lines: TArray<FText>           // randomly chosen
VOAssets: TArray<TSoftObjectPtr<USoundBase>>  // optional ElevenLabs VO
Context: FGameplayTag          // Bark.Aggro / Bark.Death / Bark.Idle / Bark.Greeting
```

---

## Slice (M2) authoritative table contents (must be filled, no TBD)
- `DT_AttributeBases`: `freeborn.warden` (+ mob bases).
- `DT_Items`: worn-shortsword, worn-shield, common chest, 3 quest rewards (blue accessory,
  blue weapon), starter potion, skitter-gland (quest), shard-fragment-of-vael (quest).
- `DT_Abilities`: warden — basic-attack, shield-bash (stun), defensive-stance (buff).
- `DT_GameplayEffects`: physical-damage, void-damage, heal-potion, stun, defensive-buff.
- `DT_Mobs`: brambleback, mire-skitter, hollow-straggler, warden-of-the-toe (elite).
- `DT_LootTables`: one per slice mob + the boss (with quest-drop guarantee).
- `DT_Quests`: the 5 L1 quests from `06_WorldBible.md` §6.2.
- `DT_NPCs`: halek-orne, mother-wend, talia-vond, doran-vale.
- `DT_Vendors`: mother-wend.
- `UZoneDataAsset`: zone.vaels-rest.
- `DT_Barks`: aggro/death/idle for the 4 creatures + greetings for the 4 NPCs.

These are the rows the M2 Gauntlet test and the data-integrity test will assert against.
