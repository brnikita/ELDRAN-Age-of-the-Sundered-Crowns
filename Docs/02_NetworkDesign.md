# 02 — Network Design

> **Principle:** the dedicated server is authoritative over all game state. The client renders,
> predicts movement, and *requests* actions; the server validates and replicates results. Never
> trust the client. This is the hardest part of the project and is covered by Gauntlet tests
> from M1 onward (spec §16).

## 1. Topology
**MVP / slice (M1–M2):**
```
[UE Client] --HTTPS--> [Login/Account Gateway] --> [PostgreSQL]
     | {sessionToken, shardAddress}                 [Redis] (sessions/presence)
     v
[Dedicated Game Server: Vael's Rest] <==replication==> [Clients]
     |--> [Persistence API on Gateway] --> [PostgreSQL]
     |--> [NPC Dialogue Service] --> [OpenRouter]   (server-side only)
```
One dedicated server process = one zone. No sharding in MVP. **Full target** adds a World
Service for zone handoff + sharding (M7) — designed-for but not built in the slice.

## 2. Login & handoff flow
1. Client → **Gateway** `POST /auth/login {account, password}` → returns
   `{ sessionToken (JWT, short TTL), accountId, shardAddress }`. Token also written to **Redis**
   (`session:<token>` → accountId, TTL).
2. Client → `GET /characters` (Bearer token) → character list. Create via `POST /characters`.
3. Client selects character → `POST /play {characterId}` → Gateway returns `shardAddress` for
   the character's current zone + a short-lived **play ticket**.
4. Client `ClientTravel("<shardAddress>?token=<ticket>&char=<id>")`.
5. **Server** `AKeldranGameSession::PreLogin` → validates ticket with Gateway/Redis (server→
   gateway `POST /auth/verify`). Reject if invalid/expired.
6. On `PostLogin`, server's `UPersistenceSubsystem` loads the character snapshot from Postgres
   (via Gateway persistence API) and spawns the pawn at the saved transform.
7. **Handoff (future):** crossing a zone border → server asks World Service for the target
   shard, saves the character, hands the client a new ticket + address, client travels.

## 3. Server-authority matrix
| System | Client may | Server authoritative over | Client prediction |
|--------|-----------|---------------------------|-------------------|
| Movement | send input/moves | final position, collision, speed | yes (CMC built-in) |
| Ability activation | request activate (input tag) | cost, cooldown, GCD, effects, hit | yes (GAS predicted) |
| Damage/heal/death | — | all attribute changes, death | no (server applies) |
| Targeting | choose local target | validity for server-side ability | local only |
| Inventory add/remove/move | request | stack rules, dupe prevention | no |
| Equip/unequip | request | slot rules, stat application | no |
| Loot pickup | request | ownership, contents, removal | no |
| Quest accept/progress/turn-in | request accept/turn-in | progress increments, rewards, prereqs | no |
| Vendor buy/sell | request | price, coin, stock | no |
| Chat | send message | rate-limit, routing, profanity guard | echo local |
| Character create/appearance | submit | validation, persistence | no |

**Rule:** any row in "Client may" is a *request only*. The server recomputes and is the source
of truth. Progress is only ever incremented server-side from server-observed events.

## 4. RPC & replication list (slice)
**Replicated state (server → clients):**
- `AKeldranPlayerState`: level, XP, ASC attributes (via GAS), active GameplayEffects.
- `AKeldranCharacterBase`: transform (CMC), team/faction, current target (for nameplates).
- `UKeldranInventoryComponent`: owner-only replicated item array.
- `UKeldranEquipmentComponent`: equipped slots (visible mesh changes multicast).
- `UKeldranQuestComponent`: owner-only quest/objective state.
- `AKeldranLootContainer`: contents (to nearby/eligible clients).
- `AKeldranGameState`: zone id, world time.

