"""Build every pathfinding algorithm and serve the animation in a browser."""

import argparse
from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
import subprocess
import webbrowser


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", type=int, default=8000)
    parser.add_argument("--no-browser", action="store_true")
    args = parser.parse_args()

    project = Path(__file__).resolve().parent
    build_script = project / "build-and-run.ps1"
    subprocess.run(
        ["powershell.exe", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", str(build_script)],
        cwd=project,
        check=True,
    )

    handler = partial(SimpleHTTPRequestHandler, directory=str(project / "web"))
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
