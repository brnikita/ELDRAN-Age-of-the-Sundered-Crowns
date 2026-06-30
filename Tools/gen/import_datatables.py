"""Import the slice DataTable JSON (Content/Data/Source/*.json) into /Game/Data/DT_* assets.
Run headless via:
  UnrealEditor-Cmd Keldran.uproject -run=pythonscript -script="Tools/gen/import_datatables.py"
    -unattended -nullrhi -nosound -stdout
"""
import os
import unreal

# (asset_name, struct script path, source json filename)
TABLES = [
    ("DT_Items",       "/Script/KeldranInventory.ItemRow",      "Items.json"),
    ("DT_Mobs",        "/Script/KeldranAI.MobRow",              "Mobs.json"),
    ("DT_LootTables",  "/Script/KeldranInventory.LootTableRow", "LootTables.json"),
    ("DT_Quests",      "/Script/KeldranQuests.QuestRow",        "Quests.json"),
    ("DT_Vendors",     "/Script/KeldranInventory.VendorRow",    "Vendors.json"),
    ("DT_Abilities",   "/Script/KeldranAbilities.AbilityRow",   "Abilities.json"),
    ("DT_NPCs",        "/Script/KeldranQuests.NPCRow",          "NPCs.json"),
    ("DT_Barks",       "/Script/KeldranQuests.BarkRow",         "Barks.json"),
]

DEST_DIR = "/Game/Data"
SRC_DIR = os.path.join(unreal.Paths.project_content_dir(), "Data", "Source")

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()


def import_one(asset_name, struct_path, json_file):
    struct = unreal.load_object(None, struct_path)
    if struct is None:
        unreal.log_error(f"[import] struct not found: {struct_path}")
        return False

    pkg = f"{DEST_DIR}/{asset_name}"
    if unreal.EditorAssetLibrary.does_asset_exist(pkg):
        unreal.EditorAssetLibrary.delete_asset(pkg)

    factory = unreal.DataTableFactory()
    factory.set_editor_property("struct", struct)
    dt = asset_tools.create_asset(asset_name, DEST_DIR, unreal.DataTable, factory)
    if dt is None:
        unreal.log_error(f"[import] failed to create asset: {pkg}")
        return False

    src = os.path.join(SRC_DIR, json_file)
    with open(src, "r", encoding="utf-8") as f:
        json_str = f.read()

    problems = unreal.DataTableFunctionLibrary.fill_data_table_from_json_string(dt, json_str)
    unreal.EditorAssetLibrary.save_loaded_asset(dt)
    row_count = len(unreal.DataTableFunctionLibrary.get_data_table_row_names(dt))
    unreal.log(f"[import] {asset_name}: {row_count} rows  (problems: {problems or 'none'})")
    return True


def main():
    ok = 0
    for name, struct_path, json_file in TABLES:
        try:
            if import_one(name, struct_path, json_file):
                ok += 1
        except Exception as e:  # noqa: BLE001
            unreal.log_error(f"[import] {name} EXCEPTION: {e}")
    unreal.log(f"[import] done: {ok}/{len(TABLES)} tables imported")


main()
