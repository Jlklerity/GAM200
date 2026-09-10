# GAM200 -- GLFW to SDL2 Migration & OpenGL ES 3.0 Review

**Session date:** 2026-09-06
**Project:** `/Users/JL/GAM200`
**Goal:** one C++ renderer targeting OpenGL ES 3.0 across desktop (Windows), Android,
and web (WebGL2).

> Historical record of the SDL2 migration. Current status, invariants and the open-items
> list live in `CLAUDE.md` at the project root.

---

## 1. Starting state

`Scene/` had been reduced to `main.cpp` + `Platform.h` — an early "coloured quad" stage. The
build scripts still referenced an older assignment's file set (`Renderer.cpp`, `Fan.cpp`,
`Ground.cpp`, `SceneHUD.cpp`, `Scene3D.cpp`, `Transform.cpp`), so the web build was broken
before any of this work started.

| Target  | Before                        | After                                  |
|---------|-------------------------------|----------------------------------------|
| Desktop | GLFW + GLEW, MSVC / VS 2022   | SDL2 + glad2 (`gles2=3.0`), MSVC       |
| Web     | Emscripten + GLFW (broken)    | Emscripten + SDL2, WebGL2              |
| Android | hand-written JNI, unused      | not migrated yet — see §8              |

---

## 2. Why SDL2 (decision record)

Chose **SDL2** over SDL3. SDL3's `SDL_AppInit`/`SDL_AppIterate` callbacks would have solved
the Emscripten main-loop problem for free, but SDL2 matches the existing course material and
the stubbed `-sUSE_SDL=2` path already in `build_web.sh`.

Three concrete payoffs over GLFW for this project:

1. **Android needs no hand-written JNI.** SDL2 ships `org.libsdl.app.SDLActivity`, which
   creates the surface, sets up the ES context, and calls `SDL_main` on its own thread.
   GLFW has no Android backend at all — that is why `NativeTemplate.cpp` had to exist.
2. **`SDL_RWops` unifies asset loading.** `SDL_RWFromFile("shaders/x.glsl", "rb")` reads from
   disk on desktop, from **inside the APK assets** on Android (SDL routes relative paths
   through `AAssetManager`), and from MEMFS on web. One path instead of three.
3. **Cleaner ES context on Windows.** `SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "1")` loads a
   real ES driver (ANGLE) rather than a desktop-GL driver pretending, so ESSL bugs fail on
   the dev machine instead of surviving to Android.

**SDL2 does _not_ fix the Emscripten main loop.** `emscripten_set_main_loop` is still required.

---

## 3. GLFW → SDL2 API mapping

| GLFW | SDL2 |
|---|---|
| `glfwInit()` | `SDL_Init(SDL_INIT_VIDEO)` |
| `glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API)` | `SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES)` |
| `GLFW_CONTEXT_VERSION_MAJOR/MINOR` | `SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3)` / `..._MINOR_VERSION, 0` |
| `glfwCreateWindow(w, h, title, ...)` | `SDL_CreateWindow(title, x, y, w, h, flags)` |
| `glfwMakeContextCurrent(win)` | `SDL_GL_CreateContext(win)` — creates *and* makes current |
| `glfwSwapInterval(1)` | `SDL_GL_SetSwapInterval(1)` |
| `glfwSwapBuffers(win)` | `SDL_GL_SwapWindow(win)` |
| `glfwPollEvents()` | `while (SDL_PollEvent(&e)) { ... }` — drain the queue yourself |
| `glfwWindowShouldClose(win)` | own `bool g_running`, cleared on `SDL_QUIT` |
| `glfwGetKey(win, GLFW_KEY_W)` | `SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_W]` |
| `glfwGetFramebufferSize` | `SDL_GL_GetDrawableSize(win, &w, &h)` |
| `glfwGetTime()` | `SDL_GetPerformanceCounter() / SDL_GetPerformanceFrequency()` |
| `glfwTerminate()` | `SDL_GL_DeleteContext` → `SDL_DestroyWindow` → `SDL_Quit` |
| `glfwSetErrorCallback` | `SDL_GetError()` after any failing call |

### Five traps

1. **Inverted return convention.** `glfwInit()` returns non-zero on *success*; `SDL_Init()`
   returns **0 on success**. Translating `if(!glfwInit())` literally errors out on success.
   *SDL3 flipped this back* (returns `bool`), so online snippets could be either — check which
   version a snippet targets.
2. **`int main(int argc, char* argv[])` exactly.** SDL2 `#define`s `main` to `SDL_main`;
   `SDL2main` supplies the real `WinMain` on Windows and `SDLActivity` calls it on Android.
   A no-parameter `main()` will not link.
