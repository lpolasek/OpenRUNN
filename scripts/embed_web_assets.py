# pyright: reportMissingImports=false

import gzip
import re
from pathlib import Path
from typing import Any

from SCons.Script import Import


Import("env")
env: Any = globals()["env"]


PROJECT_DIR = Path(env.subst("$PROJECT_DIR"))
OUTPUT_DIR = Path(env.subst("$BUILD_DIR")) / "generated"
OUTPUT_FILE = OUTPUT_DIR / "WebAssets.h"


def minify_html(content):
    content = re.sub(r"<!--(?!\[if).*?-->", "", content, flags=re.DOTALL)
    content = re.sub(r">\s+<", "><", content)
    return content.strip()


def minify_css(content):
    content = re.sub(r"/\*.*?\*/", "", content, flags=re.DOTALL)
    content = re.sub(r"\s+", " ", content)
    content = re.sub(r"\s*([{}:;,])\s*", r"\1", content)
    return content.replace(";}", "}").strip()


def minify_javascript(content):
    content = re.sub(r"^\s*//.*$", "", content, flags=re.MULTILINE)
    content = re.sub(r"\s+", " ", content)
    return content.strip()


def format_byte_array(data):
    rows = []
    for offset in range(0, len(data), 16):
        chunk = data[offset:offset + 16]
        rows.append("    " + ", ".join(f"0x{value:02x}" for value in chunk))
    return ",\n".join(rows)


assets = [
    ("INDEX_HTML_GZ", PROJECT_DIR / "assets/web/index.html", minify_html),
    ("STYLES_CSS_GZ", PROJECT_DIR / "assets/web/styles.css", minify_css),
    ("APP_JS_GZ", PROJECT_DIR / "assets/web/app.js", minify_javascript),
]

sections = []
for symbol, path, minifier in assets:
    source = path.read_text(encoding="utf-8")
    compressed = gzip.compress(minifier(source).encode("utf-8"), compresslevel=9, mtime=0)
    sections.append(
        f"static const uint8_t {symbol}[] PROGMEM = {{\n"
        f"{format_byte_array(compressed)}\n"
        f"}};\n"
        f"static const size_t {symbol}_LEN = sizeof({symbol});"
    )

header = (
    "#pragma once\n\n"
    "#include <Arduino.h>\n\n"
    "namespace WebAssets {\n\n"
    + "\n\n".join(sections)
    + "\n\n}  // namespace WebAssets\n"
)

OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
OUTPUT_FILE.write_text(header, encoding="ascii")
env.Append(CPPPATH=[str(OUTPUT_DIR)])