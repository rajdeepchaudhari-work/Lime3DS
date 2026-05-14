<h1 align="center">
  <br>
  <a href="https://github.com/rajdeepchaudhari-work/Lime3DS"><img src="https://raw.githubusercontent.com/Lime3DS/Lime3DS/1b1c4f29d4280c750702459fd9a6ada539a4e9a9/dist/lime.svg" alt="Lime3DS" width="200"></a>
  <br>
  <b>Lime3DS</b>
  <br>
  <sub>Enhanced Edition</sub>
  <br>
</h1>

<p align="center">
  <img src="https://img.shields.io/badge/based%20on-Lime3DS-brightgreen" alt="Based on Lime3DS">
  <img src="https://img.shields.io/badge/platform-Windows%20%7C%20macOS%20%7C%20Linux-blue" alt="Platform">
  <img src="https://img.shields.io/badge/feature-Web%20Controller-orange" alt="Web Controller">
  <img src="https://img.shields.io/badge/feature-Fast%20Forward-purple" alt="Fast Forward">
</p>

**Lime3DS Enhanced Edition** is a fork of [Lime3DS](https://github.com/Lime3DS/Lime3DS) — a continuation of the Citra Nintendo 3DS emulator — with additional quality-of-life features focused on a better play experience on desktop and mobile.

---

## ✨ New Features (Enhanced Edition)

### 🎮 Web-Based Virtual Controller (iPhone / Android)

Turn any phone or tablet on the same Wi-Fi network into a wireless gamepad — no app install required. Just open a URL in Safari or Chrome.

- **Full button support** — A, B, X, Y, D-pad, L/R shoulders, Start, Select
- **Circle pad analog** — smooth analog stick emulation via touch joystick
- **Live bottom screen** — the 3DS touch screen is streamed as a live video feed directly in the center of the controller page, at native 320×240 resolution
- **Touch input** — tap or drag on the live screen to send touch input to the game at the correct coordinates
- **Zero install** — pure HTML/JS served directly from the emulator over HTTP
- **PWA support** — add to Home Screen on iPhone for a fullscreen, no-chrome experience
- **Edit mode** — drag and resize any button group to fit your hands
- **Haptic feedback** — subtle vibration on button press (where supported)

**How to use:**
1. Go to **Settings → Controls** and enable **Web Controller**, set a port (default: 8080)
2. Start a game
3. On your phone, open `http://<your-mac-ip>:8080`
4. Play — the URL is also shown in the emulator status bar

You can also toggle the web controller mid-game via **Emulation → Web Controller** without stopping the game.

---

### ⏩ Configurable Fast-Forward Toggle

A hotkey-driven fast-forward with a configurable speed multiplier.

- **Hotkey** — bind any key to **Toggle Fast Forward** in Settings → Hotkeys
- **Multiplier-based** — set 2×, 3× … 10× in **Settings → General → Fast-Forward Multiplier**
  - Always relative to your current speed limit (e.g., 2× at 100% → 200%; 2× at 50% → 100%)
  - Works correctly even if your base frame limit is already at a high value
- **Vsync-aware** — automatically switches the Vulkan swapchain to mailbox present mode when fast-forwarding, so the game actually runs faster even with vsync on
- **Status bar indicator** — speed percentage updates in real time so you can see the effect

---

### 🕹️ In-Game Web Controller Toggle

Enable or disable the web controller while a game is running — no restart needed.

- **Emulation → Web Controller** (checkable menu item)
- Works even if the "Web Controller" setting is off in preferences — the menu item is an explicit override
- Checkmark shows current state at a glance
- Safely stops the screen stream and restores your original input profile when disabled

---

## 📦 Building from Source

```bash
git clone https://github.com/rajdeepchaudhari-work/Lime3DS
cd Lime3DS
git submodule update --init --recursive

mkdir build && cd build
cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release
ninja lime-qt
```

### Windows (MSYS2)
```bash
pacman -S mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja mingw-w64-x86_64-gcc
# then the same cmake/ninja steps above
```

### Windows (MSVC)
```bash
# In Visual Studio 2022 Developer Command Prompt:
cmake .. -G"Visual Studio 17 2022" -A x64
cmake --build . --config Release --target lime-qt
```

### macOS
```bash
cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64
ninja lime-qt
```

---

## System Requirements

### Desktop
```
OS:  Windows 10 (64-bit), macOS 13 Ventura+, or modern 64-bit Linux
CPU: x86-64 or Apple Silicon. Single-core Passmark > 1,800
GPU: OpenGL 4.3 or Vulkan 1.1
RAM: 2 GB minimum, 4 GB recommended
```

### Android
```
OS:  Android 9.0+
CPU: Snapdragon 835 or better
GPU: OpenGL ES 3.2 or Vulkan 1.1
RAM: 2 GB minimum, 4 GB recommended
```

---

## Upstream

This fork tracks [Lime3DS](https://github.com/Lime3DS/Lime3DS). All original features, compatibility, and platform support are preserved. New features are additive only.

For game compatibility reports, see the upstream [compatibility list](https://github.com/Lime3DS/compatibility-list).