3. **`SDL_WINDOW_OPENGL` is mandatory** in the `SDL_CreateWindow` flags, or
   `SDL_GL_CreateContext` fails with an error that does not mention the flag.
4. **Attributes before `SDL_CreateWindow`.** Also: SDL2 defaults `SDL_GL_DEPTH_SIZE` to **16**
   where GLFW defaults to 24 — set it explicitly.
5. **`SDL_GetKeyboardState` is only refreshed by `SDL_PumpEvents`** (called by
   `SDL_PollEvent`). Poll *before* reading input, or you read a stale/never-pumped array.
   Use `SDL_SCANCODE_*` (physical position) not `SDLK_*` (layout-mapped) for WASD.

---

## 4. Review findings — original GLFW code

### Fixed during this session

- **Fragment shader had no precision qualifier.** In GLSL ES 3.00 the *fragment* language has
  no default precision for `float` (§4.5.4 gives defaults only for `int`, `sampler2D`,
  `samplerCube`; the vertex language defaults to `highp float`). Desktop GL accepts the
  omission; a conforming ES driver rejects it. Added `precision mediump float;`.
  → *Why the asymmetry? Because ES 2.0-class mobile hardware was allowed to omit high
  precision in fragment shaders entirely.*
- `if(!SDL_Init(...))` inverted check (trap 1 above).
- `Input()` ran before `glfwPollEvents()` / `SDL_PollEvent` — always one frame stale.
- Movement was `+= 0.01f` per frame, tying speed to frame rate. Now `0.6f * dt` NDC/second,
  which reproduces the old speed at exactly 60 Hz.
- `glGetUniformLocation` called **every frame** for both uniforms. Now cached at link time.
- `glDrawElements(..., 6, ...)` hardcoded → `gIndexCount`.
- `VBO` was a local, so the handle was lost and the buffer could never be deleted.
- `glDisableVertexAttribArray(0/1)` called *after* `glBindVertexArray(0)` — the enable bits
  are VAO state, already captured, so those calls acted on VAO 0, not ours. Removed.
- `CleanUp()` deleted no GL objects at all.
- No `SDL_QUIT` handling → the window close button did nothing.
- No framebuffer-size handling → stale viewport on resize, wrong size on HiDPI
  (`SDL_GL_GetDrawableSize` gives pixels; `SDL_GetWindowSize` gives points).
- `CreateGraphicsPipeline()` never checked whether the program linked.

### Still open (see Open items in `CLAUDE.md`)

- `LoadShaderAsString` returns `""` on a missing file, so the compiler gets empty source and
  the real failure (file not found) is never reported.
- `CompileShader`: `shaderObject` is **uninitialised** if `type` is neither vertex nor
  fragment (UB), and on compile failure it deletes the shader then returns the now-invalid
  handle, which the caller attaches.
- **Index winding is clockwise.** Vertices `{2,0,1}` and `{3,2,1}` both wind CW. Culling is
  off so it draws today; enable `GL_CULL_FACE` with the default `GL_CCW` front face and the
  quad vanishes.
- Two `float` uniforms where a `vec2` — or really a model/view/projection matrix — belongs.
  GLM is linked but unused.
- Everything is a bare file-scope global. Needs a `Renderer` type with
  `Init/Update/Render/Shutdown` before Android (which enters via `SDLActivity`, not `main`)
  or a second window makes the globals untenable.

---

## 5. Changes applied, file by file

Every edited file has a `.bak` beside it. `.bak` does not match CMake's `*.cpp` glob, so the
backups are not compiled.

### `Scene/Platform.h`
Header section rewritten. GLEW and the `USE_GLFW` split are gone.

| Branch | Includes | Loader |
|---|---|---|
| `PLATFORM_EMSCRIPTEN` | `<GLES3/gl3.h>`, `<SDL2/SDL.h>`, `<emscripten.h>` | none |
| `PLATFORM_WINDOWS` | `<glad/gles2.h>`, `<SDL.h>` | `PLATFORM_NEEDS_GL_LOADER` |
| `PLATFORM_ANDROID` | `<GLES3/gl3.h>`, `<SDL.h>` | none |

Platform *detection* order is unchanged and still matters: `__EMSCRIPTEN__` must be tested
before `_WIN32`, because emcc defines both on some toolchain setups.

### `Scene/main.cpp`
- Globals: added `g_window`, `g_context`, `VBO`, `gIndexCount`, `gLocOffsetX/Y`;
  forward-declared `CleanUp()`.
