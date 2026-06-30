"""Build a minimal playable L1 Vael's Rest test map (headless). Floor + lights + PlayerStart +
VaelsRest GameMode + placed mobs/NPC/marker wired to the imported DataTables. Placeholder
geometry (engine basic shapes) — visual polish (MetaHuman/Quixel) is M3.

Run: UnrealEditor-Cmd Keldran.uproject -run=pythonscript -script="Tools/gen/build_slice_map.py"
       -unattended -nullrhi -nosound -stdout
"""
import unreal

MAP_PKG = "/Game/Maps/L1_VaelsRest"

les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

DT_Mobs = unreal.load_asset("/Game/Data/DT_Mobs")
DT_Loot = unreal.load_asset("/Game/Data/DT_LootTables")
DT_NPCs = unreal.load_asset("/Game/Data/DT_NPCs")
GameMode = unreal.load_class(None, "/Script/Keldran.VaelsRestGameMode")
MobClass = unreal.load_class(None, "/Script/KeldranAI.KeldranMobCharacter")
AICtrl = unreal.load_class(None, "/Script/KeldranAI.KeldranAIController")
NpcClass = unreal.load_class(None, "/Script/KeldranQuests.KeldranNpcCharacter")
MarkerClass = unreal.load_class(None, "/Script/KeldranQuests.KeldranAreaMarker")


def v(x, y, z):
    return unreal.Vector(x, y, z)


def spawn(cls, loc, rot=None):
    return eas.spawn_actor_from_class(cls, loc, rot or unreal.Rotator(0, 0, 0))


def build():
    les.new_level(MAP_PKG)

    # --- Environment: floor + lights so it renders and pawns have ground ---
    plane = unreal.load_asset("/Engine/BasicShapes/Plane")
    floor = spawn(unreal.StaticMeshActor, v(0, 0, 0))
    floor.static_mesh_component.set_static_mesh(plane)
    floor.set_actor_scale3d(unreal.Vector(80, 80, 1))  # ~80m x 80m

    spawn(unreal.DirectionalLight, v(0, 0, 800), unreal.Rotator(-45, 30, 0))
    spawn(unreal.SkyLight, v(0, 0, 600))
    spawn(unreal.SkyAtmosphere, v(0, 0, 0))
    spawn(unreal.PlayerStart, v(0, 0, 120))

    # --- Creatures (Thornwood Path) wired to DT_Mobs + DT_LootTables ---
    for i, row in enumerate(["mob.brambleback", "mob.brambleback", "mob.brambleback",
                             "mob.mire-skitter", "mob.hollow-straggler"]):
        mob = spawn(MobClass, v(600 + i * 350, 200, 120))
        if mob:
            mob.set_editor_property("mob_table", DT_Mobs)
            mob.set_editor_property("mob_row", unreal.Name(row))
            mob.set_editor_property("loot_table_asset", DT_Loot)
            mob.set_editor_property("auto_possess_ai", unreal.AutoPossessAI.PLACED_IN_WORLD)
            mob.set_editor_property("ai_controller_class", AICtrl)

    # --- Mini-boss (Quiet Glade) ---
    boss = spawn(MobClass, v(2200, -400, 120))
    if boss:
        boss.set_editor_property("mob_table", DT_Mobs)
        boss.set_editor_property("mob_row", unreal.Name("mob.warden-of-the-toe"))
        boss.set_editor_property("loot_table_asset", DT_Loot)
        boss.set_editor_property("auto_possess_ai", unreal.AutoPossessAI.PLACED_IN_WORLD)
        boss.set_editor_property("ai_controller_class", AICtrl)

    # --- NPC quest-giver ---
    npc = spawn(NpcClass, v(-200, 0, 120))
    if npc:
        npc.set_editor_property("npc_table", DT_NPCs)
        npc.set_editor_property("npc_row", unreal.Name("npc.halek-orne"))

    # --- Reach marker (trailhead) ---
    marker = spawn(MarkerClass, v(400, 200, 120))
    if marker:
        marker.set_editor_property("marker_id", unreal.Name("marker.thornwood-trailhead"))
        marker.set_actor_scale3d(unreal.Vector(3, 3, 3))

    # --- GameMode override on world settings ---
    ws = unreal.GameplayStatics.get_game_mode(unreal.EditorLevelLibrary.get_editor_world())
    world = unreal.EditorLevelLibrary.get_editor_world()
    world.get_world_settings().set_editor_property("default_game_mode", GameMode)

    les.save_current_level()
    unreal.log("[map] L1_VaelsRest built and saved.")


build()
