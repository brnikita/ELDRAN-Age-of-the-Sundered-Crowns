"""KELDRAN Login/Account Gateway.

HTTP API the UE client and dedicated server use (Docs/02_NetworkDesign.md §2).
- Client:  register, login, list/create characters, request play (-> shard + ticket).
- Server:  verify ticket on PreLogin, load/save character snapshots (persistence API).

Server-authoritative: the gateway only issues credentials and stores persistent data.
All gameplay validation happens on the dedicated server.
"""
import os
from contextlib import asynccontextmanager
from uuid import UUID

from fastapi import FastAPI, HTTPException, Header
from pydantic import BaseModel, Field

from . import db, security

DEFAULT_SHARD = os.environ.get("DEFAULT_SHARD_ADDRESS", "127.0.0.1:7777")


@asynccontextmanager
async def lifespan(_app: FastAPI):
    await db.init_pools()
    yield
    await db.close_pools()


app = FastAPI(title="KELDRAN Gateway", lifespan=lifespan)


# ---------- models ----------
class RegisterReq(BaseModel):
    email: str
    password: str = Field(min_length=6)


class LoginReq(BaseModel):
    email: str
    password: str


class CreateCharReq(BaseModel):
    name: str = Field(min_length=2, max_length=24)
    race: str
    char_class: str = Field(alias="class")
    appearance: dict = {}


class PlayReq(BaseModel):
    character_id: str


class VerifyReq(BaseModel):
    ticket: str


class SaveReq(BaseModel):
    character_id: str
    level: int
    xp: int
    zone_id: str
    pos: list[float]  # [x, y, z]
    rot_yaw: float
    inventory: list[dict]      # [{item_row, qty, instance_id, slot}]
    equipment: list[dict]      # [{slot, instance_id}]
    quests: list[dict]         # [{quest_row, status, objectives}]
    coin: int = 0
    shards: int = 0


# ---------- helpers ----------
async def _account_from_bearer(authorization: str | None) -> str:
    if not authorization or not authorization.startswith("Bearer "):
        raise HTTPException(401, "missing bearer token")
    account_id = await security.verify_session(authorization[7:])
    if account_id is None:
        raise HTTPException(401, "invalid or expired session")
    return account_id


# ---------- health ----------
@app.get("/health")
async def health():
    try:
        async with db.pool().acquire() as con:
            await con.execute("SELECT 1")
        await db.redis().ping()
        return {"status": "ok"}
    except Exception as e:  # noqa: BLE001
        raise HTTPException(503, f"unhealthy: {e}")


# ---------- auth ----------
@app.post("/auth/register")
async def register(req: RegisterReq):
    async with db.pool().acquire() as con:
        exists = await con.fetchval("SELECT 1 FROM accounts WHERE email=$1", req.email)
        if exists:
            raise HTTPException(409, "email already registered")
        account_id = await con.fetchval(
            "INSERT INTO accounts(email, pw_hash) VALUES($1,$2) RETURNING account_id",
            req.email, security.hash_password(req.password),
        )
    return {"account_id": str(account_id)}


@app.post("/auth/login")
async def login(req: LoginReq):
    async with db.pool().acquire() as con:
        row = await con.fetchrow(
            "SELECT account_id, pw_hash FROM accounts WHERE email=$1", req.email
        )
    if not row or not security.verify_password(req.password, row["pw_hash"]):
        raise HTTPException(401, "bad credentials")
    account_id = str(row["account_id"])
    token = await security.issue_session(account_id)
    return {"session_token": token, "account_id": account_id}


@app.post("/auth/verify")
async def verify(req: VerifyReq):
    """Called by the dedicated server in PreLogin to validate a play ticket (single use)."""
    result = await security.consume_play_ticket(req.ticket)
    if result is None:
        raise HTTPException(401, "invalid or expired ticket")
    account_id, character_id = result
    return {"account_id": account_id, "character_id": character_id}


# ---------- characters ----------
@app.get("/characters")
async def list_characters(authorization: str | None = Header(default=None)):
    account_id = await _account_from_bearer(authorization)
    async with db.pool().acquire() as con:
        rows = await con.fetch(
            "SELECT character_id, name, race, class, level, xp, zone_id "
            "FROM characters WHERE account_id=$1 ORDER BY created_at", account_id
        )
    return {"characters": [dict(r) | {"character_id": str(r["character_id"])} for r in rows]}


@app.post("/characters")
async def create_character(req: CreateCharReq, authorization: str | None = Header(default=None)):
    account_id = await _account_from_bearer(authorization)
    async with db.pool().acquire() as con:
        taken = await con.fetchval("SELECT 1 FROM characters WHERE name=$1", req.name)
        if taken:
            raise HTTPException(409, "name taken")
        char_id = await con.fetchval(
            "INSERT INTO characters(account_id,name,race,class,appearance) "
            "VALUES($1,$2,$3,$4,$5) RETURNING character_id",
            account_id, req.name, req.race, req.char_class, __import__("json").dumps(req.appearance),
        )
        await con.execute("INSERT INTO currency(character_id) VALUES($1)", char_id)
    return {"character_id": str(char_id)}