- `InitializeProgram()`: correct init check with `SDL_GetError()`, full attribute set
  (profile ES / 3 / 0 / doublebuffer / depth 24), `SDL_WINDOW_OPENGL | SHOWN | RESIZABLE |
  ALLOW_HIGHDPI`, staged failure paths that unwind exactly what was built, glad load behind
  `PLATFORM_NEEDS_GL_LOADER`, `SDL_GL_GetDrawableSize` for the initial size.
- New `PollEvents()` handling `SDL_QUIT` and `SDL_WINDOWEVENT_SIZE_CHANGED`.
- `Input(float dt)` — keyboard-state pointer fetched once, scancodes, dt-scaled movement.
- New `Frame()`; `MainLoop()` branches to `emscripten_set_main_loop(Frame, 0, 1)` on web.
- `VertexSpecification()`, `CreateGraphicsPipeline()`, `PreDraw()`, `Draw()`, `CleanUp()`,
  `main(argc, argv)` updated per §4.

Function order (declaration-before-use matters here):
`CleanUp` fwd decl → … → `InitializeProgram` → `PollEvents` → `Input` → `PreDraw` → `Draw`
→ `Frame` → `MainLoop` → `CleanUp` → `main`.

### `CMakeLists.txt` / `cmake/ImportDependencies.cmake`
GLFW + GLEW fetches replaced by SDL2 + glad2; `-DGLEW_STATIC` dropped.

```cmake
target_link_libraries(GAM200
    PRIVATE SDL2::SDL2-static
    PRIVATE SDL2::SDL2main
    PRIVATE glad_gles2
    PRIVATE glm::glm
)
```

### `build_web.sh` / `build_web.bat`
Both rewritten: single SDL2 build, source list reduced to `Scene/main.cpp`, correct preload
path, `em++` presence check. The compiler is **`em++`**, not `emcc` -- see section 6 for why,
and for the flag changes. The `.bat` also auto-activates `.\emsdk` when `em++` is not
already on PATH. `.sh` is LF, `.bat` is CRLF -- a `.gitattributes` should pin both.

---

## 6. Verified facts (do not trust from memory — these were checked)

Several first guesses in this session were **wrong**. Recorded here with how they were checked.

| Claim | First guess | Verified value | How |
|---|---|---|---|
| Newest SDL2 tag | `release-2.30.9` | **`release-2.32.10`** | `git ls-remote --tags libsdl-org/SDL` |
| Newest glad2 tag | `v2.0.6` | **`v2.0.8`** | `git ls-remote --tags Dav1dde/glad` |
| glad CMake subdir | `cmake` | confirmed | `cmake/CMakeLists.txt` exists in v2.0.8 |
| glad API spec name | `gles2=3.0` | confirmed | ran the generator; emits `include/glad/gles2.h` |
| glad loader symbol | `gladLoadGLES2` | confirmed | `GLAD_API_CALL int gladLoadGLES2` in generated header |
| glad needs Python | assumed | **confirmed required** | `find_package(Python COMPONENTS Interpreter REQUIRED)` in `glad.cmake:181` |
| SDL2 CMake options | `SDL_SHARED/STATIC/TEST` | confirmed | `CMakeLists.txt:505-507` @ release-2.32.10 |
| SDL2 targets | `SDL2::SDL2-static`, `SDL2::SDL2main` | confirmed | alias targets at lines 3396 / 3525 |

> **Never pin `master` on `libsdl-org/SDL` — that branch is SDL3 now.** A bare `master` would
> silently pull the wrong major version.

### Emscripten flags (checked against `emscripten-core/emscripten` @ `d10aa3a`, 2026-09-05)

| Flag | Status | Source |
|---|---|---|
| `-sUSE_WEBGL2=1` | **deprecated** — "Pass `-sMAX_WEBGL_VERSION=2`" | `src/settings.js:544` |
| `-sMIN/MAX_WEBGL_VERSION` | current; no automatic WebGL1 fallback exists | `src/settings.js:548-563` |
| `-sFULL_ES3=1` | **not needed here** — emulates GLES3 features WebGL2 lacks (mainly client-side vertex arrays) and turns on `FULL_ES2` | `src/settings.js:574` |
| `-sUSE_SDL=2` | current (`--use-port=sdl2` is the newer spelling; both valid) | `tools/ports/sdl2.py:107` |
| `-sWASM=1` | now the default; no longer passed | — |
| **`em++`, not `emcc`** | `emcc` compiles `.cpp` as C++ but does **not** link `libc++`/`libc++abi`, giving undefined `std::`/`__cxa_`/`operator new` symbols at link time | `src/settings.js:2082` (`DEFAULT_TO_CXX = false`), `tools/link.py:1764`, `emcc.py:290` |

