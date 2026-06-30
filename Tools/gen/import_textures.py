"""Import generated icon PNGs (Content/UI/Icons/**) into UTexture2D assets at the same /Game path.
Run headless: UnrealEditor-Cmd Keldran.uproject -run=pythonscript -script=Tools/gen/import_textures.py
"""
import os
import unreal

# Gen pipeline writes icons to <repo>/Content/UI/Icons; the project is <repo>/Keldran.
REPO_CONTENT = os.path.normpath(os.path.join(unreal.Paths.project_dir(), "..", "Content"))
ICON_ROOT = os.path.join(REPO_CONTENT, "UI", "Icons")
tools = unreal.AssetToolsHelpers.get_asset_tools()

tasks = []
for root, _dirs, files in os.walk(ICON_ROOT):
    for f in files:
        if not f.lower().endswith(".png"):
            continue
        abs_png = os.path.join(root, f)
        rel = os.path.relpath(root, REPO_CONTENT).replace("\\", "/")  # e.g. UI/Icons/Items
        dest = "/Game/" + rel
        t = unreal.AssetImportTask()
        t.filename = abs_png
        t.destination_path = dest
        t.destination_name = os.path.splitext(f)[0]
        t.automated = True
        t.replace_existing = True
        t.save = True
        tasks.append(t)

if tasks:
    tools.import_asset_tasks(tasks)
    unreal.log(f"[tex] imported {len(tasks)} textures")
else:
    unreal.log("[tex] no PNGs found to import")
