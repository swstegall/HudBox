# HudBox

HudBox is a lightweight desktop “HUD” (heads‑up display) window for Linux desktops built with GTK 4 and WebKitGTK. It renders a frameless, optionally transparent window that loads any web address you provide — perfect for dashboards, status boards, stream overlays, or pinning a small web app on your desktop.

Core capabilities:
- Frameless GTK 4 window (no system decorations)
- Loads arbitrary web content via WebKitGTK
- Optional translucency/opacity control (0.0–1.0)
- Optional fully transparent window + transparent web page background
- Click‑and‑drag the content to move the window (when unlocked)
- JSON configuration (single window or multiple windows)
- Autogenerates a default config at `~/.hudbox.json` if one is missing


## Requirements

Build-time dependencies:
- A C23‑capable compiler (GCC/Clang)
- CMake ≥ 3.31.6
- pkg-config
- GTK 4 development files (pkg-config name: `gtk4`)
- WebKitGTK 6.0 development files (pkg-config name: `webkitgtk-6.0`)
- JSON‑GLib development files (pkg-config name: `json-glib-1.0`)

Example packages by distro (names may vary; check your distribution):
- Debian/Ubuntu: `sudo apt install build-essential cmake pkg-config libgtk-4-dev libwebkitgtk-6.0-dev libjson-glib-dev`
- Fedora: `sudo dnf install gcc cmake pkgconf-pkg-config gtk4-devel webkitgtk6.0-devel json-glib-devel`
- Arch: `sudo pacman -S base-devel cmake pkgconf gtk4 webkitgtk-6.0 json-glib`


## Build

This is a standard CMake project. The executable is named `hudbox`.

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

The resulting binary will be at `build/hudbox` (or your generator’s default output folder).


## Install

HudBox now provides a standard CMake install target. You can install it either for your user account (recommended) or system‑wide.

User (local) install to ~/.local:

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$HOME/.local"
cmake --build build -j
cmake --install build
```

Ensure that `~/.local/bin` is on your `PATH` (most distros add this by default).

System‑wide install (requires sudo):

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
sudo cmake --install build
```

Notes:
- The installed executable is named `hudbox` and is placed in `$prefix/bin` (e.g., `$HOME/.local/bin/hudbox` or `/usr/local/bin/hudbox`).

## Usage

Run with an optional path to a JSON configuration file:

```
hudbox [path/to/config.json]
```

- If a path is provided, HudBox loads only that configuration and does not create any files.
- If no path is provided, HudBox looks for `~/.hudbox.json`. If it does not exist, a default file is created automatically, then loaded.
- If no valid configuration can be loaded, a single default window is shown.

Window behavior:
- When `locked` is `false`, click‑and‑drag anywhere on the web content to move the window.
- When `locked` is `true`, dragging is disabled.
- `opacity` controls GTK window opacity (0.0–1.0). Values are clamped in code.
- When `transparent` is `true`, the GTK window uses a transparent background and an injected user style sheet forces the page background to transparent. This is useful for overlays. Some websites with aggressive styling might still render opaque elements — that’s content‑specific.


## Configuration

HudBox reads either a single JSON object (one window) or a JSON array of objects (multiple windows). All fields are optional and have defaults.

Schema (all fields optional):
- `title` (string) — Window title. Default: `"HudBox"`.
- `address` (string) — URL to load. Also accepts legacy key `uri`. Default: `"https://github.com/swstegall/HudBox"`.
- `width` (integer) — Initial width in pixels. Default: `800`.
- `height` (integer) — Initial height in pixels. Default: `600`.
- `locked` (boolean) — If `true`, disables drag‑to‑move. Default: `false`.
- `opacity` (number) — 0.0–1.0 window opacity. Default: `0.9` (clamped to range at runtime).
- `transparent` (boolean) — Make window + page background transparent. Default: `false`.

Single window example:

```
{
  "title": "My HUD",
  "address": "https://example.com/dashboard",
  "width": 1280,
  "height": 300,
  "locked": false,
  "opacity": 0.85,
  "transparent": true
}
```

Multiple windows example:

```
[
  {
    "title": "Status Bar",
    "address": "https://status.example.com/mini",
    "width": 1200,
    "height": 80,
    "locked": true,
    "opacity": 0.95,
    "transparent": true
  },
  {
    "title": "Notes",
    "address": "https://app.example.com/notes",
    "width": 520,
    "height": 700,
    "locked": false,
    "opacity": 1.0,
    "transparent": false
  }
]
```

Default config that HudBox writes to `~/.hudbox.json` on first run (when missing):

```
[
	{
	  "title": "HudBox",
	  "address": "https://github.com/swstegall/HudBox",
	  "width": 800,
	  "height": 600,
	  "locked": false,
	  "opacity": 0.9,
	  "transparent": false
	}
]
```

Notes:
- The code accepts `address` or legacy `uri` for the URL field; `address` takes precedence when both are present.


## Tips & Troubleshooting

- If CMake cannot find GTK/WebKitGTK/JSON‑GLib, ensure you have the “-dev”/“-devel” packages and `pkg-config` installed. You can run `pkg-config --modversion gtk4 webkitgtk-6.0 json-glib-1.0` to verify.
- Wayland/X11 transparency: HudBox applies a global CSS provider for GTK and sets the WebKit view background to fully transparent when `transparent` is `true`. Actual appearance may depend on your compositor/theme.
- Content transparency: Even with a transparent window, page content can still have opaque elements. HudBox injects a user style sheet that forces `html, body` to transparent, but site‑specific elements may need their own CSS (which you could provide via the site or a custom page).
- Dragging doesn’t work? Set `locked` to `false`. Drag anywhere on the page with the primary mouse button.
- Opacity out of range? Values are clamped to `[0.0, 1.0]` at runtime.


## Development

- Project type: CMake (see `CMakeLists.txt`)
- Language standard: C23 (`set(CMAKE_C_STANDARD 23)`)
- Main sources: `main.c`


## License

No license file is included in this repository at the moment.
