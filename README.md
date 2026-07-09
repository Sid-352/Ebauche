# Ebauche

An entire hard drive, rendered as a galaxy. Folders orbit each other and files glow based on size. The whole thing animates in real time.

Name is a french word roughly (from what I know of French) translating to "Rough draft" or "In progress work". I believe that Ebauche can be turned into something much more impressive with some gutting, so this app remains a transitional period.

---

## Requirements

- CMake 3.20+ and Git
- Any C++20 compiler; MSVC 2022, GCC 12+, or Clang 14+
- On Windows: Visual Studio 2022 with the C++ workload (or Build Tools)
- On Linux: `libgl1-mesa-dev`, `libx11-dev`, and the usual X11/audio dev packages that Raylib needs

---

## Building

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

First run will fetch Raylib, ImGui, and rlImGui automatically. Binary lands at `build/Release/Ebauche`.

Run from the project root; shaders live in `resources/` and need to be findable from CWD.

---

## Getting Started

On launch you get a dialog with two options:

**Scan a directory:** Pick a folder, hit Start. The scan will run in the background. When it finishes, it will save a `.bin` manifest and drop you into the galaxy.

**Load a manifest:** Loads the `.bin` directly. Should load near instantly.

For memory safety and performance reasons, scanning a full drive with several hundred thousand files takes a second or two.

---

## Controls

| Input | Action |
|---|---|
| `W / S / A / D` | Fly forward / backward / left / right |
| `Space / Shift` | Move up / down |
| `Ctrl` + move | 10× speed |
| `Alt` + move | 0.1× speed |
| `Scroll wheel` | Change base speed |
| `Right mouse` + drag | Look around |
| `Middle mouse` + drag | Pan |
| `Left click` | Select node (click again to deselect) |

Clicking on a node shows its name, size, and full path in the right hand HUD. **Open in OS** opens it in the default File Manager.

---

## Overlay

The diagnostic panel is open by default. The useful sliders:

- **Simulation Speed:** how fast everything orbits
- **Directory / File Size:** scale the visual size of nodes
- **Bloom Intensity:** glow strength
- **Render Distance:** self explainatory
- **Search:** type anything, hit Next to cycle through matches

`Z` hides all of this for a clean view.
