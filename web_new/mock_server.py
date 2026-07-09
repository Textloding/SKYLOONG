#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""本地预览用的模拟设备服务器:模拟 ESP32 固件的全部 HTTP 接口。
用法: python mock_server.py [端口]  然后浏览器打开 http://127.0.0.1:8000/
"""
import json, os, sys, time, urllib.parse
from http.server import HTTPServer, BaseHTTPRequestHandler

ROOT = os.path.dirname(os.path.abspath(__file__))
FS_DIR = os.path.join(ROOT, "_mockfs")   # 模拟 LittleFS
os.makedirs(FS_DIR, exist_ok=True)

STATE = {
    "mode": "STA", "ssid": "MyHomeWiFi", "ip": "192.168.1.100",
    "theme": 0, "aps_enable": True, "weather_enable": True,
    "gif_enable": True, "jpg_enable": True,
    "time_roll": 5000, "jpg_mode": "roll", "jpg_file": "",
    "timezone": 8, "language": 0, "keytone": 1, "keytone_file": "",
    "volume": 6, "video_fit": "contain", "video_audio": False,
}
APP_SETTINGS = {
    "weather": "",
    "city": "北京",
    "weather_provider": "openmeteo",
    "weather_endpoint": "http://api.open-meteo.com",
    "weather_lat": "39.9042",
    "weather_lon": "116.4074",
}
TOTAL_BYTES = 9 * 1024 * 1024

MIME = {".html": "text/html", ".js": "application/javascript",
        ".css": "text/css", ".svg": "image/svg+xml", ".png": "image/png",
        ".jpg": "image/jpeg", ".ico": "image/x-icon", ".mp3": "audio/mpeg",
        ".wav": "audio/wav", ".mpeg": "video/mpeg", ".json": "application/json"}


def fs_used():
    return sum(os.path.getsize(os.path.join(FS_DIR, f)) for f in os.listdir(FS_DIR))


class Handler(BaseHTTPRequestHandler):
    def _send(self, code=200, body=b"OK", ctype="text/plain"):
        if isinstance(body, str):
            body = body.encode("utf-8")
        self.send_response(code)
        self.send_header("Content-Type", ctype + "; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def _args(self):
        q = urllib.parse.urlparse(self.path).query
        return {k: v[0] for k, v in urllib.parse.parse_qs(q).items()}

    def _body(self):
        n = int(self.headers.get("Content-Length", 0))
        return self.rfile.read(n) if n else b""

    def do_GET(self):
        path = urllib.parse.urlparse(self.path).path
        if path == "/info":
            return self._send(body=json.dumps(STATE), ctype="application/json")
        if path == "/scan_networks":
            time.sleep(1.5)
            nets = [{"ssid": "MyHomeWiFi", "rssi": -45}, {"ssid": "TP-LINK_5G", "rssi": -60},
                    {"ssid": "Xiaomi_AP", "rssi": -72}, {"ssid": "CU_gU3f", "rssi": -85}]
            return self._send(body=json.dumps({"networks": nets}), ctype="application/json")
        if path == "/list":
            data = [{"type": "file", "name": f, "size": str(os.path.getsize(os.path.join(FS_DIR, f)))}
                    for f in sorted(os.listdir(FS_DIR)) if not f.startswith(".")]
            return self._send(body=json.dumps({"size": fs_used(), "total": TOTAL_BYTES, "data": data}),
                              ctype="application/json")
        if path == "/config.json":
            public_settings = {k: v for k, v in APP_SETTINGS.items() if k != "weather"}
            public_settings["weather_configured"] = bool(APP_SETTINGS.get("weather"))
            return self._send(body=json.dumps(public_settings, ensure_ascii=False), ctype="application/json")
        # 静态文件:优先 web_new 源码,其次模拟文件系统(上传的文件)
        rel = "index.html" if path in ("/", "/wifi") else path.lstrip("/")
        for base in (ROOT, FS_DIR):
            fp = os.path.join(base, rel.replace("/", os.sep))
            if os.path.isfile(fp):
                with open(fp, "rb") as f:
                    return self._send(body=f.read(), ctype=MIME.get(os.path.splitext(fp)[1], "text/plain"))
        # ffmpeg.js / worker 从旧 web 目录借用,便于本地测视频转码
        old = os.path.join(ROOT, "..", "web", rel.replace("/", os.sep))
        if os.path.isfile(old):
            with open(old, "rb") as f:
                return self._send(body=f.read(), ctype=MIME.get(os.path.splitext(old)[1], "text/plain"))
        self._send(404, "FileNotFound")

    def do_POST(self):
        path = urllib.parse.urlparse(self.path).path
        ctype = self.headers.get("Content-Type", "")
        body = self._body()
        if path == "/edit":  # multipart 上传
            boundary = ctype.split("boundary=")[-1].encode()
            parts = body.split(b"--" + boundary)
            for part in parts:
                if b"filename=" not in part:
                    continue
                header, _, content = part.partition(b"\r\n\r\n")
                fname = header.split(b'filename="')[1].split(b'"')[0].decode("utf-8")
                content = content.rsplit(b"\r\n", 1)[0]
                if fs_used() + len(content) > TOTAL_BYTES:
                    return self._send(500, "NO SPACE")
                with open(os.path.join(FS_DIR, os.path.basename(fname)), "wb") as f:
                    f.write(content)
                time.sleep(min(2.0, len(content) / 400000))  # 模拟慢速写入
            return self._send(200, "")
        args = {**self._args(), **{k: v[0] for k, v in urllib.parse.parse_qs(body.decode("utf-8", "ignore")).items()}}
        if path == "/config_wifi":
            STATE.update(ssid=args.get("ssid", ""), mode="STA")
            return self._send()
        if path == "/config_theme":
            STATE["theme"] = int(args.get("theme", 0)); return self._send()
        if path == "/config_timezone":
            STATE["timezone"] = int(args.get("timezone", 8)); return self._send()
        if path == "/config_language":
            STATE["language"] = int(args.get("language", 0)); return self._send()
        if path == "/config_keytone":
            STATE["keytone"] = int(args.get("keytone", 0))
            STATE["keytone_file"] = args.get("keytone_file", ""); return self._send()
        if path == "/config_volume":
            STATE["volume"] = max(0, min(9, int(args.get("volume", 6)))); return self._send()
        for app in ("aps", "gif", "weather"):
            if path == f"/config_app_{app}":
                if "enable" in args:
                    STATE[f"{app}_enable"] = args.get("enable") == "true"
                if app == "gif":
                    if args.get("video_fit") in ("contain", "cover"):
                        STATE["video_fit"] = args.get("video_fit")
                    if "video_audio" in args:
                        STATE["video_audio"] = args.get("video_audio") == "true"
                return self._send()
        if path == "/config_app_jpg":
            STATE["jpg_enable"] = args.get("enable") == "true"
            STATE["jpg_mode"] = args.get("jpg_mode", "roll")
            STATE["jpg_file"] = args.get("jpg_file", "")
            STATE["time_roll"] = int(args.get("time_roll", 5000)); return self._send()
        if path == "/config.json":
            incoming = json.loads(body.decode("utf-8"))
            if not incoming.get("weather"):
                incoming.pop("weather", None)
            APP_SETTINGS.update(incoming); return self._send()
        if path == "/time":
            return self._send()
        if path == "/reboot":
            print("** 模拟重启 **"); return self._send()
        self._send(404, "unknown")

    def do_DELETE(self):
        if urllib.parse.urlparse(self.path).path == "/edit":
            name = list(self._args().values())
            if not name:
                return self._send(500, "BAD ARGS")
            fp = os.path.join(FS_DIR, os.path.basename(name[0]))
            if os.path.isfile(fp):
                os.remove(fp)
                return self._send(200, "")
            return self._send(404, "FileNotFound")
        self._send(404)

    def do_PUT(self):
        self._send(200, "")

    def log_message(self, fmt, *a):
        sys.stderr.write("%s %s\n" % (self.command, self.path))


if __name__ == "__main__":
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 8000
    print(f"Mock ESP32 server: http://127.0.0.1:{port}/  (文件存于 {FS_DIR})")
    HTTPServer(("127.0.0.1", port), Handler).serve_forever()
