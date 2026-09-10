# GAM200

C++ renderer targeting OpenGL ES 3.0, built for the **web** (WebGL2 via Emscripten), with a
**desktop** (Windows/MSVC) build kept for local testing.

Both targets compile the same source from `Scene/`. Shaders live in `shaders/`.
Background and migration history: `docs/SDL2_MIGRATION_NOTES.md`.

---

## Prerequisites

| Tool | Needed by | Notes |
|---|---|---|
| Git | both | `build_web` clones GLM; CMake FetchContent clones SDL2 + glad |
| CMake >= 3.15 | desktop | must be on `PATH` |
| Visual Studio 2022 | desktop | generator `Visual Studio 17 2022`, x64 |
| Python 3 + `jinja2` | desktop | glad2 generates its loader via `python -m glad` |
| Emscripten SDK 6.0.9 | web | see setup below |

None of these are committed. The toolchain is per-machine; the source is what's shared.
`emsdk/`, `third_party/`, `web/` and `build_desktop/` are all generated and gitignored.

### Python + Jinja2 (desktop only)

glad2 ships no generated sources: `glad_add_library()` runs `python -m glad` as a build step,
and glad imports Jinja2 to render its C templates. A missing Jinja2 surfaces as `MSB8066` on
`glad_gles2.vcxproj`, which names MSBuild rather than the actual cause.

CMake's `find_package(Python)` frequently picks a *different* interpreter than the one on
`PATH`. `script_build_and_run.bat` reads the one CMake recorded and checks it for you -- install
into that one:

```
python -m pip install jinja2
```

To force a specific interpreter:

```
set GAM200_PYTHON=C:\Path\To\python.exe
script_build_and_run.bat CLEAN
```

---

## Emscripten SDK setup (web build)

Pinned version: **6.0.9**. Pin it explicitly rather than using `latest`, so everyone on the
team compiles with the same toolchain.

From the project root:

```
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
emsdk install 6.0.9
emsdk activate 6.0.9
cd ..
```

`build_web.bat` checks for `em++.bat` on `PATH` and falls back to `.\emsdk\emsdk_env.bat`, so
cloning into the project root as `emsdk/` is picked up automatically. `emsdk/` is gitignored --
it is ~1.9 GB and platform-specific.

**Install it on the machine that will build.** `emsdk/upstream` holds native binaries for one
platform only, and a macOS or Linux emsdk copied into a Windows tree fails misleadingly: it has
no `.exe` and no `.bat` wrappers, yet bare `WHERE em++` still succeeds, because `WHERE` matches
the extensionless Unix launcher before trying `PATHEXT`. Verify a real Windows install with:

```
dir emsdk\upstream\emscripten\em++.bat
```

**Reinstalling:** `emsdk install` skips the download when the versioned directory already
exists, so a stale `upstream/`, `node/` or `python/` survives silently. Move them aside first.

---

## Build: web (primary target)

Windows:

```
build_web.bat
```

macOS / Linux:

```
source /path/to/emsdk/emsdk_env.sh
./build_web.sh
```

Output: `web/index.html` plus `index.js`, `index.wasm`, `index.data`. On first run the script
clones GLM 1.0.1 into `third_party/glm` if it is absent.

It must be served over HTTP -- opening `web/index.html` as a `file://` path fails, because the
browser fetches `.wasm` and `.data` at startup:

```
cd web
python -m http.server 8080
```

Then open <http://localhost:8080/>.

For a debug build, swap `-O2` for `-O0 -g -gsource-map -sASSERTIONS=2` in the build script.

---

## Build: desktop (Windows, local testing)

From the project root:

```
script_build_and_run.bat
```

Configures, builds and runs in one pass. Flags combine, in any order:

| Flag | Effect |
|---|---|
| *(none)* | incremental configure + build + run |
| `CLEAN` | wipe `build_desktop/` first -- forces a full SDL2 re-clone and rebuild |
| `GET_VERSION` | compile with `GET_VERSION=1` to print OpenGL version info at startup |

Output: `build_desktop/Debug/GAM200.exe`. Press **ESC** to quit.

The first configure pulls SDL2 `release-2.32.10`, glad2 `v2.0.8` and GLM `1.0.1` via
FetchContent, so expect it to take a while. `CLEAN` discards `_deps/` along with everything
else, so prefer plain `script_build_and_run.bat` unless you genuinely need a clean configure.

Without the wrapper script:

```
cmake -S . -B build_desktop -G "Visual Studio 17 2022" -A x64
cmake --build build_desktop --config Debug
```

> macOS cannot run the desktop target -- no OpenGL ES driver exists there. Use the web build for
> Mac-side iteration.

---

## Repository layout

```
Scene/                      shared C++ source (main.cpp, Platform.h)
shaders/                    GLSL ES 3.00 shaders
cmake/                      ImportDependencies.cmake -- FetchContent for SDL2, glad, GLM
docs/                       migration notes and background
build_web.bat               web build (Windows)
build_web.sh                web build (macOS/Linux)
script_build_and_run.bat    desktop configure + build + run
```

Generated, not tracked: `emsdk/`, `third_party/`, `web/`, `build_desktop/`.
