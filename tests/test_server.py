"""Run with: python -m unittest discover -s tests -p test_server.py"""

from functools import partial
from http.server import ThreadingHTTPServer
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import threading
import unittest
from unittest.mock import patch
from urllib.error import HTTPError
from urllib.request import Request, urlopen

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from build import build
from serve import PathfindingHandler, SAVED_MAP, prepare_project, validate_rows


class MapServerTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.folder = tempfile.TemporaryDirectory(prefix="pathfinding-test-")
        cls.addClassCleanup(cls.folder.cleanup)
        cls.project = Path(cls.folder.name)
        source = Path(__file__).resolve().parents[1]
        shutil.copy2(source / "main.cpp", cls.project / "main.cpp")
        for directory in ("Algorithms", "DataStructures", "Output"):
            shutil.copytree(source / directory, cls.project / directory)
        build(cls.project)
        handler = partial(PathfindingHandler, project=cls.project)
        cls.server = ThreadingHTTPServer(("127.0.0.1", 0), handler)
        cls.thread = threading.Thread(target=cls.server.serve_forever, daemon=True)
        cls.thread.start()
        cls.url = f"http://127.0.0.1:{cls.server.server_port}"

    @classmethod
    def tearDownClass(cls):
        cls.server.shutdown()
        cls.server.server_close()
        cls.thread.join()

    def post(self, payload, path="/api/run", content_type="application/json"):
        request = Request(self.url + path, data=json.dumps(payload).encode(),
                          headers={"Content-Type": content_type}, method="POST")
        try:
            response = urlopen(request, timeout=15)
        except HTTPError as error:
            response = error
        with response:
            return response.status, json.load(response)

    def setUp(self):
        (self.project / SAVED_MAP).unlink(missing_ok=True)

    def test_autosave_keeps_draft_without_running(self):
        target = self.project / "web" / "run-astar.json"
        before = target.read_bytes()
        rows = [".G.", "S#."]
        code, result = self.post({"rows": rows}, path="/api/map")
        self.assertEqual(code, 200, result)
        saved = self.project / SAVED_MAP
        self.assertEqual(saved.read_text().splitlines(), rows)
        self.assertEqual(target.read_bytes(), before)
        self.assertEqual(self.post({"rows": ["SSG"]}, path="/api/map")[0], 400)
        self.assertEqual(saved.read_text().splitlines(), rows)

    def test_restart_uses_saved_map_and_endpoints(self):
        source = (self.project / "main.cpp").read_bytes()
        rows = ["..S", "#G."]
        self.assertEqual(self.post({"rows": rows}, path="/api/map")[0], 200)
        # This is the same compile-and-run startup path used by a new serve.py process.
        prepare_project(self.project)
        result = json.loads((self.project / "web" / "run-breadthfirstsearch.json").read_text())
        self.assertEqual(result["rows"], rows)
        self.assertEqual(result["path"][0], 2)
        self.assertEqual(result["path"][-1], 4)
        self.assertEqual((self.project / "main.cpp").read_bytes(), source)

    def test_startup_fallback_and_invalid_saved_file(self):
        with patch("serve.build") as compile_project:
            prepare_project(self.project)
            compile_project.assert_called_once_with(self.project, map_file=None)
        saved = self.project / SAVED_MAP
        saved.write_text("SSG\n")
        with patch("serve.build") as compile_project, self.assertRaisesRegex(ValueError, "Invalid saved-map"):
            prepare_project(self.project)
        compile_project.assert_not_called()
        self.assertEqual(saved.read_text(), "SSG\n")

    def test_save_failure_preserves_previous_map(self):
        rows = ["S.G"]
        self.assertEqual(self.post({"rows": rows}, path="/api/map")[0], 200)
        with patch("serve.Path.replace", side_effect=OSError("disk full")):
            code, result = self.post({"rows": [".SG"]}, path="/api/map")
        self.assertEqual(code, 500, result)
        self.assertEqual((self.project / SAVED_MAP).read_text().splitlines(), rows)
        self.assertEqual(list(self.project.glob(".saved-map-*")), [])

    def test_static_assets_are_not_cached(self):
        with urlopen(self.url + "/algorithms.json") as response:
            self.assertEqual(response.headers["Cache-Control"], "no-store")

    def test_validation(self):
        for rows in (None, [], [1], [""], ["SG", "."], ["S?G"], ["S.."],
                     ["SSG"], ["S" + "." * 4096 + "G"]):
            with self.subTest(rows=repr(rows)[:40]), self.assertRaises(ValueError):
                validate_rows(rows)
        self.assertEqual(validate_rows(["S#G"]), ["S#G"])

    def test_edited_maps_and_unreachable_goal(self):
        executable = self.project / ".build" / ("pathfinding.exe" if sys.platform == "win32" else "pathfinding")
        compiled_at = executable.stat().st_mtime_ns
        for rows, expected_steps in ((["S.G"], 2), (["S#G", "..."], 4),
                                     (["S#G", ".##"], None), (["S.G"], 2)):
            with self.subTest(rows=rows):
                code, result = self.post({"rows": rows})
                self.assertEqual(code, 200, result)
                for identifier in ("breadthfirstsearch", "depthfirstsearch", "dijkstra", "greedy", "astar"):
                    data = result["runs"][identifier]
                    self.assertEqual(data["rows"], rows)
                    if expected_steps is None:
                        self.assertEqual(data["path"], [])
                    else:
                        self.assertEqual(len(data["path"]) - 1, expected_steps)
                        width = len(rows[0])
                        for index in data["path"]:
                            self.assertNotEqual(rows[index // width][index % width], "#")
                published = json.loads((self.project / "web" / "run-astar.json").read_text())
                self.assertEqual(published, result["runs"]["astar"])
        self.assertEqual(executable.stat().st_mtime_ns, compiled_at)

    def test_bad_requests_preserve_results(self):
        target = self.project / "web" / "run-astar.json"
        before = target.read_bytes()
        self.assertEqual(self.post({"rows": ["SSG"]})[0], 400)
        self.assertEqual(self.post([])[0], 400)
        self.assertEqual(self.post({}, path="/unknown")[0], 404)
        self.assertEqual(self.post({}, content_type="text/plain")[0], 415)
        self.assertEqual(target.read_bytes(), before)

    def test_timeout_and_busy_responses(self):
        target = self.project / "web" / "run-astar.json"
        before = target.read_bytes()
        with patch("serve.subprocess.run", side_effect=subprocess.TimeoutExpired("runner", 10)):
            self.assertEqual(self.post({"rows": ["SG"]})[0], 504)
        self.assertEqual(target.read_bytes(), before)
        PathfindingHandler.run_lock.acquire()
        try:
            self.assertEqual(self.post({"rows": ["SG"]})[0], 409)
        finally:
            PathfindingHandler.run_lock.release()

    def test_cpp_rejects_invalid_input(self):
        path = self.project / "invalid.txt"
        path.write_text("SSG\n", encoding="utf-8")
        executable = self.project / ".build" / ("pathfinding.exe" if sys.platform == "win32" else "pathfinding")
        result = subprocess.run([str(executable), str(path)], cwd=self.project,
                                capture_output=True, text=True, timeout=10)
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("exactly one S", result.stderr)


if __name__ == "__main__":
    unittest.main()
