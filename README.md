# Synthwave Drive: DeLorean DMC-12 & Akira Edition (Group SE-2423)

Real-time interactive 3D graphics application built in **C++17** and **OpenGL 3.3 Core Profile** for the **Computer Graphics Fundamentals** Final Project at Astana IT University.

- **Course:** Computer Graphics Fundamentals
- **Instructor:** Nazira Shaimerdenova
- **Group:** SE-2423
- **Topic:** #13 — **Synthwave Drive ★★** (Core + all 3 official Stretch Goals + extended 8-biome endless highway)
- **Repository:** https://github.com/aset-syrgabaev/synthwave-drive (Release `v1.0`)
- **Demo Video (1080p, 4:15):** https://youtu.be/_IpGSN-HNdA

---

## Team Members & Module Ownership

| Team Member | Email | Responsibilities & Owned Modules |
| :--- | :--- | :--- |
| **Tulebayeva Marzhan** | `241398@astanait.edu.kz` | Procedural sky & 80s striped sun shader (`Sky.h`, `sky.vert/frag`), HDR MSAA Framebuffer, dual-radius Bloom & CRT post-processing (`PostProcess.h`, `postprocess.vert/frag`), 8-world palettes (`Theme.h`), Kaneda Akira Bike & translucent vehicle effects (`AkiraBike.h`, `AkiraBike*.cpp`, `CarRenderAkira.cpp`, `CarRenderEffects.cpp`), speed/weather particles (`SpeedParticles.h`, `particles.vert/frag`) |
| **Aiymzhan Abilgazy** | `240977@astanait.edu.kz` | 5-mode jump-safe frame-rate-independent camera with mouse/keyboard orbit (`Camera.h`), Cook-Torrance PBR & Blinn-Phong vehicle/track shader (`car.vert/frag`), 3-lane track generator, solid LRT ceiling/train physics & Classic/Free Play modes (`Track.h`, `TrackSpawner.cpp`, `TrackPhysics.cpp`, `TrackRenderer.h`, `TrackDraw*.cpp`, `GameInput.cpp`, `GameEvents.cpp`), 8-biome procedural skylines (`VegasCity.h`, `VegasCityEras*.cpp`) |
| **Gaziza Bakir** | `241375@astanait.edu.kz` | Analytical `fwidth()` anti-aliased scrolling grid & 8 biome roads (`Terrain.h`, `terrain.vert/frag`), procedural mountain/dune terrain (`Mountain.h`, `mountain.vert/frag`), hardware-instanced palm trees (`PalmTree.h`, `instanced.vert/frag`), procedural city facade & billboard shader (`building.vert/frag`), and 2D bitmap HUD with persistent FPS & Help (`TextRenderer.h`, `Font5x7.h`, `text.vert/frag`, `GameHud.cpp`) |
| **Aset Syrgabaev** | `242052@astanait.edu.kz` | Core OpenGL 3.3 architecture & CMake build (`CMakeLists.txt`, `GLHeaders.h`, `Shader.h`, `Mesh.h`, `Primitives.h`, `GameApp.h`, `main.cpp`, `GameRender.cpp`), DeLorean DMC-12 procedural geometry, physics & parent-child hierarchy (`Car.h`, `CarModel.h`, `CarModel*.cpp`, `CarPhysics.cpp`, `CarRenderDeLorean.cpp`) |

---

## Mandatory Requirements (R1–R8) & Stretch Goals

