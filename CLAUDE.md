# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Lime3DS is a continuation of the Citra Nintendo 3DS emulator project. It's a C++ application supporting multiple platforms (Windows, macOS, Linux, Android) with Qt-based GUI and SDL2 frontends.

**Key Stats:**
- Language: C++20
- Build System: CMake 3.15+
- Main Targets: `lime-qt` (Qt GUI), `lime` (SDL2), `lime-room` (dedicated server), `tests`
- Architecture: ARM interpreter/JIT, HLE services emulation, Vulkan/OpenGL/software renderers

## Build Configuration

### Dependencies
The project uses **vendored dependencies** in `/externals/` (Boost, Qt6, SDL2, Vulkan, dynarmic, cryptopp, httplib, nlohmann/json, catch2, fmt, etc.) to ensure consistent builds across platforms. Qt is downloaded at configure time unless `USE_SYSTEM_QT=ON`.

**Submodules** must be initialized before building:
```bash
git submodule update --init --recursive
```

### CMake Build Commands

**Basic build (all platforms):**
```bash
mkdir build && cd build
cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release
ninja
```

**With common development options:**
```bash
cmake .. -GNinja \
  -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_QT_TRANSLATION=ON \
  -DENABLE_COMPATIBILITY_LIST_DOWNLOAD=ON \
  -DUSE_DISCORD_PRESENCE=ON
ninja
```

**macOS universal build:**
```bash
cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64
ninja bundle
# Then use .ci/macos-universal.sh to combine x86_64 and arm64 builds
```

**Run tests after build:**
```bash
ctest -VV -C Release
```

### Key CMake Options

| Option | Default | Purpose |
|--------|---------|---------|
| `ENABLE_QT` | ON | Build Qt frontend (lime-qt) |
| `ENABLE_SDL2_FRONTEND` | ON (not Android) | Build SDL2 frontend (lime) |
| `ENABLE_TESTS` | ON | Build test executable |
| `ENABLE_QT_TRANSLATION` | OFF | Compile translation files |
| `ENABLE_QT_UPDATER` | ON | Built-in updater for Qt |
| `ENABLE_VULKAN` | ON | Vulkan renderer backend |
| `ENABLE_OPENGL` | ON (not macOS ARM) | OpenGL renderer backend |
| `ENABLE_SOFTWARE_RENDERER` | ON (not Android) | Software renderer fallback |
| `ENABLE_LTO` | ON (Release, non-MSVC) | Link-time optimization |
| `CITRA_USE_PRECOMPILED_HEADERS` | ON | Precompiled headers for faster builds |
| `CITRA_WARNINGS_AS_ERRORS` | ON | Treat warnings as errors |
| `USE_SYSTEM_SDL2` | OFF | Use system SDL2 instead of bundled |
| `USE_SYSTEM_BOOST` | OFF | Use system Boost instead of bundled |
| `USE_SYSTEM_QT` | OFF | Use system Qt6 instead of downloaded |
| `USE_DISCORD_PRESENCE` | OFF | Discord Rich Presence integration |
| `ENABLE_SCRIPTING` | ON | RPC server for external scripting |

## Code Architecture

### Core System Design

**System Singleton** (`src/core/core.h`):
- `Core::System::GetInstance()` — central instance managing emulation state
- Initializes and orchestrates: ARM CPU, memory, kernel, HLE services, GPU, audio
- Handles lifecycle: initialization, ROM loading, timing, shutdown
- `system.ApplySettings()` propagates settings changes to all subsystems at runtime (e.g. triggers HID `ReloadInputDevices()`)

**Key Subsystems:**

1. **ARM CPU Execution** (`src/core/arm/`):
   - Dynamic interpreter (`dyncom/`) for instruction decoding and execution
   - VFP (floating-point) support
   - Exclusive monitors for synchronization

2. **Memory System** (`src/core/memory/`, managed via `Kernel::KernelSystem`):
   - Virtual address translation
   - Multiple memory regions (FCRAM, VRAM, shared)
   - MemoryMode: determines available RAM (2MB default, 32MB New3DS)

