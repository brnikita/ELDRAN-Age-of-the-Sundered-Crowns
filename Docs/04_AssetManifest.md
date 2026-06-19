# 04 — Asset Manifest

> **Every** external or generated asset is tracked here with its source, license, and target
> path. A **credits screen** is built from this file (M2/M3 task). The data-integrity automation
> test cross-checks that asset paths referenced by DataTables resolve. Mixed-source assets must
> pass the **coherence pass** (unified scale, material setup, shared post-process/lighting).

## How to use this file
- Add a row to the relevant table below whenever an asset is acquired or generated.
- `License` must be specific (CC0, CC-BY + attribution string, Epic Content License, Fab free
  tier, owned-generated, etc.).
- `Status`: `planned` | `acquired` | `imported` | `in-use`.
- Generated assets are also logged in `Docs/gen/manifest-<date>.jsonl` (with content hash);
  this file holds the human-readable summary + license.

## Free / open source libraries (allowed sources, spec §8)
| Need | Source | License | Notes |
|------|--------|---------|-------|
| Realistic humanoid characters | **MetaHuman** | Free for UE projects | Playable races + NPCs |
| Character animations | **Adobe Mixamo** | Free, royalty-free | Auto-rig + retarget to UE5/MetaHuman |
| Photoreal environments/props/surfaces/vegetation | **Quixel Megascans on Fab** | Fab free tier + legacy library (Epic Content License) | Use free subset; claimed assets are permanent |
| CC0 HDRIs/textures/models | **Poly Haven** | CC0 | Lighting, PBR, props |
| CC0 PBR materials | **ambientCG** | CC0 | Tileable surfaces |
| CC0/downloadable 3D | **Sketchfab** (filter CC0) | CC0 / CC-BY (per asset) | Gap-fill; record per-asset license |
| Free SFX | **Freesound** | CC (per asset) | Supplement ElevenLabs |
| Free music | **Incompetech** | CC-BY | Supplement ElevenLabs; attribution required |

**Generated content:** GPT Image 2 (2D), ElevenLabs (audio), OpenRouter (text) → license
`owned-generated` (subject to each provider's usage terms; ElevenLabs Music v2 is cleared for
commercial/gaming). Logged per `03_AIContentPipeline.md`.

---

## Characters & animations
| Asset | Source | License | Target path | Status |
|-------|--------|---------|-------------|--------|
| Freeborn Warden base (player) | MetaHuman | Free for UE | `Content/Characters/Player/Freeborn/` | planned |
| NPC: Halek Orne / Wend / Talia / Doran | MetaHuman | Free for UE | `Content/Characters/NPC/` | planned |
| Locomotion + combat anims (retarget) | Mixamo | Free royalty-free | `Content/Characters/Anims/` | planned |
| Creature meshes (Brambleback, Skitter, Hollow Straggler, Warden-of-the-Toe) | Sketchfab CC0 / Fab free / placeholder | per-asset | `Content/Characters/Creatures/` | planned |

## Environment — Vael's Rest (L1)
| Asset | Source | License | Target path | Status |
|-------|--------|---------|-------------|--------|
| Temperate forest vegetation/rocks/ground | Quixel/Fab free | Epic Content License | `Content/Environments/VaelsRest/` | planned |
| HDRIs (dawn/overcast) | Poly Haven | CC0 | `Content/Environments/HDRI/` | planned |
| Tileable PBR (stone/wood/dirt) | ambientCG / Poly Haven | CC0 | `Content/Textures/Surfaces/` | planned |
| Village structures/props | Quixel/Fab free / Sketchfab CC0 | per-asset | `Content/Environments/VaelsRest/Props/` | planned |

## UI / 2D (generated — GPT Image 2)
| Asset | Source | License | Target path | Status |
|-------|--------|---------|-------------|--------|
| Ability icons (basic-attack, shield-bash, defensive-stance) | GPT Image 2 | owned-generated | `Content/UI/Icons/Abilities/` | planned |
| Item icons (slice items) | GPT Image 2 | owned-generated | `Content/UI/Icons/Items/` | planned |
| Buff/debuff icons (stun, defensive) | GPT Image 2 | owned-generated | `Content/UI/Icons/Status/` | planned |
| UI frame set + Vael's Rest loading screen | GPT Image 2 | owned-generated | `Content/UI/Frames/`, `Content/UI/Loading/` | planned |

## Audio (generated — ElevenLabs; supplemented by CC)
| Asset | Source | License | Target path | Status |
|-------|--------|---------|-------------|--------|
| NPC VO + creature barks | ElevenLabs TTS/Dialogue | owned-generated | `Content/Audio/VO/` | planned |
| Ability/combat SFX | ElevenLabs SFX (`eleven_text_to_sound_v2`) | owned-generated | `Content/Audio/SFX/` | planned |
| Ambient woodland + Rift-tense layers, combat music | ElevenLabs Music v2 | owned-generated (commercial-cleared) | `Content/Audio/Music/` | planned |
| Supplemental SFX | Freesound | CC (per asset) | `Content/Audio/SFX/CC/` | planned |
| Supplemental music | Incompetech | CC-BY (attribution) | `Content/Audio/Music/CC/` | planned |

## Text / lore (generated — OpenRouter)
| Asset | Source | License | Target path | Status |
|-------|--------|---------|-------------|--------|
| Quest text, vendor flavor, bark pools | OpenRouter (build-time) | owned-generated | imported into `Content/Data/` tables | planned |

---

## Attribution & credits
Any CC-BY / per-asset-attributed entry must include its required attribution string in a
`Credits/attributions.csv` that feeds the in-game **credits screen** (built in M2 UI task,
populated continuously). CC0 assets need no attribution but are still listed for provenance.

## Coherence pass checklist (spec §8) — run when assets land in a zone
- [ ] Unified world scale (1 uu = 1 cm; characters ~180 uu).
- [ ] Shared master material / material instances; consistent roughness/spec baseline.
- [ ] One post-process + lighting profile per zone (`UZoneDataAsset.PostProcessProfile`).
- [ ] No source-mismatched texel density on hero assets.
- [ ] Audio loudness normalized; consistent reverb per space.
