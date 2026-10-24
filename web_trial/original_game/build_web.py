"""Build the real game's saved sources with a browser-specific backend."""
from pathlib import Path
import json
import sys
import subprocess
import hashlib
import shutil
from concurrent.futures import ThreadPoolExecutor

here = Path(__file__).resolve().parent
source = here / "port"
source.mkdir(exist_ok=True)
manifest = json.loads((here / "manifest.json").read_text(encoding="utf-8"))
# Only initialize the port once; subsequent manual fixes are preserved.
for path in (here / "utf8").iterdir():
    dest = source / path.name
    if not dest.exists():
        dest.write_bytes(path.read_bytes())

main = source / "Main.cpp"
text = main.read_text(encoding="utf-8")
text = text.replace(
    "int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hRrevInstance, _In_ LPSTR IpCmdLine, _In_ int nShowCmd)",
    "int main()")
main.write_text(text, encoding="utf-8")
game = source / "Game.cpp"
text = game.read_text(encoding="utf-8").replace("frameTime = min(", "frameTime = std::min(")
start = text.index("void Game::InitConsole()")
end = text.index("//クリアシーン", start)
text = text[:start] + 'void Game::InitConsole() { /* Browser developer console is used. */ }\n\n' + text[end:]
game.write_text(text, encoding="utf-8")

# Port-only C++ portability fixes; leave the original snapshot unchanged.
fixes = {
    "PlayScene.h": [("~PlayScene() override = default;", "~PlayScene() override;")],
    "PlayerEntity.h": [("bool SetIsHide(bool flag) { m_isHide = flag; }",
                        "bool SetIsHide(bool flag) { m_isHide = flag; return m_isHide; }")],
    "PlayerEntity.cpp": [("#include <Windows.h>", "// Platform input is provided by the browser backend.")],
}
for name, changes in fixes.items():
    path = source / name
    content = path.read_text(encoding="utf-8")
    for old, new in changes:
        content = content.replace(old, new)
    path.write_text(content, encoding="utf-8")
# Attacks may spawn drops, which reallocates Scene's actor vector.
path = source / "PlayerEntity.cpp"
content = path.read_text(encoding="utf-8")
start = content.index("void PlayerEntity::CheckAttackHit(")
end = content.index("void PlayerEntity::UpdateState", start)
part = content[start:end].replace(
    "    for (Actor* actor : m_scene->GetActors())\n    {",
    "    // Damage can spawn drops and reallocate the scene actor vector.\n"
    "    // Keep this attack's targets stable until collision processing finishes.\n"
    "    const auto attackTargets = m_scene->GetActors();\n"
    "    for (Actor* actor : attackTargets)\n    {\n"
    "        if (!actor || actor->IsDead()) continue;")
path.write_text(content[:start] + part + content[end:], encoding="utf-8")
path = source / "PlayScene.cpp"
content = path.read_text(encoding="utf-8")
if "PlayScene::~PlayScene()" not in content:
    path.write_text(content + "\nPlayScene::~PlayScene() = default;\n", encoding="utf-8")

out = here / "build/web"
out.mkdir(parents=True, exist_ok=True)
assets = here / "web-assets"
if not assets.exists():
    shutil.copytree(here / "snapshot/assets", assets)
    for path in assets.rglob("*"):
        if path.suffix.lower() not in (".csv", ".txt", ".json"):
            continue
        raw = path.read_bytes()
        try:
            content = raw.decode("utf-8-sig")
        except UnicodeDecodeError:
            content = raw.decode("cp932")
        path.write_bytes(content.encode("utf-8"))
args = [str(here.parent / "tools/emsdk/upstream/emscripten/em++.exe"),
        "-std=c++17", "-O1", "-fexceptions", "-ferror-limit=6",
        "-I", str(here / "web"), "-I", str(source), "-I", str(here / "vendor/include"),
        "-include", "typeinfo", "-include", "algorithm",
        "-sASYNCIFY", "-sASYNCIFY_STACK_SIZE=65536", "-sALLOW_MEMORY_GROWTH", "-sSTACK_SIZE=4194304",
        "--emit-symbol-map", "-sEXPORTED_RUNTIME_METHODS=callMain", "-sASSERTIONS=1",
        "--preload-file", str(assets) + "@/assets",
        "--preload-file", str(here / "snapshot/save") + "@/save",
        "--shell-file", str(here / "web/shell.html"),
        str(here / "web/bridge.cpp")]
args += [str(source / name) for name in manifest["sources"]]
args += ["-o", str(out / "index.html")]
# Keep object files across attempts; recompile whenever source or headers change.
base = args[:args.index("-sASYNCIFY")]
if "--asan" in sys.argv:
    base += ["-fsanitize=address", "-g"]
objects = out / "objects"
objects.mkdir(exist_ok=True)
header_hash = hashlib.sha256(b"".join(p.read_bytes() for folder in (source, here / "web")
    for p in sorted(folder.glob("*.h")))).digest()
units = [here / "web/bridge.cpp"] + [source / n for n in manifest["sources"]]
regression = "--regression" in sys.argv
if regression:
    units = [p for p in units if p.name != "Main.cpp"] + [here / "web/combat_regression.cpp"]
def compile_unit(path):
    obj = objects / (path.stem + ".o")
    stamp = obj.with_suffix(".sha")
    signature = hashlib.sha256(header_hash + path.read_bytes() + str(base).encode()).hexdigest()
    if obj.exists() and stamp.exists() and stamp.read_text() == signature:
        return 0, ""
    result = subprocess.run(base + ["-c", str(path), "-o", str(obj)],
        cwd=here, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    if result.returncode == 0:
        stamp.write_text(signature)
    return result.returncode, result.stdout.decode("utf-8", errors="replace")
with ThreadPoolExecutor(max_workers=4) as pool:
    results = list(pool.map(compile_unit, units))
if any(code for code, _ in results):
    report = "".join(text for _, text in results)
    (here / "logs/web-build.log").write_text(report, encoding="utf-8")
    print("\n".join(line for line in report.splitlines() if "error:" in line))
    raise SystemExit(1)
link = args[args.index("-sASYNCIFY"):args.index(str(here / "web/bridge.cpp"))]
args = base + link + [str(objects / (p.stem + ".o")) for p in units] + ["-o", str(out / ("regression.html" if regression else "index.html"))]
with (here / "logs/web-build.log").open("w", encoding="utf-8") as log:
    result = subprocess.run(args, cwd=here, stdout=log, stderr=subprocess.STDOUT)
print("Web build exit:", result.returncode)
print((here / "logs/web-build.log").read_text(encoding="utf-8", errors="replace")[-10000:])
raise SystemExit(result.returncode)
