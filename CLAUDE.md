# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

Open and build via Xcode:
```
open MilTech-C-1.xcodeproj
```

Or compile directly (no external dependencies required):
```bash
clang++ -std=c++17 -o MilTech-C-1 MilTech-C-1/main.cpp
```

Run the executable — it reads from `input.txt` and writes to `output.txt` in the working directory:
```bash
./MilTech-C-1
```

No test framework or lint tooling is configured.

## Architecture

Single-file C++17 command-line tool (`main.cpp`, ~174 lines). All logic lives in `main()`.

**Data model**: Five hardcoded ammunition types (`VOG-17`, `M67`, `RKG-3`, `GLIDING-VOG`, `GLIDING-RKG`) stored as parallel const arrays: `BOMB_NAMES_LIST`, `BOMB_M` (mass kg), `BOMB_D` (diameter m), `BOMB_L` (lift coefficient — `0.0` ballistic, `1.0` gliding).

**Pipeline**:
1. Read and parse a single space-delimited line from `input.txt`: 3D position (`xd yd zd`), direction, attack speed, acceleration path, ammo name, and simulation tuning params (`arrayTimeStep`, `simTimeStep`, `hitRadius`, `angularSpeed`, `turnThreshold`).
2. Validate ammo name against the hardcoded list.
3. Solve a cubic equation for time-of-flight, then compute trajectory height `h` using the matched ammo's physical properties and `GRAVITY_FORCE = 9.81`.
4. Output the firing point coordinates (`x y`) to `output.txt`.

**Status**: The physics and output sections (lines ~86–164) are currently commented out — this is intentional in-progress work ("Lesson 2"). The input parsing and ammo validation are complete.
