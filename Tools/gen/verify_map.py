import unreal
import os

les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
ues = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)

les.load_level("/Game/Maps/L1_VaelsRest")

lines = []
actors = eas.get_all_level_actors()
lines.append(f"total_actors: {len(actors)}")
counts = {}
for a in actors:
    cn = a.get_class().get_name()
    counts[cn] = counts.get(cn, 0) + 1
for k, v in sorted(counts.items()):
    lines.append(f"  {k}: {v}")

world = ues.get_editor_world()
gm = world.get_world_settings().get_editor_property("default_game_mode")
lines.append(f"default_game_mode: {gm.get_name() if gm else 'NONE'}")

out = os.path.join(unreal.Paths.project_saved_dir(), "verify_result.txt")
with open(out, "w", encoding="utf-8") as f:
    f.write("\n".join(lines))
