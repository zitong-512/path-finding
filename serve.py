"""Build every pathfinding algorithm and serve the animation in a browser."""

import argparse
from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
import json
import os
from pathlib import Path
import subprocess
import tempfile
import threading
import webbrowser

from build import build


def validate_rows(rows):
    if not isinstance(rows, list) or not rows or not all(isinstance(row, str) for row in rows):
        raise ValueError("Map must contain a nonempty list of row strings.")
    width = len(rows[0])
    if not width or any(len(row) != width for row in rows):
        raise ValueError("Map rows must be nonempty and equally wide.")
    if width * len(rows) > 4096:
        raise ValueError("Map must contain at most 4096 cells.")
    text = "".join(rows)
    if any(symbol not in "SG.#" for symbol in text):
        raise ValueError("Use only S, G, '.', and '#' in the map.")
    if text.count("S") != 1 or text.count("G") != 1:
        raise ValueError("Map must contain exactly one S and one G.")
    return rows


def run_map(project, rows):
    """Run the compiled backend in isolation; publish only complete results."""
    executable = project / ".build" / ("pathfinding.exe" if os.name == "nt" else "pathfinding")
    with tempfile.TemporaryDirectory(prefix="map-", dir=project / ".build") as folder:
        work = Path(folder)
        (work / "web").mkdir()
        map_file = work / "map.txt"
        map_file.write_text("\n".join(rows) + "\n", encoding="utf-8")
        subprocess.run([str(executable), str(map_file)], cwd=work, check=True,
                       capture_output=True, text=True, timeout=10)
        manifest = json.loads((work / "web" / "algorithms.json").read_text(encoding="utf-8"))
        runs = {}
        for item in manifest["algorithms"]:
            filename = f"run-{item['id']}.json"
            runs[item["id"]] = json.loads((work / "web" / filename).read_text(encoding="utf-8"))
        # Each replacement is atomic, so static clients never read partial JSON.
        for source in (work / "web").glob("*.json"):
            source.replace(project / "web" / source.name)
        return {"algorithms": manifest["algorithms"], "runs": runs}


class PathfindingHandler(SimpleHTTPRequestHandler):
    run_lock = threading.Lock()

    def __init__(self, *args, project, **kwargs):
        self.project = project
        super().__init__(*args, directory=str(project / "web"), **kwargs)

    def reply_json(self, code, value):
        body = json.dumps(value).encode("utf-8")
        self.send_response(code)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Cache-Control", "no-store")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_POST(self):
        if self.path != "/api/run":
            self.reply_json(404, {"error": "Unknown endpoint."})
            return
        if self.headers.get_content_type() != "application/json":
            self.reply_json(415, {"error": "Send the map as application/json."})
            return
        try:
            length = int(self.headers.get("Content-Length", "0"))
            if not 0 < length <= 32768:
                raise ValueError("Map request is empty or too large.")
            payload = json.loads(self.rfile.read(length))
            if not isinstance(payload, dict):
                raise ValueError("Expected a JSON object containing rows.")
            rows = validate_rows(payload.get("rows"))
        except (ValueError, UnicodeError) as error:
            self.reply_json(400, {"error": str(error)})
            return
        if not self.run_lock.acquire(blocking=False):
            self.reply_json(409, {"error": "Another map is running. Try again shortly."})
            return
        try:
            self.reply_json(200, run_map(self.project, rows))
        except subprocess.TimeoutExpired:
            self.reply_json(504, {"error": "An algorithm exceeded the 10-second limit. Your edits are kept."})
        except (OSError, ValueError, subprocess.CalledProcessError) as error:
            detail = error.stderr.strip() if isinstance(error, subprocess.CalledProcessError) else str(error)
            self.reply_json(500, {"error": f"Could not run this map: {detail}"})
        finally:
            self.run_lock.release()


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", type=int, default=8000)
    parser.add_argument("--no-browser", action="store_true")
    args = parser.parse_args()

    project = Path(__file__).resolve().parent
    try:
        build(project)
    except (RuntimeError, OSError, subprocess.CalledProcessError) as error:
        parser.exit(1, f"Build failed: {error}\n")

    handler = partial(PathfindingHandler, project=project)
    with ThreadingHTTPServer(("127.0.0.1", args.port), handler) as server:
        url = f"http://localhost:{args.port}/"
        print(f"Serving pathfinding at {url} (Ctrl+C to stop)", flush=True)
        if not args.no_browser:
            webbrowser.open(url)
        try:
            server.serve_forever()
        except KeyboardInterrupt:
            print("\nServer stopped.")


if __name__ == "__main__":
    main()
