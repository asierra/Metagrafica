# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this project is

MetaGráfica is a 2D descriptive vector graphics language. The `mg` binary compiles `.mg` source files into EPS, SVG or PDF (chosen by output extension). Versioning follows the project's publication history (see `include/structures.h` header): V1 (grammar of two-letter commands, 1999–2024) is frozen on the `v1-legacy` branch; `main` is the **V3** development line. `include/version.h` is fixed (`MG_VERSION "3.0.0"`) and must stay consistent with the `structures.h` header.

The forward-looking design lives in `especificacion_mg.md`: §3.1 (isometric space), §16 (nested windows), §22 (engine continuity plan), §22.6 (work order). Read §22 before large engine changes.

## Build and test

```bash
make                  # build bin/mg (the V3 compiler) and man page
make clean
./bin/mg examples/primitives.mg          # → primitives.eps
./bin/mg examples/fig2-3.mg out.svg      # backend by extension (.eps/.svg/.pdf)

bash test/run.sh check    # golden-file regression (EPS+SVG) — EN PAUSA (ver abajo)
bash test/run.sh capture  # re-bless goldens (only after verifying changes are intended)
```

**Harness golden EN PAUSA (2026-07-09).** El corpus V3 se movió de `examples/v3/` a
`examples/` y se está corrigiendo a mano (las salidas ya no están atadas al oráculo
V1), así que `test/run.sh` tiene un guard que sale (`exit 0`) sin correr. Mientras
tanto, verifica los cambios del motor **renderizando el corpus a EPS y pasándolo por
Ghostscript** (`gs -dNOPAUSE -dBATCH -sDEVICE=nullpage f.eps`), y en SVG/PDF por
inspección. Cuando los ejemplos sean estables: borrar el guard de `test/run.sh`,
apuntar `EXDIR` a `examples/` y re-bendecir con `capture`. Golden files
(`test/golden/`) no están en git; PDF se verifica por vista.

Toolchain: `clang++`/`g++` (C++14, `-fno-rtti -fno-exceptions`), `flex` (regenerates `src/lexmg.cpp` from `src/mgpp.l`), `pandoc` (man page). Do not edit `src/lexmg.cpp` or `include/version.h` content by hand. libharu is vendored in `third_party/` for PDF.

## Layout

Headers in `include/`, sources in `src/`, binary in `bin/`, regression harness in `test/`.

The example corpus is split for the V1→V3 transition (see `examples/v1/README.md`):
- **`examples/v1/`** — frozen V1-syntax corpus (two-letter commands). Serves as translator fixtures + provenance. `examples/v1/reference/*.svg` are the committed **migration oracle**: renders produced while the compiler still parses V1 (SVG chosen for size; SVG/EPS/PDF match). These SVGs are force-included past the `*.svg` gitignore.
- **`examples/`** (raíz) — corpus V3 **compilable** con `bin/mg` (los 15 `.mg`: arrow, curvas3, fig2-1/2-3/2-6, fig4-1/4-10, fig6-1/6-10, fill_styles, line_patterns, primitives, rpstest, sines, texto). Se movió aquí desde `examples/v3/` el 2026-07-09; Alejandro los está refinando a mano y sus salidas **ya no están atadas** al oráculo V1 (dejan de ser traducción 1:1 y pasan a ejercitar/mostrar la gramática V3).

**Cutover hecho (§22.6):** `bin/mg` en `main` **es el compilador V3** (se arma de `src/parserv3.cpp` + `src/lexv3.cpp` + motor + PDF/haru). `test/run.sh` compilaba `examples/v3/` con la salida del propio renderer V3 como red golden (regresión, no el oráculo V1); hoy el corpus vive en `examples/` y el harness está **en pausa** durante el refinamiento manual (ver "Build and test"). `src/main.cpp`, `src/Parser.cpp`, `src/lexmg.cpp` (front-end V1) quedan en el árbol pero **fuera del build**; V1 sigue congelado en `v1-legacy`. `make v3test` es un alias (`cp bin/mg bin/v3test`).

## Architecture

Pipeline (V3, post-cutover): `.mg` → **lexer** (`src/lexer.l` → `src/lexv3.cpp`) → **Parser V3** (`src/parserv3.cpp`: descenso recursivo → AST de `Stmt` → `exec` emite `GraphicsItem`s) → in-memory tree (`MetaGrafica`) → **Display** backend → EPS/SVG/PDF. *(El pipeline V1 —`src/mgpp.l` → `src/Parser.cpp`— sigue en el árbol como referencia/insumo del traductor, pero fuera del build; ver `plan_mg1to2.md`.)*

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

1. `GI_*` enum + subclass in `include/primitives.h`; 2. despacho por nombre en `parserv3.cpp` (`isPrim()` + `PrimStmt`, o un `Stmt`/`parse*` propio para sintaxis con bloque, p. ej. `axis`/`compound`); 3. `draw(Display&)` calling `Display` virtuals; 4. implement those in the three backends. *(V3 despacha las primitivas por su nombre-cadena en `parseStatement`, no por token del lexer; solo hace falta tocar `src/lexer.l` para símbolos/operadores nuevos, no para comandos.)*

