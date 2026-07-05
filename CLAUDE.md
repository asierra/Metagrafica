# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this project is

MetaGráfica is a 2D descriptive vector graphics language. The `mg` binary compiles `.mg` source files into EPS, SVG or PDF (chosen by output extension). Versioning follows the project's publication history (see `include/structures.h` header): V1 (grammar of two-letter commands, 1999–2024) is frozen on the `v1-legacy` branch; `main` is the **V3** development line. `include/version.h` is fixed (`MG_VERSION "3.0.0"`) and must stay consistent with the `structures.h` header.

The forward-looking design lives in `especificacion_mg.md`: §3.1 (isometric space), §16 (nested windows), §22 (engine continuity plan), §22.6 (work order). Read §22 before large engine changes.

## Build and test

```bash
make                  # build bin/mg and man page
make clean
./bin/mg examples/primitives.mg          # → primitives.eps
./bin/mg examples/fig2-3.mg out.svg      # backend by extension (.eps/.svg/.pdf)

bash test/run.sh check    # golden-file regression (EPS+SVG). MUST be ok=16 fail=0
bash test/run.sh capture  # re-bless goldens (only after verifying changes are intended)
```

**Always run `test/run.sh check` before and after touching the engine.** Golden files (`test/golden/`) are not in git; PDF is verified by eye (timestamps vary).

Toolchain: `clang++`/`g++` (C++14, `-fno-rtti -fno-exceptions`), `flex` (regenerates `src/lexmg.cpp` from `src/mgpp.l`), `pandoc` (man page). Do not edit `src/lexmg.cpp` or `include/version.h` content by hand. libharu is vendored in `third_party/` for PDF.

## Layout

Headers in `include/`, sources in `src/`, binary in `bin/`, examples (+`.mg` corpus) in `examples/`, regression harness in `test/`.

## Architecture

Pipeline: `.mg` → **MGLexer** (`src/mgpp.l`) → **Parser** (`src/Parser.cpp`) → in-memory tree (`MetaGrafica`) → **Display** backend → EPS/SVG/PDF.

- **`GraphicsItem`** (`include/primitives.h`) — abstract base of every drawable; hierarchy is non-copyable (use-count bookkeeping in `StructureUser`). `Path` = `std::vector<point>`.
- **`Structure` / `MetaGrafica`** (`include/structures.h`) — named reusable groups; `MetaGrafica` is the document (dimensions `$D` in cm, world window `WW`, font size). `StructureLine/Arc/Path/Rectangle` place structs geometrically.
- **`Display`** (`include/Display.h`) — abstract backend + device-independent state machine. Implementations: `EPSDisplay`, `SVGDisplay`, `PDFDisplay`.
- **`Matrix`** (`include/matrix.h`) — 3×3 homogeneous transforms; post-multiplies (`translate(); scale();` ⇒ `T·S`). `transform_radii()` maps radii by column norms (circles survive rotation).

### Coordinate system (implemented 2026-07-05, spec §3.1)

The engine is **isometric by construction**: `Display::pushWorldMatrix()` builds the single world→device seed matrix in each backend's `start()` — uniform *meet* scale `s = min(W/wdx, H/wdy)` plus centered margin. `stretch_mode` (internal bool, no CLI flag) reproduces V1 per-axis stretching for the future translator. Key invariants:

- The parser does **not** normalize document-level coordinates; the real `WW` goes to `MetaGrafica::setWindow` → `Display::setWindow`. Struct bodies **do** keep their local window normalized to the unit box (V1 placement contract).
- cm→pt scale is exact (§3.2); the `+0.5` rounds only the printed `%%BoundingBox`.
- No per-primitive aspect compensations exist anymore (no `getRatio()`, no `w=h` forcing in `arc`). Do not reintroduce them.
- V1 front-end translates V1 placement semantics ("struct box = min(canvas)") via `docwmin = min(wdx,wdy)` applied to all placement kinds (`RPST`/`DPST`, `LNST`/`ARCST`, identifier). This factor dies with the V1 front-end.
- Anisotropic user transforms (`SCST x≠y`, shear) legitimately produce ellipses; the parser sets `flags.using_ellipse` so EPS emits its procedure.

### Adding a new primitive

1. Token in `src/mgpp.l` (`keyword_map` in `init_tables()`); 2. `GI_*` enum + subclass in `include/primitives.h`; 3. handle in `Parser::parsePrimitive()` or a `parse*` helper; 4. `draw(Display&)` calling `Display` virtuals; 5. implement those in the three backends.

## Roadmap state (2026-07-05)

Done: golden harness; orthodox-C++ hardening; isometric engine (§3.1/§22.4). Next per §22.6: add approved non-square examples (fig2-6, fig4-1, fig6-10) to `EXAMPLES` in `test/run.sh`; V3 grammar (§1–§18) — **start with fig4-10**, the canonical case study (needs drastic redesign with per-panel structs + `fit`, compare against `examples/fig4-10-V1.eps`); then `mg1to2.py` translator (disproportionate V1 files need `stretch=true` or renumbering, §21). Known wart: `EPSDisplay::start` doesn't check `fopen` failure (segfaults on unwritable path).

## Code style

[Orthodox C++](https://gist.github.com/bkaradzic/2e39896bc7d8c34e042b): no RTTI, no exceptions; `std::unique_ptr` for ownership, raw pointers non-owning. `-Wall -Wpedantic -Wsuggest-override`, warnings-clean. In headers: fully qualified `std::` (no `using` at namespace scope), `override` on all overrides, include guards `MG_*_H` (never `__*`), in-class member initializers. Project language for comments/messages is Spanish; keep new features in the compiler itself (no external preprocessors).