3. **HLE Services** (`src/core/hle/`):
   - Service manager maps OS service handles to C++ implementations
   - Services like APT (applet), CAM (camera), CFG (config), FS (filesystem)
   - IPC (inter-process communication) via HLE_IPC

4. **Kernel** (`src/core/hle/kernel/`):
   - Process, thread, scheduler, mutex, semaphore, timer implementations
   - SVC (supervisor call) handlers bridging user to kernel operations

5. **GPU / Video Core** (`src/video_core/`):
   - Pica command-list interpreter (3DS GPU ISA)
   - Multiple renderers:
     - Software (`renderer_software/`) — always available
     - OpenGL (`renderer_opengl/`) — requires GL 4.3+
     - Vulkan (`renderer_vulkan/`) — requires Vulkan 1.1+
   - Shader generation: converts Pica shaders to GLSL/SPIR-V
   - Texture and framebuffer management

6. **Audio Core** (`src/audio_core/`):
   - DSP (digital signal processor) emulation
   - HLE and LLE (high/low-level) implementations
   - Cubeb and OpenAL backend support
   - ADPCM/AAC decoders

7. **Input System** (`src/input_common/`):
   - See the **Input Factory System** section below for details
   - Keyboard, SDL2 gamepad, UDP motion input (CemuhookUDP)
   - GameCube adapter support via libusb (`ENABLE_LIBUSB=ON`)
   - Touch input and motion emulation

8. **File System** (`src/core/file_sys/`):
   - Archive backends: NCCH (game cartridges), ExtSaveData, SDMC (SD card), etc.
   - ARTIC (host access) for network file serving

9. **Networking & Web Services** (`src/network/`, `src/web_service/`):
   - RPC server for scripting on localhost:6000 (when `ENABLE_SCRIPTING=ON`)
   - Telemetry (when `ENABLE_WEB_SERVICE=ON`)

### Input Factory System

The input system uses a **factory + device** pattern to decouple input sources from consumers. Understanding this is essential when adding new input backends (e.g. the web controller).

**Core interfaces** (`src/core/frontend/input.h`):
- `Input::InputDevice<StatusType>` — abstract base; subclasses implement `GetStatus()`
- `Input::Factory<DeviceType>` — abstract factory; subclasses implement `Create(ParamPackage)`
- Type aliases: `ButtonDevice = InputDevice<bool>`, `AnalogDevice = InputDevice<std::tuple<float, float>>`, `TouchDevice = InputDevice<std::tuple<float, float, bool>>`
- `Input::RegisterFactory<T>(name, factory)` / `Input::UnregisterFactory<T>(name)` — global registry

**Registration** (`src/input_common/main.cpp`):
- `InputCommon::Init()` registers all backends ("keyboard", "sdl", "cemuhookudp", "gcpad", etc.)
- `InputCommon::Shutdown()` unregisters them
- Both are called once at emulator start/stop, **not** per game

**HID polling** (`src/core/hle/service/hid/hid.cpp`):
- `Module::LoadInputDevices()` — reads `Settings::values.current_input_profile.buttons[]` param strings and creates device objects via `Input::CreateDevice<T>("engine:foo,param:bar")`
- `Module::UpdatePadCallback()` — called ~60Hz; calls `buttons[i]->GetStatus()` for each of the 14 HID buttons (`NativeButton::Values`: A through Gpio14) and `circle_pad->GetStatus()` for the analog
- `system.ApplySettings()` triggers `ReloadInputDevices()` which re-runs `LoadInputDevices()` at next frame

**Adding a new input backend** (reference pattern: `src/input_common/udp/` and `src/input_common/keyboard.cpp`):
1. Create a shared state struct protected by `std::mutex` (background thread writes, device reads)
2. Implement `ButtonDevice`/`AnalogDevice` subclasses whose `GetStatus()` reads that state
3. Implement `Factory<ButtonDevice>` / `Factory<AnalogDevice>` whose `Create()` instantiates those devices
4. Add `StartMyBackend()` / `StopMyBackend()` helpers to `src/input_common/main.h/cpp`
5. Register factories inside `StartMyBackend()`, unregister inside `StopMyBackend()`
6. Add source files to `src/input_common/CMakeLists.txt`

