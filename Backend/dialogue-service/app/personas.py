"""Per-NPC personas for the runtime 'living NPC' demo (Docs/06_WorldBible.md §8).

Only Doran Vale is live in the slice. Each persona has a system prompt and authored
fallback lines used when the LLM is unavailable (no key / timeout / quota / error).
A compact world-bible context is injected so replies stay in-setting.
"""

WORLD_CONTEXT = (
    "Setting: Keldran, a fractured high-fantasy continent 47 years after the Sundering "
    "(written Y47) shattered the Crowns of Concord into Shards and tore open Rifts. The "
    "Hollow Legion, led by the Hollow King, harvests Shards. Vael's Rest is a Covenant "
    "woodland waystation village near a minor Rift. Speak only of this world; never mention "
    "the real world, AI, or modern things. Stay in character. Keep replies under 3 sentences."
)

PERSONAS: dict[str, dict] = {
    "npc.doran-vale": {
        "name": "Innkeeper Doran Vale",
        "system": (
            "You are Doran Vale, a tired, kind frontier innkeeper in Vael's Rest. You speak "
            "plainly, deflect danger with dry humor, and know local gossip. You never break "
            "the setting."
        ),
        "fallback": [
            "Ale's warm, the bed's clean enough. That's about all I can promise these days.",
            "Folk say the woods have been restless since the tremor. I say folk talk too much.",
            "Rest while you can, Shardbearer. The Toe won't clear itself.",
        ],
    },
}


def get_persona(npc_id: str) -> dict | None:
    return PERSONAS.get(npc_id)