### Mandatory Technical Requirements
- **R1. Modern Pipeline:** Pure OpenGL 3.3 Core Profile with VAO, VBO, EBO, Instanced VBOs, and HDR 4x MSAA FBO. 9 pairs of custom `.vert` / `.frag` GLSL 330 core shaders; zero fixed-function calls.
- **R2. Scene Setup (5+ Distinct Objects):** Articulated DeLorean DMC-12, Akira Bike, 8-biome procedurally instanced skylines & billboards, instanced palm trees, sculpted mountain ridges & sand dunes, scrolling highway with 200 m underground tunnel, subway trains & ramps, elevated LRT viaducts & monorail trains, high-altitude SkyCatwalk, laser gates, coins, 3D word letters, bonus orbs, drones, and speed particles.
- **R3. Transformation Pipeline & Hierarchy:** Custom GLM Model-View-Projection pipeline plus parent-child vehicle hierarchy (`computeRootMatrix()` → rotating/steering/90°-folding wheels, hinged gull-wing doors, Akira steering fork, light trail ribbons, hover thrusters).
- **R4. Lighting & Shading:** Per-fragment **Blinn-Phong** (`instanced.frag`, `terrain.frag`, `building.frag`, `mountain.frag`) and per-fragment **Cook-Torrance PBR** (`car.frag`: GGX normal distribution, Smith geometry shadowing, Schlick Fresnel) with 2+ light sources (`lightPos1` horizon light, `lightPos2` city fill light) plus dynamic toggleable headlights (`L`) projecting volumetric beams onto the road and obstacles.
- **R5. Texturing / Procedural Effects:** Analytical screen-space derivative (`fwidth()`) anti-aliased grid and procedural road/facade/dune/skin shaders (including carbon weave, digital matrix rain, frost crystals, and holographic stealth), HDR FBO render-to-texture, and procedural R8 bitmap font atlas (`TextRenderer.h`).
- **R6. Interaction:** Smooth mouse/keyboard `ORBIT 360` camera (`4`) plus 4 additional camera views (`1` Chase, `2` Action, `3` Hood, `5` Akira Cam, `V` cycle), interactive Settings panel (`G`), on-screen Controls card (`H`), mode switching (`CLASSIC` vs `FREE PLAY`), vehicle transformation (`K`), 8 worlds (`T`), 8 skins (`B`), headlights (`L`), doors (`O`), hover flight (`F`), and CRT toggle (`C`).
- **R7. Real-Time Performance:** 60+ FPS at 1280×720 with live FPS displayed both on-screen in the HUD (`FPS: ...`) and in the window title bar.
- **R8. Build System:** Clean CMake 3.16+ build using `FetchContent` for `GLFW 3.4`, `GLM 1.0.1`, and `GLEW 2.2.0` — zero manual library installation needed.

### Topic #13 Core & Stretch Goals
- **Core:** Infinite scrolling grid ground (`Terrain.h`, `terrain.frag`), sculpted side mountains (`Mountain.h`), striped 80s neon sun & gradient sky (`1985 LAS VEGAS`, key `T`), and steerable neon vehicle (`Car.h`).
- **Stretch Goal 1 — Hardware-Instanced Palm Trees:** `PalmTree.h` renders roadside palms via `glDrawArraysInstanced` and per-instance model matrices (`glVertexAttribDivisor`) in Las Vegas, Sahara Oasis, and Neon Jungle.
- **Stretch Goal 2 — Glow Around Neon Lines (HDR Bloom):** `PostProcess.h` + `postprocess.frag` renders into a `GL_RGBA16F` MSAA framebuffer and applies a dual-radius threshold bloom pass with ACES filmic tone mapping.
- **Stretch Goal 3 — CRT Scanline Post-Processing:** Toggleable (`C`) screen curvature, horizontal scanlines, RGB chromatic aberration, and vignette in `postprocess.frag`.
- **Extended Gameplay Features:** 2 game modes (`CLASSIC` where all perks—Nitro `SHIFT`, 2x/3x Air Jump, Shield `E`, Flight `F`, and `Q/X/Z/N/U` abilities—must be earned from road/LRT bonus coins, and `FREE PLAY` sandbox for testing every feature), 8 distinct biomes (`T`), 8 full-lighting vehicle skins (`B`), DeLorean ↔ Akira Bike transformation (`K`), and solid-ceiling LRT viaduct parkour.

---

## Controls

| Key / Input | Action |
| :--- | :--- |
| `A` / `D` or `Left` / `Right` | Switch between 3 highway lanes (with animated turn signals) |
| `SPACE` / `W` / `Up` | Jump (in `CLASSIC`: single jump by default; pick up green Jump Coin for 2x/3x Air Jump; always unlocked in `FREE PLAY`) |
| `S` / `Down` | Duck under laser gates / Akira Slide on bike / Dive in flight |
| `SHIFT` | Nitro Boost (in `CLASSIC`: uses fuel from orange Nitro Coins / Boost Pads; unlimited in `FREE PLAY`) |
| `J` | 360° Air Backflip trick while airborne |
| `K` | Transform vehicle: DeLorean DMC-12 ↔ Kaneda Akira Bike |
| `B` | Cycle 8 vehicle skins (changes body shader, headlights, road beam, taillights, trails, and shield) |
| `T` | Cycle 8 3D worlds (`0: Night City`, `1: 1985 Las Vegas`, `2: Neo-Tokyo Akira`, `3: Arctic Aurora`, `4: Orbital Station`, `5: Sahara Oasis`, `6: Neon Jungle`, `7: Mars Colony`) |
| `F` / `E` | Hover Flight (`4` Sky Rings in `CLASSIC`) / Energy Shield (requires road token in `CLASSIC`) |
| `Q` / `X` / `Z` / `N` / `U` | Slow-Mo / EMP Wave / Phase Ghost / Shock-Dash / Coin Storm Magnet (requires tech token in `CLASSIC`) |
| `L` / `O` / `C` | Toggle Headlights / Toggle DeLorean Gull-Wing Doors / Toggle CRT Scanlines |
| `1`–`5` or `V` + Mouse Drag | Switch 5 cameras (`4` = Orbit 360 with mouse drag & arrow keys) |
| `G` / `H` / `P` / `TAB` / `ESC` | Interactive Settings / Controls Help / Pause / Switch Classic ↔ Free Play / Menu |

