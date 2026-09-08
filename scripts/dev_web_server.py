#!/usr/bin/env python3

import argparse
import json
import threading
import time
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import urlparse


PROJECT_DIR = Path(__file__).resolve().parent.parent
WEB_ROOT = PROJECT_DIR / "assets" / "web"


class SimulatedTelemetry:
    def __init__(self, speed_kph):
        self._lock = threading.Lock()
        self._simulated_speed_kph = speed_kph
        self._distance_meters = 0.0
        self._last_update = time.monotonic()

    def _update(self):
        now = time.monotonic()
        elapsed_seconds = now - self._last_update
        self._last_update = now
        self._distance_meters += self._simulated_speed_kph / 3.6 * elapsed_seconds

    def status(self):
        with self._lock:
            self._update()
            speed_kph = self._simulated_speed_kph
            pace_seconds_per_km = 3600.0 / speed_kph if speed_kph > 0 else 0.0
            return {
                "distanceMeters": round(self._distance_meters, 2),
                "speedKph": round(speed_kph, 2),
                "paceSecondsPerKm": round(pace_seconds_per_km, 1),
            }


class DashboardHandler(SimpleHTTPRequestHandler):
    session = None

    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=str(WEB_ROOT), **kwargs)

    def do_GET(self):
        if urlparse(self.path).path == "/api/status":
            self._send_json(self.session.status())
            return
        super().do_GET()

    def end_headers(self):
        if urlparse(self.path).path.startswith("/api/"):
            self.send_header("Cache-Control", "no-store")
        super().end_headers()

    def _send_json(self, payload):
        body = json.dumps(payload, separators=(",", ":")).encode("utf-8")
        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)


def main():
    parser = argparse.ArgumentParser(description="Run the OpenRUNN dashboard development server.")
    parser.add_argument("--host", default="0.0.0.0", help="Address to bind (default: 0.0.0.0).")
    parser.add_argument("--port", type=int, default=8765, help="Port to bind (default: 8765).")
    parser.add_argument("--speed-kph", type=float, default=10.0, help="Simulated running speed.")
    args = parser.parse_args()

    DashboardHandler.session = SimulatedTelemetry(max(0.0, args.speed_kph))
    server = ThreadingHTTPServer((args.host, args.port), DashboardHandler)
    print(f"OpenRUNN development server: http://{args.host}:{args.port}")
    print(f"Simulated speed: {DashboardHandler.session._simulated_speed_kph:.1f} km/h")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nStopping server.")
    finally:
        server.server_close()


if __name__ == "__main__":
    main()