@app.post("/play")
async def play(req: PlayReq, authorization: str | None = Header(default=None)):
    account_id = await _account_from_bearer(authorization)
    async with db.pool().acquire() as con:
        row = await con.fetchrow(
            "SELECT zone_id FROM characters WHERE character_id=$1 AND account_id=$2",
            UUID(req.character_id), account_id,
        )
    if not row:
        raise HTTPException(404, "character not found")
    ticket = await security.issue_play_ticket(account_id, req.character_id)
    return {"shard_address": DEFAULT_SHARD, "ticket": ticket, "zone_id": row["zone_id"]}


# ---------- persistence API (server-side) ----------
@app.get("/persistence/{character_id}")
async def load_character(character_id: str):
    async with db.pool().acquire() as con:
        cid = UUID(character_id)
        char = await con.fetchrow("SELECT * FROM characters WHERE character_id=$1", cid)
        if not char:
            raise HTTPException(404, "character not found")
        inv = await con.fetch(
            "SELECT item_row,qty,instance_id,slot FROM inventory WHERE character_id=$1", cid)
        equ = await con.fetch(
            "SELECT slot,instance_id FROM equipment WHERE character_id=$1", cid)
        qst = await con.fetch(
            "SELECT quest_row,status,objectives FROM quest_progress WHERE character_id=$1", cid)
        cur = await con.fetchrow("SELECT coin,shards FROM currency WHERE character_id=$1", cid)
    return {
        "character_id": character_id,
        "name": char["name"], "race": char["race"], "class": char["class"],
        "level": char["level"], "xp": char["xp"], "zone_id": char["zone_id"],
        "pos": [char["pos_x"], char["pos_y"], char["pos_z"]], "rot_yaw": char["rot_yaw"],
        "appearance": char["appearance"],
        "inventory": [dict(r) | {"instance_id": str(r["instance_id"])} for r in inv],
        "equipment": [dict(r) | {"instance_id": str(r["instance_id"]) if r["instance_id"] else None} for r in equ],
        "quests": [dict(r) for r in qst],
        "coin": cur["coin"] if cur else 0, "shards": cur["shards"] if cur else 0,
    }


@app.post("/persistence/save")
async def save_character(req: SaveReq):
    """Full snapshot save in one transaction (autosave + on logout). Server calls this."""
    import json
    cid = UUID(req.character_id)
    async with db.pool().acquire() as con:
        async with con.transaction():
            await con.execute(
                "UPDATE characters SET level=$2,xp=$3,zone_id=$4,pos_x=$5,pos_y=$6,pos_z=$7,"
                "rot_yaw=$8,updated_at=now() WHERE character_id=$1",
                cid, req.level, req.xp, req.zone_id, req.pos[0], req.pos[1], req.pos[2], req.rot_yaw,
            )
            await con.execute("DELETE FROM inventory WHERE character_id=$1", cid)
            for it in req.inventory:
                await con.execute(
                    "INSERT INTO inventory(character_id,item_row,qty,instance_id,slot) "
                    "VALUES($1,$2,$3,$4,$5)",
                    cid, it["item_row"], it.get("qty", 1),
                    UUID(it["instance_id"]) if it.get("instance_id") else __import__("uuid").uuid4(),
                    it.get("slot", -1),
                )
            await con.execute("DELETE FROM equipment WHERE character_id=$1", cid)
            for eq in req.equipment:
                await con.execute(
                    "INSERT INTO equipment(character_id,slot,instance_id) VALUES($1,$2,$3)",
                    cid, eq["slot"], UUID(eq["instance_id"]) if eq.get("instance_id") else None,
                )
            for q in req.quests:
                await con.execute(
                    "INSERT INTO quest_progress(character_id,quest_row,status,objectives,updated_at) "
                    "VALUES($1,$2,$3,$4,now()) "
                    "ON CONFLICT (character_id,quest_row) DO UPDATE SET "
                    "status=EXCLUDED.status,objectives=EXCLUDED.objectives,updated_at=now()",
                    cid, q["quest_row"], q.get("status", "active"), json.dumps(q.get("objectives", [])),
                )
            await con.execute(
                "INSERT INTO currency(character_id,coin,shards) VALUES($1,$2,$3) "
                "ON CONFLICT (character_id) DO UPDATE SET coin=EXCLUDED.coin,shards=EXCLUDED.shards",
                cid, req.coin, req.shards,
            )
    return {"status": "saved"}