### Why the shader path works on web — and why that is fragile

`main.cpp` opens `"../../shaders/vertexshader.glsl"`. Under Emscripten the working directory
is `/` (`src/lib/libfs.js:58`), and `PATH.normalize` passes `allowAboveRoot = !isAbsolute`
(`src/lib/libpath.js:43`). Once the path resolves to absolute, `..` components above the root
are simply **dropped**, so `/../../shaders/x` → `/shaders/x`, which is where
`--preload-file shaders` mounts them. (Verified in `tools/file_packager.py:444`: with no
`@`, srcpath and dstpath are the same, so a bare `shaders` lands at `/shaders`.)

It works, but it depends on above-root clamping in two different filesystems for two different
reasons. The real fix is `SDL_RWops` (§8).

---

## 7. Building

### Desktop (Windows / MSVC)
```
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Debug
```
- **Delete `build_desktop/` first.** It still holds a CMake cache from the GLFW/GLEW
  configuration; stale cache variables will fight the new one.
- First configure is slow: it clones SDL2 and glad, and glad's generator downloads the
  Khronos XML specs — so that configure needs network access and **Python on `PATH`**.

### Web (works on macOS) -- VERIFIED WORKING 2026-09-06
```
source /path/to/emsdk/emsdk_env.sh
./build_web.sh
cd web && python3 -m http.server 8080     # → http://localhost:8080/
```

The emsdk is installed at `./emsdk`, so in practice:
`source ./emsdk/emsdk_env.sh && ./build_web.sh`.

First run of this failed with undefined C++ symbols because the script called `emcc`.
Fixed by switching to `em++`; see the last row of the table in section 6.

### macOS native — not possible
Verified in SDL `release-2.32.10`, `src/video/cocoa/SDL_cocoaopengl.m:272`:

```c
if (_this->gl_config.profile_mask == SDL_GL_CONTEXT_PROFILE_ES) {
#ifdef SDL_VIDEO_OPENGL_EGL
    /* Switch to EGL based functions */ ...
#else
    SDL_SetError("SDL not configured with EGL support");
    return NULL;
#endif
}
```

Even inside the `#ifdef`, `Cocoa_GLES_LoadLibrary` calls `SDL_EGL_LoadLibrary`, which
`dlopen`s `libEGL`/`libGLESv2` — macOS ships neither. Apple's GL is desktop-only, capped at
4.1, deprecated since 2018. Additionally `#version 300 es` will not compile in a desktop GL
context, and `Platform.h` has no `__APPLE__` branch (it only limps along because
`CMakeLists.txt` unconditionally passes `-DPLATFORM_WINDOWS` — accidental, not correct, and it
would produce a clean compile followed by a runtime failure at context creation).

**Use the web target for Mac-side iteration.** WebGL2 *is* ES 3.0, so `#version 300 es`
compiles unchanged.

---

## 9. Things to watch on first run

- `SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24)` now actually requests a depth buffer, and the
  quad's indices are wound clockwise. Neither matters while depth testing and culling are off,
  but you meet both the first time you enable them — and **the symptom is identical in each
  case (nothing renders)**, which makes them easy to confuse.
- Movement is now frame-rate independent. On a 144 Hz display the quad moves at the same
  real-world speed instead of 2.4× faster.
- `dt` gotcha: `SDL_GetPerformanceCounter()` and `SDL_GetPerformanceFrequency()` are both
  `Uint64`. Dividing them in integer arithmetic yields `0` essentially always — cast to
  `double` *before* dividing.


---

## 10. Change log

- **2026-09-06** Reviewed the original GLFW / ES 3.0 code; fixed the fragment shader precision
  qualifier. Migrated windowing from GLFW to SDL2 across `Platform.h`, `main.cpp`,
  `CMakeLists.txt`, `cmake/ImportDependencies.cmake`. Replaced GLEW with glad2. Rewrote
  `build_web.sh` for SDL2 + WebGL2. Web build confirmed working after correcting `emcc`
  to `em++`. Renamed these notes from `SDL2_MIGRATION_NOTES.md` to `claude.md`.
- **2026-09-06 (later)** Split these notes out of `CLAUDE.md`. Brought `build_web.bat` up to
  parity with `build_web.sh` (SDL2, `em++`, current WebGL2 flags, correct sources and preload
  path, auto-activates `.\emsdk`). Simplified `--preload-file shaders@/shaders` to
  `--preload-file shaders` in both scripts.