---

## Build & Run Instructions (Fresh Clone)

### Prerequisites
- CMake 3.16 or newer
- C++17 compiler (Visual Studio 2019/2022/2026 on Windows, GCC 9+ on Linux, or Clang on macOS)
- Internet access on first configure (CMake automatically fetches `GLFW 3.4`, `GLM 1.0.1`, and `GLEW 2.2.0`)
- GPU driver supporting OpenGL 3.3 Core Profile

### Windows (Visual Studio / MSVC)
```powershell
git clone https://github.com/aset-syrgabaev/synthwave-drive.git
cd synthwave-drive
cmake -B build -S .
cmake --build build --config Release
cd build\Release
.\SynthwaveDrive.exe
```

### Linux / macOS
```bash
git clone https://github.com/aset-syrgabaev/synthwave-drive.git
cd synthwave-drive
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
cd build && ./SynthwaveDrive
```
*(CMake automatically copies the `shaders/` directory next to the compiled executable.)*

---

## Project Structure (Modular Architecture — Every File ≤ 500 Lines)

```text
synthwave-drive/
├── CMakeLists.txt
├── README.md
├── CREDITS.md
├── include/               # Clean module headers (all <= 220 lines)
│   ├── GLHeaders.h, Shader.h, Mesh.h, Primitives.h, Font5x7.h, Theme.h
│   ├── Camera.h, Sky.h, Terrain.h, Mountain.h, PalmTree.h, SpeedParticles.h
│   ├── PostProcess.h, TextRenderer.h, VegasCity.h
│   ├── CarModel.h, AkiraBike.h, Car.h, Track.h, TrackRenderer.h, GameApp.h
├── src/                   # Modular C++17 implementations (all <= 480 lines)
│   ├── main.cpp, GameInput.cpp, GameEvents.cpp, GameRender.cpp, GameHud.cpp
│   ├── CarModelBody.cpp, CarModelParts.cpp, AkiraBikeBody.cpp, AkiraBikeDetails.cpp
│   ├── CarPhysics.cpp, CarRenderDeLorean.cpp, CarRenderAkira.cpp, CarRenderEffects.cpp
│   ├── TrackSpawner.cpp, TrackPhysics.cpp, TrackDrawPickups.cpp, TrackDrawWorld.cpp
│   └── VegasCityErasA.cpp, VegasCityErasB.cpp
└── shaders/               # Separate GLSL 330 core vertex & fragment shaders
    ├── sky.vert/frag, terrain.vert/frag, mountain.vert/frag, building.vert/frag
    ├── car.vert/frag, instanced.vert/frag, particles.vert/frag
    └── postprocess.vert/frag, text.vert/frag
```

---

## Academic Integrity & AI Use Note

In full compliance with the Astana IT University *Computer Graphics Fundamentals* academic integrity policy:

- **AI Tool Used:** Google Gemini / Antigravity AI coding assistant.
- **Specific Tasks Assisted:**
  1. Generating initial `CMakeLists.txt` `FetchContent` boilerplate for GLFW/GLEW/GLM.
  2. Assisting with modular C++ refactoring (splitting monolithic headers into `.h`/`.cpp` modules under 500 lines) and adding explanatory comments.
  3. Debugging mathematical and physics edge cases (anti-aliased `fwidth()` grid transitions, Cook-Torrance GGX specular roughness mapping, wheel ground-contact clamping, and solid LRT ceiling collision bounds).
  4. Proofreading and structuring technical documentation (`README.md`, written report, and presentation slides) in academic English.
- **What the Team Designed, Changed, and Verified:**
  - The team selected Topic #13, defined the overall architecture and 8-biome art direction, authored and iteratively tuned vehicle proportions, skin materials, shader parameters, gameplay balance (`CLASSIC` coin-gated progression vs `FREE PLAY` sandbox), camera behaviors, and HUD layout.
  - Every shader, transformation matrix, lighting equation, collision check, and FPS benchmark was compiled, visually tested, and reviewed by all four team members (`Tulebayeva Marzhan`, `Aiymzhan Abilgazy`, `Gaziza Bakir`, `Aset Syrgabaev`). Every member is prepared to walk through and explain their assigned modules and underlying mathematics during the live defense.
