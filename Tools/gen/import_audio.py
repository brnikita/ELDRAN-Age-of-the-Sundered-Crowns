"""Import generated WAV audio (Content/Audio/wav/**) into USoundWave assets under /Game/Audio/**.
SFX -> /Game/Audio/SFX/<name>, VO -> /Game/Audio/VO/<name>.

Run headless:
  UnrealEditor-Cmd Keldran.uproject -run=pythonscript -script=Tools/gen/import_audio.py
Results are written to Saved/import_audio_result.txt (unreal.log doesn't reach commandlet stdout).
"""
import os
import unreal

REPO_CONTENT = os.path.normpath(os.path.join(unreal.Paths.project_dir(), "..", "Content"))
WAV_ROOT = os.path.join(REPO_CONTENT, "Audio", "wav")
tools = unreal.AssetToolsHelpers.get_asset_tools()

tasks = []
for root, _dirs, files in os.walk(WAV_ROOT):
    for f in files:
        if not f.lower().endswith(".wav"):
            continue
        abs_wav = os.path.join(root, f)
        # .../Audio/wav/SFX -> SFX ; map to /Game/Audio/SFX
        sub = os.path.basename(root)
        dest = "/Game/Audio/" + sub
        t = unreal.AssetImportTask()
        t.filename = abs_wav
        t.destination_path = dest
        t.destination_name = os.path.splitext(f)[0]
        t.automated = True
        t.replace_existing = True
        t.save = True
        tasks.append((dest + "/" + os.path.splitext(f)[0], t))

imported = []
if tasks:
    tools.import_asset_tasks([t for _, t in tasks])
    for path, _ in tasks:
        imported.append(path)

result = os.path.join(unreal.Paths.project_saved_dir(), "import_audio_result.txt")
with open(result, "w", encoding="utf-8") as fh:
    fh.write(f"imported {len(imported)} sound waves\n")
    for p in imported:
        exists = unreal.EditorAssetLibrary.does_asset_exist(p)
        fh.write(f"  {'OK ' if exists else 'MISSING '}{p}\n")
unreal.log(f"[audio] imported {len(imported)} sound waves")