**In-progress: Web Controller** (`src/input_common/web_controller/`) — an HTTP server backend using the vendored `externals/httplib/` library that serves a touch-optimized virtual gamepad UI, allowing an iPhone on the same Wi-Fi network to send input via POST requests. Starts/stops per game in `GMainWindow::BootGame()` / `ShutdownGame()`.

### Frontend Architecture

**Qt Frontend** (`src/lime_qt/`):
- Main window (`main.h/cpp`) — game lifecycle (`BootGame()`, `ShutdownGame()`), signals `EmulationStarting` / `EmulationStopping`
- Game list management (`game_list.cpp`) — discovers and caches game metadata
- Configuration dialogs (`configuration/configure_*.cpp`) — per-game and global settings; `config.cpp` persists to INI via `ReadBasicSetting`/`WriteBasicSetting` macros
- Debugger UI (`debugger/`) — graphics inspection, registers, profiler
- Multiplayer/chat UI (`multiplayer/`)

**SDL2 Frontend** (`src/lime/`):
- Lightweight CLI-based interface for headless/server deployments

### Settings Architecture

**Settings System** (`src/common/settings.h`):
- Central `Settings::values` struct (all configurable values)
- Organized by category: graphics, audio, input, system, debug, ui
- Persisted to `lime3ds-config.ini` (INI format via `externals/inih`)
- Per-game override capability
- `Setting<T>` wrapper with `GetValue()` / `SetValue()` — `ReadBasicSetting` / `WriteBasicSetting` macros in `config.cpp` handle INI persistence

## Testing

Tests use **Catch2** framework and are built via `ENABLE_TESTS=ON` (default).

**Run all tests:**
```bash
ctest -VV -C Release
```

**Run specific test:**
```bash
./build/bin/Release/tests [test_name]
```

**Test categories** (`src/tests/`):
- Common utilities (bit fields, file I/O, parameter parsing)
- Core timing and memory system
- Audio codecs (ADPCM decoder, Biquad filters)
- Shader compilation
- IPC serialization

## Code Style & Formatting

**Style Guide** (`.clang-format` in repo root):
- 4-space indentation, 100-character line limit, no tabs
- Pointer alignment: left (`Type* var`)
- `AlwaysBreakTemplateDeclarations: true`
- `SortIncludes: true` — includes are auto-sorted by clang-format

**Format all source files:**
```bash
cmake --build build/ --target clang-format
```

**Important rules:**
- Warnings treated as errors by default (`CITRA_WARNINGS_AS_ERRORS=ON`)
- MSVC: stricter conformance with `/permissive-`
- Clang/GCC: stack protector, fortified source, `-Werror`
- **Pre-commit hook** automatically installed at CMake configure time to enforce formatting

## Platform-Specific Notes

**macOS:**
- Minimum deployment target: macOS 13
- Requires MoltenVK for Vulkan (auto-downloaded if `ENABLE_VULKAN=ON` and `USE_SYSTEM_MOLTENVK=OFF`)
- Universal binaries: arm64 + x86_64 combined via `lipo` in `.ci/macos-universal.sh`
- OpenGL disabled on macOS ARM by default (use Vulkan or Software renderer)

**Windows:**
- MSVC or MinGW (MSYS2) supported — no emulation difference, only compiler differences
- MSVC: smaller binary, requires Visual C++ redistributable
- MinGW: open-source, sometimes more compatible when MSVC runtime issues arise

**Linux:**
- clang-18 used in CI
- Qt6 DBus support required
- `ninja bundle` produces AppImage

**Android:**
- NDK-based build via Gradle/CMake in `src/android/`
- Software renderer and OpenGL disabled; Vulkan only

## Compilation Flags & Debugging

**Debug build:**
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug && ninja
```

**Speed up builds (ccache):**
```bash
cmake .. -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
```

**Disable architecture-specific optimizations:**
```bash
cmake .. -DENABLE_GENERIC=ON
```

## Build Artifacts

After `ninja`:
- `build/bin/Release/lime3ds-gui` — Qt frontend
- `build/bin/Release/lime` — SDL2 frontend
- `build/bin/Release/tests` — test executable

`ninja bundle` creates distributable bundles (macOS `.app`, Linux AppImage) in `build/bundle/`.