**Server RPCs (client → server), all validated:**
- `ServerActivateAbility(InputTag)` — (GAS handles most; this for non-GAS triggers).
- `ServerRequestMoveInventory(FromSlot, ToSlot)`, `ServerRequestUseItem(InstanceId)`,
  `ServerRequestEquip(InstanceId, Slot)`, `ServerRequestUnequip(Slot)`.
- `ServerRequestLoot(ContainerId, EntryIndex|All)`.
- `ServerRequestAcceptQuest(QuestRow)`, `ServerRequestTurnInQuest(QuestRow)`.
- `ServerRequestVendorBuy(VendorRow, ItemRow, Qty)`, `ServerRequestVendorSell(InstanceId, Qty)`.
- `ServerRequestInteract(ActorId)` (NPC talk / dialogue open).
- `ServerSendChat(Channel, Text)`.
- `ServerRequestCreateCharacter(CreatePayload)` (or done via Gateway pre-travel — see §2).

**Client RPCs / Multicast (server → client):**
- `ClientNotifyQuestUpdate`, `ClientNotifyLootResult`, `ClientNotifyError(Reason)`,
  `MulticastPlayAbilityMontage`, `MulticastOnDeath`, `MulticastChat`.

**Relevancy:** UE **Replication Graph** groups actors by spatial grid + always-relevant set
(GameState, owning PlayerState). Tunes for many clients later; slice uses a simple grid node.

## 5. Persistence model
**Postgres tables (managed by `Backend/db/migrations`):**
- `accounts(account_id PK, email, pw_hash, created_at)`
- `characters(character_id PK, account_id FK, name UNIQUE, race, class, level, xp,
   zone_id, pos_x, pos_y, pos_z, rot_yaw, appearance JSONB, updated_at)`
- `inventory(id PK, character_id FK, item_row, qty, instance_id, slot)`
- `equipment(character_id FK, slot, instance_id)`
- `quest_progress(character_id FK, quest_row, status, objectives JSONB, updated_at)`
- `currency(character_id FK, coin, shards)`

**Save policy:** load full snapshot on `PostLogin`; **autosave** every 60s (dirty-flag) and on
logout/disconnect; write within a transaction. The **relog-persistence** requirement (M2 DoD)
is: kill/quest/loot/equip on client A, disconnect, reconnect → state restored exactly. This is
asserted by the Gauntlet controller.

**Redis:** `session:<token>`, `presence:<accountId>`, dialogue response cache, rate-limit
counters. Ephemeral; never the source of truth for persistent data.

## 6. NPC Dialogue Service (runtime, server-side only)
- Endpoint `POST /dialogue {npcId, playerUtterance, contextHints}` → `{line}`.
- Enforces per-NPC persona/system prompt, injects a *compact* world-bible context, caches by
  `(npcId, normalizedUtterance)` in Redis, rate-limits per account, applies content guardrails,
  and **falls back to an authored line** (`FNPCRow.FallbackLines`) on timeout/quota/error.
- Default model: a free OpenRouter model for ambient NPCs (see `03_AIContentPipeline.md`).
- **The game client never calls OpenRouter** — only the dedicated server's
  `UDialogueServiceClient` → this service → OpenRouter.

## 7. Security & anti-cheat (slice-level; hardened in M7)
- All gameplay mutations server-validated (matrix §3). Movement sanity (speed/teleport) checks.
- Tokens are short-TTL JWTs verified server-side; play tickets are single-use.
- Rate-limit chat and dialogue. No secrets in client; keys only on backend/server.
- Input from client treated as untrusted; out-of-range requests → `ClientNotifyError`, no state
  change, logged.

## 8. Testing hooks (see `07_TestPlan.md`)
- Gauntlet `UGauntletTestController` drives N clients through connect → move → attack → loot →
  quest → relog and asserts replication correctness + persistence. CSV profiler captured.
- Unit tests cover loot math and ability cost/cooldown determinism (no network needed).
