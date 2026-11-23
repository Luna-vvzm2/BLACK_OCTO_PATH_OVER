"""Snapshot the original game and attempt a real WebAssembly build.

Never modifies the source game. Refuses to overwrite an existing snapshot.
Use --build-only to repeat the build against the saved snapshot.
"""
from pathlib import Path
import hashlib
import json
import shutil
import subprocess
import sys
import xml.etree.ElementTree as ET

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent.parent
SDK = HERE.parent / "tools/emsdk"
DX = Path(r"C:\DxLib_VC\プロジェクトに追加すべきファイル_VC用")
NS = {"m": "http://schemas.microsoft.com/developer/msbuild/2003"}


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def utf8_copy(source, dest):
    raw = source.read_bytes()
    try:
        text = raw.decode("utf-8-sig")
        encoding = "utf-8"
    except UnicodeDecodeError:
        text = raw.decode("cp932")
        encoding = "cp932"
    dest.parent.mkdir(parents=True, exist_ok=True)
    dest.write_bytes(text.encode("utf-8"))
    return encoding


def prepare():
    snap = HERE / "snapshot"
    if snap.exists():
        raise SystemExit("Snapshot already exists. Use --build-only; no files overwritten.")
    project = ROOT / "SHINOBI_class.vcxproj"
    tree = ET.parse(project)
    sources, excluded = [], []
    for item in tree.findall(".//m:ItemGroup/m:ClCompile", NS):
        name = item.get("Include")
        if not name:
            continue
        skip = any(
            x.text == "true" and "Debug|x64" in x.get("Condition", "")
            for x in item.findall("m:ExcludedFromBuild", NS)
        )
        (excluded if skip else sources).append(name.replace("\\", "/"))
    # Preserve all game source files, including newly added files not registered yet.
    files = sorted(set(ROOT.glob("*.cpp")) | set(ROOT.glob("*.h")) |
                   set(ROOT.glob("*.sln")) | set(ROOT.glob("*.vcxproj*")) |
                   set(ROOT.glob("vcpkg*.json")))
    snap.mkdir(parents=True)
    records = {}
    for source in files:
        dest = snap / source.name
        shutil.copy2(source, dest)
        records[source.name] = {"sha256": digest(source)}
        if source.suffix in (".cpp", ".h"):
            records[source.name]["encoding"] = utf8_copy(source, HERE / "utf8" / source.name)
    for folder in ("assets", "save"):
        if (ROOT / folder).is_dir():
            shutil.copytree(ROOT / folder, snap / folder)
    for source in DX.glob("*.h"):
        # Keep exact originals and an encoding-normalized build copy separately.
        target = HERE / "vendor/dxlib-original" / source.name
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, target)
        utf8_copy(source, HERE / "vendor/dxlib-utf8" / source.name)
    json_headers = ROOT / "vcpkg_installed/x64-windows/include/nlohmann"
    if json_headers.is_dir():
        shutil.copytree(json_headers, HERE / "vendor/include/nlohmann")
    manifest = {
        "configuration": "Debug|x64", "sources": sources, "excluded": excluded,
        "files": records,
        "unregistered_cpp": sorted(p.name for p in ROOT.glob("*.cpp") if p.name not in sources + excluded),
        "branch": subprocess.check_output(["git", "branch", "--show-current"], cwd=ROOT, text=True).strip(),
    }
    (HERE / "manifest.json").write_text(json.dumps(manifest, ensure_ascii=False, indent=2), encoding="utf-8")
    print(f"Snapshot: {len(files)} files; {len(sources)} project translation units.", flush=True)
    return manifest


def build(manifest):
    (HERE / "logs").mkdir(exist_ok=True)
    (HERE / "build").mkdir(exist_ok=True)
    compiler = SDK / "upstream/emscripten/em++.exe"
    # A direct build and a second attempt after lossless text-encoding conversion.
    # No placeholder implementations or disabled gameplay are introduced.
    for label, folder, dx_folder in (
        ("original", "snapshot", "dxlib-original"),
        ("utf8", "utf8", "dxlib-utf8"),
    ):
        args = [str(compiler), "-std=c++17", "-O0", "-fexceptions", "-ferror-limit=3",
                "-I", str(HERE / folder), "-I", str(HERE / "vendor" / dx_folder),
                "-I", str(HERE / "vendor/include")]
        args += [str(HERE / folder / name) for name in manifest["sources"]]
        args += ["--preload-file", str(HERE / "snapshot/assets") + "@/assets",
                 "-o", str(HERE / "build" / (label + ".html"))]
        (HERE / "logs" / (label + "-command.json")).write_text(
            json.dumps(args, ensure_ascii=False, indent=2), encoding="utf-8")
        with (HERE / "logs" / (label + ".log")).open("w", encoding="utf-8") as output:
            result = subprocess.run(args, cwd=HERE, stdout=output, stderr=subprocess.STDOUT)
        print(f"{label} complete: exit={result.returncode}; log=logs/{label}.log", flush=True)
    changed = [name for name, item in manifest["files"].items() if digest(ROOT / name) != item["sha256"]]
    print("Original source changes since snapshot:", changed, flush=True)


if __name__ == "__main__":
    manifest = json.loads((HERE / "manifest.json").read_text(encoding="utf-8")) if "--build-only" in sys.argv else prepare()
    build(manifest)
