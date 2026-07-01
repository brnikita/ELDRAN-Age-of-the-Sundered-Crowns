"""Create the Enhanced Input assets for the slice under /Game/Input:
  IA_Move (Axis2D), IA_Look (Axis2D), IA_Ability1/2/3 (Digital bool),
  IMC_Warden mapping WASD->Move, Mouse XY->Look, keys 1/2/3->abilities.

Run headless (editor context — the input-editor factories must be available):
  UnrealEditor-Cmd Keldran.uproject -run=pythonscript -script=<abs path>
Results -> Saved/build_input_result.txt
"""
import os
import unreal

tools = unreal.AssetToolsHelpers.get_asset_tools()
lib = unreal.EditorAssetLibrary
FOLDER = "/Game/Input"


def make_input_action(name, value_type):
    path = f"{FOLDER}/{name}"
    ia = lib.load_asset(path) if lib.does_asset_exist(path) else \
        tools.create_asset(name, FOLDER, unreal.InputAction, None)
    ia.set_editor_property("value_type", value_type)
    lib.save_loaded_asset(ia)
    return ia


DIGITAL = unreal.InputActionValueType.BOOLEAN
AXIS2D = unreal.InputActionValueType.AXIS2D

ia_move = make_input_action("IA_Move", AXIS2D)
ia_look = make_input_action("IA_Look", AXIS2D)
ia_ab1 = make_input_action("IA_Ability1", DIGITAL)
ia_ab2 = make_input_action("IA_Ability2", DIGITAL)
ia_ab3 = make_input_action("IA_Ability3", DIGITAL)

imc_path = f"{FOLDER}/IMC_Warden"
if lib.does_asset_exist(imc_path):
    imc = lib.load_asset(imc_path)
    imc.set_editor_property("mappings", [])  # clear before rebuilding
else:
    imc = tools.create_asset("IMC_Warden", FOLDER, unreal.InputMappingContext, None)


def K(name):
    k = unreal.Key()
    k.set_editor_property("key_name", name)
    return k


def mapping(action, key, modifiers=None):
    m = unreal.EnhancedActionKeyMapping()
    m.set_editor_property("action", action)
    m.set_editor_property("key", key)
    if modifiers:
        m.set_editor_property("modifiers", modifiers)
    return m


def swizzle():
    return unreal.InputModifierSwizzleAxis()


def negate():
    return unreal.InputModifierNegate()


# Movement: single keys drive a 2D axis; a key's default value lands on +X.
# W/S -> forward/back (Y) via SwizzleAxis (routes X->Y); A/D -> strafe (X).
mappings = [
    mapping(ia_move, K("W"), [swizzle()]),             # +Y
    mapping(ia_move, K("S"), [swizzle(), negate()]),   # -Y
    mapping(ia_move, K("D")),                           # +X
    mapping(ia_move, K("A"), [negate()]),              # -X
    mapping(ia_look, K("Mouse2D")),                     # yaw/pitch
    mapping(ia_ab1, K("One")),
    mapping(ia_ab2, K("Two")),
    mapping(ia_ab3, K("Three")),
]
imc.set_editor_property("mappings", mappings)
lib.save_loaded_asset(imc)

created = ["IA_Move", "IA_Look", "IA_Ability1", "IA_Ability2", "IA_Ability3", "IMC_Warden"]
result = os.path.join(unreal.Paths.project_saved_dir(), "build_input_result.txt")
with open(result, "w", encoding="utf-8") as fh:
    for n in created:
        p = f"{FOLDER}/{n}"
        fh.write(f"{'OK ' if lib.does_asset_exist(p) else 'MISSING '}{p}\n")
    # dump IMC mappings for verification
    for m in imc.get_editor_property("mappings"):
        act = m.get_editor_property("action")
        k = m.get_editor_property("key")
        mods = m.get_editor_property("modifiers")
        kn = k.get_editor_property("key_name")
        fh.write(f"  map {act.get_name() if act else '?'} <- {kn} "
                 f"mods={[type(x).__name__ for x in mods]}\n")
unreal.log("[input] input assets built")