## Roadmap state (2026-07-09)

El parser V3 (`src/parserv3.cpp`) compila los 15 ejemplos de `examples/` a EPS/SVG/PDF.
Grande hecho: expresiones+control de flujo (§5-6), structs+invocación+place/fit/repeat
(§8/§10/§17), generadores §13 (numbers/ticks/axis/grid), primitivas geométricas+bezier+
sine, texto con markup, estado color/fill/line_width/dash/font/align/valign + atributos
por-primitiva (§7.5) con alcance correcto (gsave/grestore en EPS/PDF), transform §11.1.

Cerrado en la sesión del 2026-07-09 (ver commits recientes): `polyline(closed=true)`
(§4.1); `hatch` y `outlinefill` como **sentencias de estado** (§4.11; la trama usa el
color de relleno, el contorno es explícito); concatenación de cadenas con `+` y `str(x
[,decimals])` (§5.2); `_`/`^` (sub/superíndice) **solo en modo matemático `$…$`** (§14);
y varios arreglos EPS (font_size scoping, fuga de estado por-primitiva, cshow/ellipse,
rect doble-stroke).

Cerrado más tarde el 2026-07-09 (segunda tanda, commits `dd84925`..`8aa7fa5`):
- **Build** (`dd84925`): libharu se liga como biblioteca estática (`obj/haru/libharu.a`,
  regla `ar`) en vez de listar ~45 `.o` en el enlace; un cambio menor al motor recompila
  solo su `.o` y re-liga sin re-archivar haru.
- **Patrones de relleno §4.11 (Fases 1–3 de `plan_patterns.md`)**: `hatch` pasó de índice
  entero a `FillPattern` en `dspstate` (nuevo `GraphicsItem` `HatchAttr`; `Display::setHatch`),
  con `hatch` **sobrecargado** (número = ángulo LIBRE de una familia; cadena = estilo
  `"hatch"`/`"hatchback"`/`"crosshatch"`) y `hatch_gap` libre. EPS dejó los 4 procs PS fijos
  por un barrido genérico por ángulo (igual método que PDF); SVG teja el crosshatch con las
  dos diagonales de un tile `gap·√2`. Se corrigió el ángulo SVG reflejado por el flip global
  `scale(1,-1)` (`rotate(90-a)`, no `a-90`). Verificado EPS/SVG/PDF idénticos para ángulos
  libres y nombrados.
- **Texto (tres arreglos, §4.8/§7.5/§14.3)**: (1) EPS sincroniza el caché de fuente
  `dev_face/dev_size` con `gsave/grestore` (antes un `grestore` revertía la fuente del
  dispositivo pero el caché quedaba obsoleto → el guard omitía el `setfont` y el texto salía
  con la fuente vieja; etiquetas encimadas en fill_styles). (2) `text()` ahora **honra sus
  atributos por-primitiva** (antes se parseaban y se ignoraban): `font_size` (alias `size`),
  `color`, `align`, `valign` como estado acotado con push/pop; (3) `font=` (cara) como cara
  inicial de `parse_text`, horneada en el `Text`. Verificado en los tres backends.

**Fase 4 de patrones CERRADA** (`plan_patterns.md`): la spec §4.11 y `fill_styles.mg`
están al modelo nuevo, y la limpieza de código muerto está hecha —`patternFor`/`AT_FPATRN`
/`max_fillpattern` borrados del compilador V3 (`Display.h`, `EPSDisplay`, `PDFDisplay`,
`primitives.cpp`); solo sobreviven en `src/mgpp.l` y `src/Parser.cpp` (front-end V1,
fuera del build) como referencia legible del traductor `mg1to2.py`. También arreglado el
wart de `fopen`: `EPSDisplay::start` ahora reporta y `exit(1)` si la ruta no es escribible
(antes segfault). Bug de contorno de `polygon` con `outlinefill` resuelto: `Polyline::draw`
cierra la costura (`closepath`) para todo `GI_POLYGON`, no solo con `closed=true` (el
`closepath fill` del relleno va dentro de gsave/grestore en EPS/PDF y no persiste).

Siguiente concreto — curvas de **fig4-1** (paneles + escala, reescribir sin `rotate 90`;
originalmente usó trucos por limitaciones de versiones previas — Alejandro está decidiendo
el enfoque). Otros pendientes: spline/smooth §9 (motor `splines.cpp` listo); `font=`
por-primitiva en `numbers`/etiquetas de `axis` si se pide; traductor `mg1to2.py`
(`plan_mg1to2.md`).

## Code style

[Orthodox C++](https://gist.github.com/bkaradzic/2e39896bc7d8c34e042b): no RTTI, no exceptions; `std::unique_ptr` for ownership, raw pointers non-owning. `-Wall -Wpedantic -Wsuggest-override`, warnings-clean. In headers: fully qualified `std::` (no `using` at namespace scope), `override` on all overrides, include guards `MG_*_H` (never `__*`), in-class member initializers. Project language for comments/messages is Spanish; keep new features in the compiler itself (no external preprocessors).
