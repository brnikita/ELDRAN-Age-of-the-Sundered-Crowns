"""Password hashing + session token (JWT) helpers, backed by Redis for revocation/TTL."""
import os
import time
import uuid
import jwt
from passlib.context import CryptContext

from .db import redis

# pbkdf2_sha256: pure-Python, no native bcrypt dependency (avoids the passlib/bcrypt 4.x
# 72-byte self-test crash) and no password length limit.
_pwd = CryptContext(schemes=["pbkdf2_sha256"], deprecated="auto")
JWT_SECRET = os.environ["JWT_SECRET"]
SESSION_TTL = int(os.environ.get("SESSION_TTL", "3600"))


def hash_password(plain: str) -> str:
    return _pwd.hash(plain)


def verify_password(plain: str, hashed: str) -> bool:
    return _pwd.verify(plain, hashed)


async def issue_session(account_id: str) -> str:
    """Create a JWT session token and record it in Redis with a TTL."""
    jti = str(uuid.uuid4())
    now = int(time.time())
    token = jwt.encode(
        {"sub": account_id, "jti": jti, "iat": now, "exp": now + SESSION_TTL},
        JWT_SECRET,
        algorithm="HS256",
    )
    await redis().set(f"session:{jti}", account_id, ex=SESSION_TTL)
    return token


async def verify_session(token: str) -> str | None:
    """Return account_id if the token is valid AND still present in Redis, else None."""
    try:
        payload = jwt.decode(token, JWT_SECRET, algorithms=["HS256"])
    except jwt.PyJWTError:
        return None
    jti = payload.get("jti")
    if not jti:
        return None
    account_id = await redis().get(f"session:{jti}")
    if account_id is None or account_id != payload.get("sub"):
        return None
    return account_id


async def issue_play_ticket(account_id: str, character_id: str) -> str:
    """Single-use short-lived ticket the client passes to the game server on travel."""
    ticket = str(uuid.uuid4())
    await redis().set(
        f"ticket:{ticket}", f"{account_id}:{character_id}", ex=60
    )
    return ticket


async def consume_play_ticket(ticket: str) -> tuple[str, str] | None:
    """Validate + delete a play ticket (single use). Returns (account_id, character_id)."""
    key = f"ticket:{ticket}"
    val = await redis().get(key)
    if val is None:
        return None
    await redis().delete(key)
    account_id, _, character_id = val.partition(":")
    return account_id, character_id
