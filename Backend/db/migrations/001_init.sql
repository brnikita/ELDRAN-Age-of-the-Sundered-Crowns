-- KELDRAN initial schema (matches Docs/02_NetworkDesign.md §5).
-- Applied automatically by the postgres container on first init.

CREATE EXTENSION IF NOT EXISTS "pgcrypto"; -- for gen_random_uuid()

-- ---- Accounts ----
CREATE TABLE IF NOT EXISTS accounts (
    account_id  UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    email       TEXT UNIQUE NOT NULL,
    pw_hash     TEXT NOT NULL,
    created_at  TIMESTAMPTZ NOT NULL DEFAULT now()
);

-- ---- Characters ----
CREATE TABLE IF NOT EXISTS characters (
    character_id UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    account_id   UUID NOT NULL REFERENCES accounts(account_id) ON DELETE CASCADE,
    name         TEXT UNIQUE NOT NULL,
    race         TEXT NOT NULL,
    class        TEXT NOT NULL,
    level        INT  NOT NULL DEFAULT 1,
    xp           BIGINT NOT NULL DEFAULT 0,
    zone_id      TEXT NOT NULL DEFAULT 'zone.vaels-rest',
    pos_x        DOUBLE PRECISION NOT NULL DEFAULT 0,
    pos_y        DOUBLE PRECISION NOT NULL DEFAULT 0,
    pos_z        DOUBLE PRECISION NOT NULL DEFAULT 0,
    rot_yaw      DOUBLE PRECISION NOT NULL DEFAULT 0,
    appearance   JSONB NOT NULL DEFAULT '{}'::jsonb,
    created_at   TIMESTAMPTZ NOT NULL DEFAULT now(),
    updated_at   TIMESTAMPTZ NOT NULL DEFAULT now()
);
CREATE INDEX IF NOT EXISTS idx_characters_account ON characters(account_id);

-- ---- Inventory ----
CREATE TABLE IF NOT EXISTS inventory (
    id           BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
    character_id UUID NOT NULL REFERENCES characters(character_id) ON DELETE CASCADE,
    item_row     TEXT NOT NULL,
    qty          INT  NOT NULL DEFAULT 1,
    instance_id  UUID NOT NULL DEFAULT gen_random_uuid(),
    slot         INT  NOT NULL DEFAULT -1
);
CREATE INDEX IF NOT EXISTS idx_inventory_character ON inventory(character_id);

-- ---- Equipment ----
CREATE TABLE IF NOT EXISTS equipment (
    character_id UUID NOT NULL REFERENCES characters(character_id) ON DELETE CASCADE,
    slot         TEXT NOT NULL,
    instance_id  UUID,
    PRIMARY KEY (character_id, slot)
);

-- ---- Quest progress ----
CREATE TABLE IF NOT EXISTS quest_progress (
    character_id UUID NOT NULL REFERENCES characters(character_id) ON DELETE CASCADE,
    quest_row    TEXT NOT NULL,
    status       TEXT NOT NULL DEFAULT 'active', -- active | complete | turned_in
    objectives   JSONB NOT NULL DEFAULT '[]'::jsonb,
    updated_at   TIMESTAMPTZ NOT NULL DEFAULT now(),
    PRIMARY KEY (character_id, quest_row)
);

-- ---- Currency ----
CREATE TABLE IF NOT EXISTS currency (
    character_id UUID PRIMARY KEY REFERENCES characters(character_id) ON DELETE CASCADE,
    coin         BIGINT NOT NULL DEFAULT 0,
    shards       BIGINT NOT NULL DEFAULT 0
);
