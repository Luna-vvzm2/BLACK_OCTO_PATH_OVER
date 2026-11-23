"""Compile saved translation units separately to identify actual blockers."""
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
import json
import re
import subprocess

here = Path(__file__).resolve().parent
manifest = json.loads((here / "manifest.json").read_text(encoding="utf-8"))
compiler = here.parent / "tools/emsdk/upstream/emscripten/em++.exe"
out = here / "build/units"
logs = here / "logs/units"
out.mkdir(parents=True, exist_ok=True)
logs.mkdir(parents=True, exist_ok=True)


def check(name):
    args = [str(compiler), "-std=c++17", "-fexceptions", "-ferror-limit=1",
            "-I", str(here / "utf8"), "-I", str(here / "vendor/dxlib-utf8"),
            "-I", str(here / "vendor/include"),
            "-c", str(here / "utf8" / name), "-o", str(out / (Path(name).stem + ".o"))]
    proc = subprocess.run(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    text = proc.stdout.decode("utf-8", errors="replace")
    (logs / (Path(name).stem + ".log")).write_text(text, encoding="utf-8")
    match = re.search(r"(?:fatal )?error: (.+)", text)
    return {"file": name, "exit_code": proc.returncode,
            "first_error": match.group(1) if match else None}


with ThreadPoolExecutor(max_workers=4) as pool:
    results = list(pool.map(check, manifest["sources"] + manifest["unregistered_cpp"]))
(here / "unit-results.json").write_text(json.dumps(results, ensure_ascii=False, indent=2), encoding="utf-8")
passed = [r["file"] for r in results if not r["exit_code"]]
print("Compiled units:", passed)
print("Failed units:", len(results) - len(passed))
print("Unique first errors:", sorted({r["first_error"] for r in results if r["first_error"]}))
