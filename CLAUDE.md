# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this project is

MetaGráfica is a 2D descriptive vector graphics language. The `mg` binary compiles `.mg` source files into EPS, SVG or PDF (chosen by output extension). Versioning follows the project's publication history (see `include/structures.h` header): V1 (grammar of two-letter commands, 1999–2024) is frozen on the `v1-legacy` branch; `main` is the **V3** development line. `include/version.h` holds the version **by hand** (`MG_VERSION "3.0.0-beta"`, nothing is derived from git) and must stay consistent with the `structures.h` header. V3 is **beta**: the grammar can still change (it did on 2026-07-16: `title`→`label`, `labels`→`tick_labels`) and parts of the spec are unbuilt.

The forward-looking design lives in `especificacion_mg.md`: §3.1 (isometric space), §16 (nested windows), §22 (engine continuity plan), §22.6 (work order). Read §22 before large engine changes.

## Build and test

```bash
make                  # build bin/mg (the V3 compiler) and man page
make clean
./bin/mg examples/primitives.mg          # → primitives.eps
./bin/mg examples/fig2-5.mg out.svg      # backend by extension (.eps/.svg/.pdf)

bash test/run.sh check    # golden (EPS+SVG+PDF) + gs + paridad: ok=51 fail=0 error=0 psfail=0 c3fail=0
bash test/run.sh capture  # re-bless goldens (only after verifying changes are intended)
```

**Harness golden ACTIVO (reactivado 2026-07-11; ampliado 2026-07-14/15/17).** Corre el corpus
de `examples/` (17 `.mg` × EPS/SVG/**PDF** = 51 goldens) y compara contra la red golden
(salida del propio renderer V3, regresión — no el oráculo V1). Tras tocar el motor:
`make` y `bash test/run.sh check` (debe dar **ok=51 fail=0 error=0 psfail=0 c3fail=0**);
re-bendecir con `capture` solo tras verificar que los cambios son intencionales. Golden
files (`test/golden/`) **no están en git** (se regeneran con `capture`).

**Tres compuertas, cada una caza una clase distinta** (razonadas en `plan_plot.md`,
"Lecciones de ingeniería"):
- **Golden por bytes** (eps/svg/pdf) — caza *regresiones*. El PDF entró a la red el
  2026-07-14: la salida de libharu resultó byte-determinista (sin `CreationDate` ni
  `/ID`, independiente del path), así que se compara igual que EPS/SVG. Antes se
  "verificaba por vista" y en ese hueco vivió el bug de rótulos en blanco en PDF.
  **No** caza un bug *preexistente*: se bendice como correcto.
- **Ghostscript sobre el EPS** (`psfail`) — caza los bugs de **prólogo**, que el golden
  no puede ver: producen un EPS byte-estable que revienta al interpretarse
  (`/undefined in ellipse`; `/cshow` sin su prólogo si falta `using_textalign`). Se
  omite con aviso si no hay `gs` instalado. Corre también en `capture`, para no
  bendecir un EPS que no interpreta.
- **Paridad entre backends** (`c3fail`, "Capa 3", nueva 2026-07-15) — caza bugs
  **preexistentes** que el golden bendice porque un backend omite algo en silencio. Dos
  invariantes robustos (cero falsos positivos en el corpus, **sin herramientas externas**:
  el PDF de libharu no está comprimido → sus operadores son grepables): **(a)** nº de
  operaciones de texto `EPS(show) == SVG(<text>) == PDF(Tj)` (caza rótulos en blanco); **(b)**
  ningún path SVG de un solo segmento (`M..L..`) puede ir `fill=color stroke=none` = línea
  de área nula invisible (caza ejes sin trazo por fuga de fill, Lección 6). Corre también en
  `capture`. Es la única capa que caza un bug preexistente; las otras dos lo bendecen.

Las tres compuertas se verificaron reintroduciendo a propósito los bugs que deben cazar.

Toolchain: `clang++`/`g++` (C++14, `-fno-rtti -fno-exceptions`), `flex` (regenerates `src/lexmg.cpp` from `src/mgpp.l`), `pandoc` (man page). Do not edit `src/lexmg.cpp` by hand (flex generates it); `include/version.h` **is** edited by hand. libharu is vendored in `third_party/` for PDF.

## Layout

Headers in `include/`, sources in `src/`, binary in `bin/`, regression harness in `test/`. Design/working notes — the `plan_*.md` files plus `audit_text_parser.md` and `notas_at_anchor.md` — live in **`docs/plans/`** (moved out of root 2026-07-17; `docs/` also holds published source PDFs). References throughout the tree cite them **by bare filename** (grep the name, e.g. `plan_plot.md`), not by path. The forward-looking spec (`especificacion_mg.md`) and the pending-work board (`PENDIENTES.md`) stay in root.

**Política V1 (2026-07-15):** todo el trabajo actual es desarrollo de **V3**; **no se trackea material V1 nuevo**. Los `.mg` crudos de V1 y sus traducciones literales se quedan en el árbol **sin commitear** (p. ej. `examples/fig4-5v1.mg`, `fig4-5v3.mg`, y los de fig6-4). Lo ya trackeado en `examples/v1/` (31 archivos: corpus congelado + oráculo) **se queda como está** — la política aplica de aquí en adelante, no se borra nada.

The example corpus is split for the V1→V3 transition (see `examples/v1/README.md`):
- **`examples/v1/`** — frozen V1-syntax corpus (two-letter commands). Serves as translator fixtures + provenance. `examples/v1/reference/*.svg` are the committed **migration oracle**: renders produced while the compiler still parses V1 (SVG chosen for size; SVG/EPS/PDF match). These SVGs are force-included past the `*.svg` gitignore.
- **`examples/`** (raíz) — corpus V3 **compilable** con `bin/mg` (17 `.mg`: curvas3, fig1, fig2-1, fig2-5, fig4-1, fig4-5, fig6-4, fig_polybar, fill_styles, franck_condon, line_patterns, markers-demo, primitives, quickstart, rpstest, sines, texto). El corpus es una **lista explícita** en `test/run.sh`, no un glob: un `.mg` nuevo en la carpeta no entra solo (por eso conviven ahí archivos crudos sin commitear, p.ej. `fig4-5v1.mg` en sintaxis V1, que no compila con V3). Se movió aquí desde `examples/v3/` el 2026-07-09; sus salidas **ya no están atadas** al oráculo V1 (dejan de ser traducción 1:1 y pasan a ejercitar/mostrar la gramática V3). Es el corpus de la red golden (`test/run.sh`, reactivada 2026-07-11). **Poda 2026-07-17** (`arrow`, `fig2-3`, `fig4-10`, `fig6-1`, `fig6-10` eliminados: redundantes o `arrow.mg` que renderizaba vacío tras migrar sus flechas a marcadores built-in). `fig6-4` (renombrado desde `fig6-4v3-clean` el 2026-07-15) entró el 2026-07-14: es el único que ejercita eje **log** + `fit(stretch)` + math con superíndices + `extend` + ticks-in, y el único **sin `font` explícito** — por eso es el que caza el bug de cara ambiente en PDF.

**Cutover hecho (§22.6):** `bin/mg` en `main` **es el compilador V3** (se arma de `src/parserv3.cpp` + `src/lexv3.cpp` + motor + PDF/haru). `test/run.sh` compila el corpus de `examples/` con la salida del propio renderer V3 como red golden (regresión, no el oráculo V1); **reactivado 2026-07-11** (ver "Build and test"). `src/main.cpp` **sí es el entry point V3** y está en el build (Makefile: `bin/mg` = `main.cpp` + `lexv3.cpp` + `parserv3.cpp` + motor + haru); los que quedan en el árbol **fuera del build** son `src/Parser.cpp` y `src/lexmg.cpp` (front-end V1). V1 sigue congelado en `v1-legacy`. `make v3test` es un alias (`cp bin/mg bin/v3test`).

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

1. `GI_*` enum + subclass in `include/primitives.h`; 2. despacho por nombre en `parserv3.cpp` (`isPrim()` + `PrimStmt`, o un `Stmt`/`parse*` propio para sintaxis con bloque, p. ej. `axis`/`compound`/`plot`); 3. `draw(Display&)` calling `Display` virtuals; 4. implement those in the three backends. *(V3 despacha las primitivas por su nombre-cadena en `parseStatement`, no por token del lexer; solo hace falta tocar `src/lexer.l` para símbolos/operadores nuevos, no para comandos.)*

## Roadmap state (act. 2026-07-15)

El parser V3 (`src/parserv3.cpp`) compila los 16 ejemplos de `examples/` a EPS/SVG/PDF.
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

### Cerrado en la sesión del 2026-07-11 (commits `51368c3`..`fig2-1`)

- **Marcadores §B**: struct del usuario como marcador (`marker_end="Arrowr"`) por la ruta `Dot`
  (`Display::marker` toma `const MarkerShape&`; `markerShapeFromStruct` extrae la geometría).
- **Texto matemático (fuentes) en EPS/PDF**: (1) ASCII dentro de `$…$` salía EN BLANCO en EPS
  (la migración LM Math `0b040c3` solo cubrió PDF/SVG) → ASCII math ahora va a Times-Italic;
  (2) run MIXTO griego+ASCII (`$\Delta V = W_B - W_A$`): la Δ salía como ¢ (EPS/PDF) y el
  subíndice embarrado (SVG). Fix raíz en `text_parser.cpp`: la cara por defecto del math pasa a
  `FN_TIMES_ITALIC`; el griego entra por `\comando` como `FN_TEX_CMMI` en run AISLADO → no se
  fusiona. EPS/PDF además parten cualquier run cmmi en segmentos griego/ASCII.
- **§19 rotación de texto**: `rotate 90 text(...)` ahora gira los GLIFOS en los 3 backends
  (`text()` saca el ángulo de `mt`; SVG `<g rotate>`, EPS `gsave/rotate`, PDF `SetTextMatrix`).
- **Parser — aridad acotada en sentencias de estado**: `line_width 0.5  P()` en un renglón ya
  NO se traga `P()` (antes leía args "hasta newline"). Varias sentencias por línea, como
  translate/rotate. hatch=1-2 args, outlinefill=0-1, resto=1.
- **Ports fieles**: `fig4-10` (rediseño 3 paneles, etiquetas en coords de panel) y **`fig6-10`**
  (puntos de ocupación con `for`+`dot()`; huecos de letrero con `place(gap=)` = LNST gap V1;
  encuadre = ensanchar `world_window` al aspecto del display, reemplaza el `SCST` anisótropo V1).
  `fig2-1` cerrada. **Lección durable:** figura "apretada en un eje" = aspecto de `world_window`
  ≠ aspecto de `display_size` (el meet isométrico letterboxea).
- **Harness golden REACTIVADO** (ver "Build and test"): `bash test/run.sh check` → ok=32.

### Cerrado en la sesión del 2026-07-12 (pseudo-3D, ver `plan_pseudo3d.md`)

Soporte de ilustración pseudo-3D por **proyección oblicua**, **sin tocar el motor**
(hallazgo clave: `shear` YA existía en V3 como sentencia §11.1 y en `transform=`; el
draft del plan asumía falsamente que faltaba). Todo por composición de primitivas 2D:

- **`lib/pseudo3d.mg`** (biblioteca nueva): structs `plano(w,h,k,…)` y
  `prisma(w,h,d,a,f,…)` por **vértices COMPUTADOS** de los params (la oblicua se hornea
  en el polígono; nada de shear ambiental ni gimnasia de ventana → cada pieza se coloca
  con `at=`/`scale=`/`rotate=`). Trig en **radianes** (`cos(a*pi/180)`); `gray(g)` de
  default. Sin z-buffer: orden de pintado = orden de escritura.
- **`examples/simulate3d/`** (carpeta nueva, fuera de la red golden): `fig10-2v3.mg`
  (planos cizallados grises = `FGRAY -80`; `shear 0.4 0` en bloque acotado; base angosta
  vía `world_window 0 2.5 0 1`) y `fig2-7b-v3.mg` (panel b: láminas `hatch` inline,
  cristal `prisma`, pantalla `plano`, anillo `ellipse` directa — calibrado al original).
  `fig2-7v3.mg` = panel a (Bragg 2D, ya existía, NO usa pseudo-3D). Los `.png` son los
  oráculos de calibración (los `.eps/.pdf` grandes de V1 quedan fuera de git por
  `.gitignore`). Compilan EPS/SVG/PDF.
- **Footgun del lenguaje V3 documentado**: identificador desnudo seguido de `(` se parsea
  como llamada a función (`dx (h+dy)`→`dx(h+dy)`); en coords, parentizar: `(dx) (h+dy)`.

Pendientes pseudo-3D (opcionales, ver `plan_pseudo3d.md`): `hatch` como parámetro de
`plano` (hoy las láminas van inline); refactorizar `fig10-2v3` para usar `plano`; puntos
3D arbitrarios como generador §13 (Fase 3, si algún ejemplo lo pide); `box_axis` (Fase 4,
diferida). Isométrica verdadera (3 ejes) es caso posterior a la oblicua.

### Cerrado en la sesión del 2026-07-14 (dos bugs de EPS + `fig6-4` con axis, ver `plan_plot.md`)

Al portar `fig6-4` (Geiger-Nuttall: x lineal, **y logarítmico**) salieron dos bugs del
motor **solo-EPS** (SVG/PDF renderizaban bien), ambos corregidos (commit `7194975`, red
golden intacta `ok=32`):

- **`/undefined in ellipse`**: `fit(stretch=true)` sobre una struct con `circle(...)` da
  elipses al dibujar (`EPSDisplay::arc` compara normas de columna del CTM), pero el proc
  `/ellipse` solo se definía según la bandera de parse-time `flags.using_ellipse`, que no
  cubre ese caso (sí `SCST x≠y`/`shear`/`ellipse()` explícito). **Fix de raíz:**
  `EPSDisplay::arc` define `/ellipse` en su **primer uso** (miembro `ellipse_defined`);
  desacople draw-time/parse-time resuelto sin ampliar la heurística de parse (que habría
  metido el proc en casi todo documento con `fit(stretch)`, ubicuo en V3).
- **Rótulos de `axis` en blanco**: `axis`/`numbers`/`grid` emiten etiquetas con `FN_NOFACE`
  ("hereda ambiente") y `Text::draw` **omite `setFontFace`** para `FN_NOFACE` (`text.cpp:416`);
  si el documento nunca fijó `font`, EPS hace `show` sin fuente actual → invisible. **Fix:**
  guard en `EPSDisplay::text()` — si nunca se emitió fuente (`dev_size<0`), fija la cara
  vigente o Times-Roman. Cero churn (un `text()` normal hornea `FN_DEFAULT`, no lo toca).

- **`examples/fig6-4v3-clean.mg`** (commiteado; versión limpia con `axis`): ejes con `axis()`
  en vez de polilínea-en-L + `ticks()` a mano; puntos = `dot()` **físicos** (redondos, no
  `circle` que el stretch deforma → es la causa del bug #1). El eje y log se rotula a mano
  (`10^n` en `text()` math) porque `axis` aún no hace `scale="log"`. Los archivos crudos
  `examples/fig6-4.mg` (original V1), `fig6-4v3.mg` (traducción literal) y `fig6.4.png`
  (oráculo) quedaron **sin commitear** (working tree).

### Cerrado en la sesión del 2026-07-14 (`axis` maduro + red de pruebas; ver `plan_plot.md`)

**Fases 1 y 1.5 de `plan_plot.md` HECHAS**, todo en `AxisStmt::exec` (`parserv3.cpp`), más
los ajustes de fidelidad que salieron de calibrar `fig6-4` contra el original del libro:
- `scale="log"` (from/to en **valores**, no exponentes; marcas iterando el **exponente
  entero**, sin deriva flotante; rótulos `10^n` por markup math, n=0→`"1"`; `minor=true`),
  `strip_zero=true`; errores claros con from/to≤0 o step no entero.
- **Auto-alineación de etiquetas** por lado (center/top horizontal, right/middle vertical);
  override `label_align=`/`label_valign=`. `extend=` (alarga solo la línea, en unidades del
  eje). `label_gap` 8→**4 pt** (ahora mide al **borde** de la etiqueta, no al baseline) y
  `title_gap` nuevo (desacopla el título, antes `label_gap*3`). **Título centrado** a lo
  largo del eje (en el vertical, sobre la base rotada).
- **Bug de PDF arreglado:** `PDFDisplay::text()` hacía `if (!current_font) return;` sin
  fallback → el texto con cara ambiente (`FN_NOFACE`: rótulos de axis/numbers/grid) salía
  **en blanco** si el documento nunca fijaba `font`. Era el análogo del guard `dev_size<0`
  de EPS, nunca reflejado en PDF.

**Red de pruebas ampliada** (ver "Build and test"): PDF entró al golden (libharu resultó
byte-determinista) + compuerta `gs` (`psfail`); `fig6-4v3-clean` promovido al corpus.
**Ambas compuertas se verificaron reintroduciendo a propósito los bugs que deben cazar.**

### Cerrado en la sesión del 2026-07-15 (`plot` Fase 4: lineal+log+grid, ver `plan_plot.md`)

**Fase 4 HECHA: `plot { }`** (constructo tipo `compound` en `parserv3.cpp`, `PlotStmt`/
`parsePlot`). Bloque de contenido en **unidades de datos** + rangos `x=`/`y=` + caja física
`box=` (default = ventana vigente). `plot` **transforma las coordenadas de su contenido**:
- **Ruta lineal** → matriz envolvente datos→box (`FitStmt::fitMatrix` con stretch); hasta
  structs invocadas funcionan.
- **Ruta log** (`xscale`/`yscale="log"`, log **no es afín**) → el contenido se ejecuta a una
  lista temporal y se remapean **solo los puntos** de cada item (`getType()`+`getPath()`/
  `setPath()`; anclas de `text()` vía `getGraphicsStateType()`+`getPosition()`/`setPosition()`,
  solo `GS_PLUMEPOSITION`). Radios/anchos/font son miembros aparte o `Attribute`s → el mapper no
  los alcanza (invariante físico gratis).
- **Ejes** `xaxis`/`yaxis` interceptados por el parser, heredan `from/to/scale` de `plot` y se
  dibujan en coords exteriores (NO pasan por el mapper).

Huella en el motor: **dos accesores const** en `GraphicsState` (`getPosition()`/
`getGraphicsStateType()`), **cero elementos gráficos nuevos** — todo es trabajo de parser.

- **Estilo por-eje** (`AxisStmt`, aplica también al `axis` suelto): `line_width=`/`color=`
  (calcado de `GridStmt`) + `label_font=`/`label_size=` (hermanos de `title_font`/`title_size`),
  acotados con push/pop. **Resuelve** que los ejes de `plot` se dibujan FUERA del envoltorio de
  contenido → un `line_width`/`font` suelto en el bloque NO les llegaba.
- **`grid=`** (un solo arg: `true`=gris default / un color / `false`=sin): capa de **fondo**
  (z-order correcto, se pinta antes del contenido), reusa `axis(ticks="grid")` con los `step`
  de `xaxis`/`yaxis` → malla auto-alineada; log gratis. El `base` de la retícula del eje pasó a
  la **línea del eje** (no al origen de ventana).
- **Tres errores claros** (Paso 5): structs colocadas en un plot **log** (bandera de contexto
  `g_plotLogContext` con save/restore, consultada por invoke/repeat/fit-de-struct/place);
  `grid()`/`ticks()`/`axis()` **pelado** en contenido log (`GI_TICKS` lleva un VECTOR); rango
  log ≤0. `fit`-de-**path** sí compone (hornea matriz afín) → no se bloquea.
- **fig2-3** (lineal) y **fig6-4** (log) portadas a `plot { }`. fig6-4: datos en **valores
  reales** (conversión píxel→dato con script de un solo uso, inversión en `plan_plot.md`), cero
  coord digitalizada, cero `text()` de potencias de diez. Layout del libro (eje x bajo la 1ª
  década): `box` de fondo en y=0, rango y extendido a `1e-20`, `yaxis(start=1e-15)`. El título
  del eje y quedó `text()` manual (horizontal, estilo libro) porque `yaxis(title=)` lo rotaría.

**Bug cazado EN REVISIÓN (Lecciones 1 y 3 del plan):** la ruta log volcaba el contenido SIN
envoltorio `GS_PUSHSTATE`/`GS_POPSTATE` (la lineal lo tiene vía la matriz) → el `fill "black"`
de los puntos se **fugaba** a los ejes → sus líneas/marcas salían **rellenas sin trazo =
invisibles en PDF/SVG** (EPS lo toleraba). **Ni el golden por bytes** (bendecía la salida rota)
**ni la compuerta `gs`** (solo mira el EPS) lo cazaban → motivó la **Capa 3** (paridad entre
backends, ya HECHA — ver "Build and test"; su detector de "línea rellena" caza justo esto).
Fix: acotar el contenido log con push/pop, como la lineal. `make` limpio, `gs` OK, golden `ok=51`.

### Cerrado en la sesión del 2026-07-15 (`base=` de ejes + `fig4-5` a 3 paneles)

- **`base=` en `xaxis`/`yaxis` dentro de `plot`** (§13.7): el eje cruza en el valor dado, en
  unidades de datos del eje **perpendicular**, en vez de quedar clavado al borde de `box`
  (`by0`/`bx0` literales). Reusa el `mapAxis` de `PlotStmt` → log gratis; `grid=` NO se mueve
  (barre la caja completa). ~20 líneas, cero churn en el corpus.
- **`examples/fig4-5.mg`** (Fig. 3.1 de IMQ — potenciales y estructura del espectro), en el
  corpus golden (17→18 ejemplos, `ok=54`). Primer ejemplo con **varios `plot` en un documento**
  (rejilla de 3 paneles) y con **ejes interiores** (eje `V` centrado en a); eje `x` sobre `V=0`
  en c) — sin `base=` no era portable). Las 3 curvas del V1 estaban **digitalizadas** a 69
  puntos sobre la caja unitaria de una struct, y resultaron **analíticas exactas** (ajustan a
  ~1e-6): `V=x²`, `V=1/r`, `V=1/(2r²)−1/r` (efectivo, centrífugo+Coulomb). Se calculan de la
  fórmula → cero coord digitalizada, y los retornos/rótulos se **derivan de E**. La caja
  unitaria de V1 había corrido el origen de b) y c) a un `r≠0` arbitrario; el port lo devuelve.
  Bug del original destapado: el V1 traía `DOT 5 011 .3` (punto decimal perdido) → la marca de
  `x'₁` se iba a x=454 y **falta en el libro**.
- Ejes esquemáticos (sin marcas ni números) = `xaxis(ticks="none", labels=false)`.
- Los 6 rótulos de eje (`V(x)`, `x`) quedan `text()` manual en coords de ventana: `title=` los
  **centra** y el libro los pone al **extremo** → pendiente `title_at=` (Fase 3 de `plan_plot.md`),
  que también absorbería el título manual de fig6-4.
- **Lección 7** (`plan_plot.md`): en bloques de coordenadas `+`/`-` y `(` interactúan — o
  parentizas todas las coords o ninguna.
- **Bug de `grid=`: no propagaba `start`** (`emitGrid`, `parserv3.cpp`). La retícula ES la
  marca barrida por el campo, pero nacía en `from` en vez de en el `start=` del eje → con un
  `start=` fuera de la malla `from + k·step` quedaba en **contrafase con su propio eje**
  (`step=2 start=1` → marcas en 1,3,5… y líneas en 0,2,4…). En fig2-3 y fig6-4 alineaba **de
  casualidad** (su `start=` cae a un número entero de pasos de `from`), por eso nunca se vio.
  Fix: propagar `start`. Único churn: fig2-3 pierde una vertical redundante en el borde de la
  caja que el eje y ya tapaba (verificado en el diff del EPS: 4 líneas, x=36.85pt = 1.3cm =
  bx0) → cero cambio visual, golden re-bendecido.
- **`evalError` ahora es FATAL** (`ast.h`, `std::exit(1)`): antes imprimía y seguía, así que
  un documento roto llegaba a la salida (`-nan` en el EPS) con **código 0** — archivo
  inválido + "todo bien". Abortar es seguro: `buildFromSource` (parse+exec) corre entero
  antes de que `main` abra el archivo, así que no queda salida a medias. `warn` sigue siendo
  el no-fatal (color desconocido → negro). Cero churn (`ok=54`).
- **Conteo impar de coordenadas = error de compilación** (`checkCoordPairs`, `parserv3.cpp`):
  antes la coord sobrante se descartaba en silencio **sin evaluarla** (lazos `i+1 < size()`),
  que es lo que esconde el footgun de arriba —`polyline { 1/u (u*u-u) }` colapsa a UNA coord
  y la primitiva desaparece muda—. Se valida en parse-time (línea/columna, antes de evaluar)
  en los 4 bloques que esperan pares: primitivas (**cada subtrayecto de `;` por separado**),
  `text`, literal de path §9 y locus de `place`. Cero churn (`ok=54`).

### Cerrado en la sesión del 2026-07-16 (`polybar`: parser + Fig. 5 del primer artículo)

**Cableado del parser HECHO** (`plan_polybar.md` Fase 1, `src/parserv3.cpp`).
El motor YA tenía la clase `Polybar` y su `draw()` (expande cada punto a un `rect()`), y el
mapeador log de `plot` ya contemplaba `GI_POLYBAR` — pero **el parser nunca lo despachaba**
(`polybar` no estaba en `isPrim()`, nada llamaba a `setWidth`/`setHorizontal`). Dos cambios:
`isPrim()` + construcción en `PrimStmt::exec` (`width` posicional o nombrado y **obligatorio**;
`dir="horizontal"`). Cero cambios en los tres backends. Golden intacto (`ok=54`).

**Las barras contiguas de un polybar no se tapan**: `rect()` hace fill dentro de `gsave` y
stroke afuera **por barra**, así que la barra k+1 cubre el borde de la k con su relleno y acto
seguido lo retraza. Requiere `outlinefill`; sin él un `fill` sólido las fusiona en una silueta.

**`examples/fig_polybar.mg` HECHO** = Fig. 5 de `docs/first_article.pdf` (p. 13; espectros de
CO₂ a 261 y 522 cm, área rayada = 0.374 µm). **Tres pasadas**, **3 decimales**, en el corpus
golden (18→19 ejemplos, `ok=57`). Primero que ejercita `polybar` y `fill`-SIN-`outlinefill`.
**Cero cambios al motor.** Verificado: checksum releído del `.mg` = 0.3695 (−1.2%, el número
predicho) y pasadas 1/3 idénticas; trama a 8.25 px (EPS/PDF) / 8.33 (SVG) vs **8.07 del
original** → `hatch_gap=1.4` fiel; bandas en los mismos valores en los 3 backends (la pasada
blanca borra igual). Método, medidas del escaneo y datos completos en `plan_polybar.md`.
Lo esencial que dejó:
- **Aquí SÍ se vale digitalizar** (a diferencia de fig4-5): es de **1988**, V0 imprimía directo
  a láser (no había EPS) y el PDF es un **escaneo** (JPEG 300 dpi + capa OCR, cero vectores).
  No hay fórmula que recuperar: es un modelo de banda de Smith (1969), no un `1−exp(−τ)`.
  **Verificador de la leyenda**: `Σ(a_522−a_261)·0.5 == 0.374 µm`; lo reconstruido da **0.3695
  (−1.2%)**. Si ese número se mueve, algo se rompió.
- **Bug de V0 fósil**: las etiquetas del eje y del escaneo están ~13.5 px **arriba** de su valor
  (centrado vertical de texto corrido). El cero se toma de la **línea del eje**, no de ellas.
  La meseta 14.0–16.0 está saturada en **1.000 exacto** (el tope del eje y cae en el mismo píxel).
- **La figura son tres pasadas**, no una barra blanca encima de una rayada (eso habría borrado
  los costados verticales, que sí están): trama sin contorno → blanco opaco sin contorno →
  contorno solo. Salen con lo que ya hay: `hatch=` enciende el relleno y sin `color=` no hay
  `outlinefill` (rellena sin trazar); un `polybar` pelado traza sin rellenar.
- **La tinta bajo λ=12..20 del escaneo NO son marcas de eje**: son cantos de barra (en λ=22,
  sin barra, no hay tinta) → el eje x va `ticks="none"`; el y sí lleva marcas adentro.
- **`$\lambda(\mu/rm)$`**: el `/r` termina el comando `\mu` (cualquier no-alfabético lo cierra)
  **y** pone la `m` en romano como el original; `\mu m` dejaría el espacio visible.
- Digitalizar del **escaneo a 300 dpi**, no de `examples/polybar.png` (captura de pantalla del
  mismo escaneo, **con el cursor del ratón encima**).
- **Medir, no mirar**: la trama del SVG *parece* más densa en una comparación a baja resolución
  (antialiasing del rasterizador); a 300 dpi los tres backends coinciden.

Pendientes conocidos de polybar (ninguno lo pide un ejemplo todavía): `width` es miembro aparte
(no va en el path) → el mapeador log de `plot` no lo alcanza y en un eje x **log** el ancho de
barra queda mal; y no hay `base=` (las barras siempre crecen desde 0). Ambos en §4.12 y
`plan_polybar.md`.

### Cerrado en la sesión del 2026-07-16 (nomenclatura §13 + renombre; `plot` EN PAUSA)

Al comparar con matplotlib/asymptote salió que **los nombres de MG estaban cruzados**:
`axis(title=)` era el *nombre del eje* (el `xlabel`/`ylabel` de todo el mundo) y `axis(labels=)`
los *tick labels* — los dos nombres que cualquiera alcanza primero significaban otra cosa, y
`title` ocupaba el del encabezado del plot, que MG no tiene. Cuatro commits (`d4c5c52`..`52be31a`):

- **§13.0 "Anatomía de una gráfica"** (normativa): un nombre por parte, reservados aunque la
  parte no exista. `plot(title=)` = encabezado; `axis(label=)` = nombre del eje; `tick_labels`
  = los números. **Sin `units=`**: van en la cadena del label (separarlas obliga a una política
  tipográfica no universal — SI `λ / µm` vs libros `λ(µm)`). `tick_labels` cubre 3 modos con un
  arg (true/false/lista), como `grid=`; por el 3º (no numéricos) **no** se llama `numbers=`.
- **Renombre HECHO** (`9d52325`). **Va en DOS FASES o colisiona** (`title_font→label_font` choca
  con el `label_font` que ya existía): primero `label_*→tick_label_*`, luego `title_*→label_*`.
  Los nombres viejos **fallan en compilación** (`checkRenamedAxisArgs`). 💡 **Técnica a repetir:**
  `label_font`/`label_size`/`label_gap` son válidos ANTES y DESPUÉS con distinto significado → no
  pueden fallar y el parser no los ve; la guardia fue que **el renombre es PURO** y exigir los 57
  goldens **byte-idénticos**. fig2-3 traía la trampa pura (`label_font` y `title_font` contiguos).
- **`label_at=`** (`1f19fe9`): `"center"` (matplotlib, fig2-3) / `"start"`/`"end"` (libro). El
  default de `label_gap` depende de `label_at` y no es magia: **lo que estorba depende de dónde
  lo pongas** (centrado libra la fila de rótulos; al extremo va más allá de ella). Absorbió los
  3 rótulos `x` de fig4-5.
- 🐞 **Bug del motor destapado por ese port:** `/i` y `/b` se tragaban **en silencio** sobre cara
  heredada. Las caras componen por OR de bits pero el centinela es `FN_NOFACE = -1`, que ya tiene
  **todos** los bits → `-1 | FN_ITALIC = -1`. Solo llegan con FN_NOFACE los rótulos de
  axis/numbers/grid; se escondía porque fig6-4 usa `/i` dentro de `$…$` (math *asigna* itálica) y
  funciona. Fix en `change_font_face`. **Lección: un centinela `-1` y un modelo de bits no conviven.**
- ⚠️ **Trampa documentada (§13.7), demostrada con medición:** `yaxis(base=0, label="V")` manda el
  nombre del eje **encima de los datos** (el label cuelga de la LÍNEA, y la línea se fue al centro).
  Ligado a la pregunta abierta de §13.5: matplotlib ancla el label a la **caja** (`set_ylabel` es
  método del Axes, no del spine). Coinciden salvo con `base=`. **No se cambió porque el anclaje a
  la caja no rescata su propio caso** (los `V(x)` de fig4-5 quieren altura común, que ningún
  anclaje da). Lo decide la primera figura donde estorbe.

**`plot` EN PAUSA (2026-07-16), con ejemplos completos y funcionales:** fig2-3 (lineal), fig6-4
(log), fig4-5 (3 paneles + `base=` + `label_at`), fig_polybar (`polybar` + 3 pasadas). Aparcado en
la spec, **sin implementar**: `rule` (§13.8, valores notables — hijo del `plot`, decidido), `table`
(§13.10); y los huecos que destapó `figure_02.pdf`: **texto multilínea** (§14.1), **retícula por
eje** (§13.6) y **`alpha`** (§4.11 — EPS no lo tiene nativo; es decisión de arquitectura, no un
atributo). Esperan figuras a propósito. `legend` (§13.9) se cerró (forma explícita) el
2026-07-17, ver abajo.

### Cerrado en la sesión del 2026-07-17 (`legend`, forma explícita; §13.9)

**Fig. 1** de `docs/first_article.pdf` (p. 8) desbloqueó `legend` (§13.9), como preveía
`plan_fig1.md`: 1 curva + 2 series de marcadores, digitalizada y portada a
`examples/fig1.mg` con la leyenda **a mano** primero (línea + 2 markers, sin caja) y
luego sustituida por el `legend { entry(...) { ... } }` real. Solo la fuente **explícita**
(§13.9 punto 2) quedó implementada; la automática vía `rule` (punto 1) sigue esperando a que
`rule` exista.

- **`legend` es hijo de `plot`**, como `xaxis`/`yaxis`: coords EXTERIORES (la caja física),
  nunca mapeada por datos, dibujada AL FINAL (encima de contenido y ejes). Cero elementos
  gráficos nuevos — reusa `FitStmt::fitMatrix` (MEET, `stretch=false`) para ajustar la muestra
  de cada `entry` (un bloque arbitrario en caja unitaria 0..1) sin deformarla a elipse, y
  `AT_TALIGN`/`AT_TVALIGN`/`AT_THEIGHT` (los mismos que usa el nombre de eje de `axis`) para el
  texto.
- **`at="top-right"/"top-left"/"bottom-right"/"bottom-left"`** ancla una esquina de la caja del
  plot con `margin=` (pt) de inset. **Insight de diseño:** el lado (`"left"`/`"right"`) fija el
  borde de la COLUMNA DE MUESTRAS, no el del texto — el compilador no puede medir el ancho de
  una cadena en parse-time (los tres backends lo resuelven en DRAW-TIME, cada uno con su propio
  mecanismo: `stringwidth` en EPS, `HPDF_Page_TextWidth` en PDF, `text-anchor` en SVG). Con
  `"...-right"` la muestra TERMINA en el margen y el texto CRECE a la izquierda desde ahí con
  `align="right"` nativo del backend — compone sin medir texto en ningún caso.
- Estilo físico (pt), como `tick_size`/`label_gap`: `margin`/`sample_width`/`sample_height`/
  `gap`/`row_gap`/`font_size`. Sin marco/fondo todavía (ninguna figura lo pide); `border=`/
  `fill=` esperan a la que lo pida, mismo criterio que el resto del lenguaje.
- Verificado en los tres backends (EPS+gs, SVG, PDF) — idénticos.

**`circle-dot` (⊙) añadido a §4.6**, resolviendo el pendiente #5 de `plan_fig1.md`: la serie de
Houghton en la Fig. 1 es ⊙, no ○ (`fig1.mg` usaba `"circle"` como aproximación). Es la
EXCEPCIÓN a "forma y relleno son ejes independientes" (§4.6): una forma compuesta con relleno
mixto por construcción. No pasa por `MarkerShape`/subpaths (polígonos rectos en caja unitaria;
un anillo necesita arco real y una sola forma no puede tener dos subtrayectos con relleno
distinto) — se resuelve en `Dot::draw` (`src/primitives.cpp`) como DOS `g.dot()` reales
superpuestos (anillo `setFilled(false)` + punto central al 30% del radio `setFilled(true)`),
igual que `circle` ya se resolvía como arco real y no polígono. Proporción de la digitalización
de Fig. 1 (anillo ~23px, punto ~6px de diámetro). Cero cambios de backend — reusa `Display::dot`/
`setFilled` en los tres. `make` limpio (sin warnings nuevos).

**`fig1.mg` entró al corpus golden** (`test/run.sh`, 19→20 ejemplos, `ok=63`): es el único que
ejercita `legend` y `circle-dot`. Ajuste de nombre en el `EXAMPLES=` del harness; sin cambios al
motor. Los ajustes finos de posición de los `label` de los ejes se dejaron como están —
Alejandro los revisó y decidió no perseguirlos ("es complicarse demasiado").

### Cerrado en la sesión del 2026-07-18 (álgebra §9 madura + fig16-9 Franck-Condon)

**Motor/parser** (todo verificado con golden; único churn: fig4-1, raster idéntico px a px):
- **`concat` variádico y SIN auto-reversión** (semántica de la spec §9; la heurística de
  extremos más cercanos elegía mal con piezas cortas — una recta de media unidad + un medio
  ciclo de coseno se soldaban por el extremo equivocado, 0.25 vs 1.25 de distancia).
  **`reverse()`** nuevo (inline en `splines.h`) es la forma explícita de orientar.
  `curvas3.mg` migró a `concat(reverse(flip_x(&H)), &H)`.
- **`sine` como expresión de path §9** (`PathSine`; generación extraída a `sineBezierPath`,
  parseo compartido `parseSineArgs`). Con `phase=90/270` cada llamada es un medio ciclo de
  coseno entre extremos con pendiente cero → funciones de onda con envolvente por tramo se
  arman con `concat` de piezas de distinta `amplitude`, C1 gratis (uniones en extremos).
  *(El `phase` por cuartos de ciclo YA estaba implementado —commit `2e1ba32`— aunque
  `plan_sine.md` y el header de `sines.mg` decían lo contrario; actualizados.)*
- **`smooth { }` §9.2 implementado** (`path_to_bezier` ya existía): extiende extremos por
  REFLEXIÓN (duplicarlos da distancia 0 → NaN en `get_bezier_tangents`, que no tiene las
  guardas del spline).

**`examples/fig16-9.mg`** (Franck-Condon, dos Morse + funciones de onda vibracionales
rellenas) en el corpus golden (16→17 ejemplos, `ok=51`). Único que ejercita sine-como-path,
concat variádico, `reverse` y bezier con `fill`+`outlinefill`. Lecciones del port:
- Las ψ **no son senoides**: tocan la base con pendiente cero y sus extremos caen en las
  uniones. La pieza es el medio ciclo de coseno (`sine(phase=90/270)`), no `sine` pelada.
- **Semántica V1 descubierta midiendo el render** (el EPS "V1" pasó por Illustrator y no es
  legible): `BZ` = controles bezier CRUDOS (el código de `Parser.cpp` manda; ambos modelos
  ajustan igual, rms ~1.2 px); `SCPT`/`SCST` COMPONEN (multiplicativo hasta `IDPT`/`IDST` —
  lo prueba que pw6 cierra en la base); `PWST tx ty sx sy` = trasladar+escalar en unidades
  de ventana; placement de struct lleva el factor `docwmin=1.3` en ambos ejes.
- Colocaciones convertidas a UNA ventana en cm (los 2 `WW` de V1 eran solo mecanismo de
  colocación); niveles verificados a ±1 px contra el oráculo; rects de las curvas por
  mínimos cuadrados (rms 1.3 px). La figura publicada trae la rama izquierda inferior
  **extendida a mano** (edición Illustrator) → se replica con una polyline por la tangente.
- El punto 32 de la curva V1 sobra del agrupamiento 1+3k y se descarta (igual que V1);
  texto rotado: `{ translate x y  rotate 90  text(...) { 0 0 } }` (el ancla se da en el
  marco YA transformado — `rotate` solo + ancla original la manda fuera del lienzo).

Siguiente concreto — el traductor **`mg1to2.py`** (`plan_mg1to2.md`, actualizado 2026-07-11 con
los mapeos correctos: GNPATH+DOT→for/dot, SCST, LNST gap, aspecto de ventana) es el gran
pendiente para migrar el material V1. Otros: `spline`/`smooth` §9 (motor `splines.cpp` listo,
bajo costo); Math P1/P2 de `plan_lmmath.md` (símbolos `map_symbol`→LM Math; latino math→itálica
LM Math en vez de Times-Italic); `marker_start/mid/end` en polygon/bezier; ventanas anidadas §16.

**Hueco de cobertura CERRADO** (follow-up #3 del code-review, 2026-07-16): el remapeo de posición
de `text()` en un `plot` **log** —la razón única de los 2 accesores que ganó el motor en la Fase 4—
ya lo ejercita **fig6-4**: sus dos rótulos de isótopo (`Po²⁹²`, `U²³⁸`) pasaron de coords de ventana
a **coords de datos DENTRO del plot**. Lo propuso Alejandro al notar que esos `text()` "se ponen por
fuera". El port **verificó el mapper de paso**: las anclas nuevas cayeron a **0.002 pt y 0.000 pt**
en `y` (el eje log) de las que el autor había fijado a mano — o sea, el remapeo puntual reproduce la
posición exacta. Los 0.026 pt de `x` son el redondeo de la inversión a 3 decimales. El `λ⁻¹(s)`
sigue fuera, y bien: es el **nombre del eje** (mobiliario de página, horizontal-arriba), no una
anotación de datos.

### Cerrado en la sesión del 2026-07-18 (`exp`/`ln` + `franck_condon.mg` paramétrico)

**`exp`/`ln` (§5.2)** entran por la vía que la spec tenía reservada —"solo cuando el
corpus lo exija"— con la figura que las exige (`c61a67e`). **Un potencial de Morse no es
escribible sin ellas**: por eso la curva de `fig16-9` está digitalizada, no por
preferencia. `ln` de argumento no positivo es `evalError` FATAL.

**`examples/franck_condon.mg`** (corpus golden, 17→18 ejemplos, `ok=54`) — diagrama de
Franck-Condon donde **nada está medido**, contraparte de `fig16-9.mg`, que se queda como
port fiel de la figura publicada. Se dan `a`, `re`, `we`, `xe` de cada estado electrónico
más `Te`, y salen en forma cerrada la curva `D(1-exp(-a(r-re)))²`, los niveles
`we(v+½)-we·xe(v+½)²`, los retornos `re - ln(1∓sqrt(E/D))/a` y `vmax = 1/(2·xe)-½`.
Único que ejercita `exp`/`ln` y la construcción de path inline en una invocación de
struct. Lo durable:

- **`D` NO es parámetro libre**: la relación de Morse `D = we/(4·xe)` lo fija. Darlo por
  separado mete niveles sobre la disociación y el retorno externo deja de existir. Lo
  cazó el `evalError` de `ln` — un error de **física** reportado como error de
  compilación. Es el argumento más fuerte a favor de que `evalError` sea fatal.
- **La onda son v+1 lóbulos ψ que CRUZAN la línea de nivel**, cada uno un medio ciclo
  entre extremos consecutivos (pendiente cero en la unión → empalme liso). ⚠️ **La
  construcción de `franck_condon.mg` cambió el 2026-07-19** de jorobas |ψ|² (una `sine`
  pelada, apoyada sobre la línea) a ψ con signo + envolvente WKB, armada con `path +=` (ver
  la sesión de 2026-07-19 abajo). El resto de esta nota es sobre fig16-9, que **no** cambió.
- **fig16-9 — ψ verdadera por lista explícita:** piezas `sine(half_cycles=1, phase=270/90)`
  con **amplitudes alternas** — las dos exteriores a la mitad (`sube10`/`baja10`), las
  interiores completas (`sube20`/`baja20`) —, **v+2 piezas → v+1 lóbulos**. Arranca y acaba
  en 0 con pendiente cero (empalma liso con las colas) y cruza el eje. El desnivel de
  paridad no aparece porque el autor elige `sube`/`baja` de arranque según la paridad. La
  cuenta v+2 vale para ESTA construcción (extremo-a-extremo); la de un solo `sine(phase=270)`
  necesitaba 2v+2 y salía chueca en v impar — por eso se abandonó (ver 2026-07-19).
- 💡 **La invocación de struct acepta una EXPRESIÓN DE PATH INLINE** —
  `Nivel(concat(&plana, sine(half_cycles=v+2, …), &plana))`— así que un `for` construye la
  onda con su propia `v`. Es la vía para no declarar N paths casi iguales, y **no requiere
  tocar el lenguaje**: 240 líneas → 150, dos lazos de 12. (Las **listas no admiten paths**
  §5.1, y **no hay funciones escalares definibles**, así que una fórmula repetida sí se
  reescribe. Documentado en §5.2.)
- La vertical de Franck-Condon aterriza en v'=6 del excitado (7.777 eV, entre 7.749 y
  7.934) **sin que nadie la coloque ahí**, y las alturas de las ondas salen del
  espaciamiento al siguiente nivel → se achatan solas hacia la disociación (el efecto que
  V1 había puesto a mano).

**Familia de reducciones path→número completada** (`fe7115d`, `f2edb71`, decisión 7 de
`plan_struct_params.md`): `path_x_min_at_y`/`path_x_max_at_y(&p, y [, expand])`. Van
**dos** expresiones porque una expresión de MG devuelve **un** número; `expand` ensancha
por **fracción del tramo**, no cantidad absoluta (permite alojar colas sin conocer de
antemano el ancho, que es lo que se estaría calculando). ⚠️ **Sin uso en el corpus** —
commit aparte a propósito, para que revertirlo sea trivial si en unos meses sigue así.

**La medición que las dejó sin caso** (detalle y tablas en `plan_struct_params.md`):
- El **sesgo del polígono de control** frente a la bezier real es ≤0.72 pt (típico
  0.1–0.5), por debajo del ruido de la digitalización → **no hace falta teselar**,
  resolviendo la advertencia que el plan dejaba abierta.
- Pero **los rects publicados de fig16-9 no son los puntos de retorno**: los exceden
  hasta 17 pt, con la razón de ensanche derivando 2.02→2.85 y el descentrado creciendo
  monótono (−2.8→−7.8 pt). Varios empiezan a la **izquierda de donde arranca la curva
  digitalizada**, o sea sobre la pared extendida a mano, que no es dato. Esa firma —error
  suave y creciente— es la de un dibujo **a ojo**. Derivarlos sería **rediseñar** la
  figura, no refactorizarla; por eso fig16-9 se queda como está y la versión con retornos
  exactos es la paramétrica.
- **Para una curva con fórmula, la forma cerrada gana**: más exacta y más barata que
  intersecar la curva ya dibujada. El lugar de estas funciones es una curva **empírica**.

**Higiene del cambio suelto de Alejandro** (`fe7115d`): `getXBoundsAtY` →
`path_x_bounds_at_y` (snake_case como el resto de `splines.h`), contrato del retorno al
header, y **vuelta a C++14** — el bump a C++17 lo pedían solo un `[[nodiscard]]` y un
binding estructurado, ambos evitables, y el `[[nodiscard]]` estaba en la **definición**,
donde ningún llamador lo ve (verificado compilando uno de prueba).

⚠️ **`git status` no basta para saber si el árbol está limpio.** Un archivo reescrito con
el mismo tamaño y el mtime restaurado se salta la comparación de contenido por el
stat-cache: en esta sesión `examples/fig16-9.mg` traía 12 líneas de más (una curva de
depuración) que `status` no reportaba y `git diff HEAD` sí.

### Cerrado en la sesión del 2026-07-18 (structs parametrizadas por path + `fit` de invocación; `plan_struct_params.md`)

**Hueco 3 primero, en commit aparte** (`00feb03`): `structBox` recibía el ámbito del
LLAMADOR en los tres sitios que lo invocan (`InvokeStmt`, `FitStmt`, `buildStructure`), así
que un `world_window` del cuerpo que usara un parámetro (`world_window 0 w -2 2`) fallaba
con `variable no definida: w`. Los tres pasan ahora el ámbito LOCAL, con los parámetros ya
ligados. Verificado sin mover bytes (`ok=51`): ningún `world_window` del corpus llevaba
identificadores.

**Parámetro de tipo path** (`struct Nivel(&onda, w = path_width(&onda))`, commit `1c28197`):
el sigilo `&` en la lista de parámetros marca el tipo (`StructDef::paramIsPath`, paralelo a
`params`). `Scope` gana `pathBindings`: un mapa con `PathBinding{expr, scope}` — el `scope`
es el ámbito del LLAMADOR, no el de la struct que declara el parámetro, así que un path
pasado como argumento resuelve sus propias referencias (p. ej. un `sine(amplitude=a)` con
`a` local del llamador) donde corresponde, no en el ámbito del cuerpo que lo recibe.
`PathRef::evalPath` consulta `pathBindings` ANTES que `g_paths`: un parámetro path
**ensombrece** un path global homónimo, deliberado. Un solo tipo `Arg{ExprPtr e; PathExprPtr
p;}` (nunca ambos) mantiene la lista de argumentos en el mismo índice que
`params`/`paramIsPath` — con listas paralelas separadas, `Nivel(&pw0, 3)` perdería la
correspondencia posicional. `bindStructParams` (compartida por `InvokeStmt`/`FitStmt`) liga
en DOS pasadas: todos los parámetros path antes que cualquier default, para que
`w = path_width(&onda)` encuentre `onda` ligado sin depender del orden de declaración.

**`fit(Struct(args), stretch=…) { rect }`**: `fit` acepta la misma invocación paramétrica
que la llamada directa (desambiguación por `T_LPAREN` tras el nombre, sin tocar la rama de
`fit(&path)` de §9). `at=`/`rotate=`/`scale=` dentro de esa invocación son **error**, no se
ignoran: competirían con la matriz del propio `fit`.

**`path_width(&p)`** (reducción path→número, extensión en x del bbox; `path_bbox()`
factorizado de la rama `fit(&path)` existente): puente Expr↔PathExpr en `parseAtom` (antes
de armar el `CallExpr` genérico), declarado junto a `parseUnary` y definido junto a
`parsePathExpr` — evita reordenar el archivo, el punto de enganche vive muy por encima de
donde vive `PathExpr`. Nombre **provisional**: primer miembro construido de una familia
reservada (`path_height`, `path_x_bounds` — ver `getXBoundsAtY` en `splines.cpp`, el cambio
suelto de Alejandro que motivó la familia); hereda su advertencia (exacto sobre paths
monótonos en x, como las ondas de fig16-9; sobre una bezier genuinamente curva opera sobre
el polígono de control, no la curva).

**`examples/fig16-9.mg`** (commit `1311eda`): las 7 structs casi idénticas `Nivel0..Nivel6`
colapsan a una — `struct Nivel(&onda, w = path_width(&onda))` —, y los 13 `fit` pasan a
`fit(NivelN(&pwK), stretch=true)` con los mismos rectángulos. Refactor puro, golden
byte-idéntico (`ok=51`) en cada paso, incluida la reescritura del ejemplo.

Errores verificados a mano (sin corpus, cero cobertura del golden): path donde va número,
número donde va path, `&nombre` no definido, argumento path faltante (aridad), y
`at=`/`rotate=`/`scale=` dentro de `fit(Struct(...))` — los cinco con mensaje claro y
`exit 1`. Vida del `Scope*` de `PathBinding`: segura por construcción — apunta siempre a un
ámbito ANCESTRO en la misma cadena de llamadas síncrona (`exec()` anidado, sin nada
diferido ni asíncrono), verificado también con reenvío de dos niveles (`Outer(&onda)` que
hace `fit(Inner(&onda), ...)` en su cuerpo).

Pendiente (decisión 8, nadie lo pide todavía): `place`/`repeat` tienen el mismo hueco 1 (no
aceptan invocación paramétrica) pero se dejan sin construir.

### Cerrado en la sesión del 2026-07-19 (`path +=` acumulación + envolvente WKB en franck_condon)

**`path w += pieza` (§9)** — construir una curva cuyo nº de piezas depende de una variable
(un `for`), que `concat` (variádico pero de aridad fija en el fuente) no cubre. Motivado por
la figura de Franck-Condon de Wikipedia: ψ **con signo** (cruza la línea de nivel) y con
**envolvente WKB** (amplitud por lóbulo). Huella mínima: token `T_PLUSASSIGN` (`+=`, solo en
la rama `path`), y en `parserv3.cpp` `FrozenPath` (path ya evaluado a puntos) + `PathAppendStmt`.
Cero cambios en los tres backends.
- **`+=` evalúa YA** (exec-time), a diferencia de `path w = <expr>` que es **diferido**
  (guarda el árbol, se evalúa al dibujar). Por eso `+=` puede leer variables del lazo: lee el
  path actual, concatena en el ámbito vigente y **congela**. Verificado byte-idéntico a un
  `concat` explícito equivalente.
- 🐞 **Bug latente arreglado de paso:** `PathDeclStmt::exec` hacía `std::move(expr)` → un
  `path w = &plana` **dentro de un `for`** se anulaba en la 2ª vuelta. Nunca disparó porque
  ningún path se declaraba en un lazo. Ahora g_paths guarda un `PathAlias` **no-propietario**
  al árbol que posee el Stmt (el AST sobrevive a exec/draw) → la declaración es re-ejecutable,
  necesario para re-sembrar el acumulador cada vuelta.
- **No hay aliasing draw-time:** las primitivas evalúan su path en exec-time y lo **congelan**
  en el `GraphicsItem` (`PrimStmt::exec`, `parserv3.cpp:1413`), así que reasignar `w` en la
  siguiente vuelta no afecta a las ondas ya emitidas. Esto es lo que hace seguro el patrón.

**`examples/franck_condon.mg` — ondas ψ con envolvente WKB, y UN solo lazo para los dos pozos:**
- La envolvente sale en forma cerrada de lo que el archivo ya tenía: amplitud del lóbulo ∝
  `(E−V(r))^(−¼)` (WKB), evaluada en la posición de cada pico. **La masa y ħ se cancelan** al
  normalizar al rect del `fit`, así que —a diferencia de la profundidad de penetración— **no**
  pide ningún parámetro nuevo. Grande cerca de los retornos, **máxima en el externo** (pared
  tendida): asimetría físicamente correcta, como Wikipedia y `pw6` de fig16-9 (que la aproxima
  a ojo, simétrica).
- **Cada lóbulo es un medio ciclo entre extremos consecutivos** (pico en el JOIN, pendiente
  cero → empalme liso). El `for k` acumula con `+=`; amplitud de la pieza = |swing|/2, phase
  270/90 según dirección. Todas las 14 ondas vuelven a nivel (verificado: primer y = último y).
- **Los dos pozos colapsan a un lazo** con los parámetros en **listas §5.1** indexadas por
  `pozo` (`aa=[a1,a2]`, `TT=[0,Te]`, `lab0=["$v''=0$","$v'=0$"]`…) — así la construcción WKB
  aparece UNA vez, no duplicada. Ejercita listas-con-variable + `+=` juntas. (Verificado que
  un literal de lista acepta variables: `TT=[0, Te]`.)
- Este ejemplo pasó de |ψ|² (jorobas apoyadas) a ψ con signo: es la forma que **explica** por
  qué ciertos traslapes de Franck-Condon se cancelan (el argumento es `|⟨ψ'|ψ''⟩|²`, con signo).
  El caption original (confirmado por Alejandro: la figura fue **eliminada del libro**) no
  menciona funciones de onda → las ondas son ilustración, pero valía hacerlas fieles.
- La vertical de Franck-Condon aterriza en v'≈6 del excitado **como resultado**, no colocada
  (sale del desplazamiento `re1=1.15` vs `re2=1.48`); los números de nivel la hacen legible.

**Penetración asimétrica en la región prohibida (efecto túnel), añadida después:** ψ no se
anula en el retorno, decae hacia afuera, y **más en la pared tendida que en la empinada**.
- ⚠️ **La fórmula "sencilla" tiene trampa:** la longitud de penetración NO es `1/√(V−E)` (que
  **diverge** en el retorno, donde `V−E=0`, porque ahí WKB se rompe y manda Airy). Es la
  **escala de Airy** `d ∝ |V'(retorno)|^(−⅓)` — raíz **cúbica** de la pendiente.
- **El cociente entre paredes sale sin masa ni ħ:** con `|V'(r±)| = 2a√(DE)(1∓s)`, `s=√(E/D)`,
  el factor común se cancela → `d_out/d_in = ((1+s)/(1−s))^⅓`. Solo `s`. ≈1 en v bajo, →2 en
  v=6 (verificado: colas 5.1↔5.9 pt en v=0, 4.2↔8.3 pt en v=6, monótono). La profundidad
  ABSOLUTA sí llevaría m/ħ (por eso se fija una escala visual, media geométrica 0.5); la
  ASIMETRÍA no. Es más fiel que Wikipedia, que la dibuja simétrica y a ojo.
- **Implementación sin motor nuevo:** las colas planas pasaron de `&plana` (ancho fijo 0.5) a
  `sine(half_cycles=1, phase=0, amplitude=0) { 0 0  (t) 0 }` — una pieza **plana de ancho
  paramétrico** (amplitude=0 → línea recta). Con anchos `tL≠tR` y el rect del `fit` extendido
  asimétricamente (`rm−gL … rp+gR`, `g=t·sx`, `sx=(rp−rm)/(v+2)`), la oscilación sigue cayendo
  exacta en `[rm,rp]` y cada cola ocupa su voladizo. El fit afín preserva las proporciones.

**`examples/fig16-9.mg` RETIRADO** en el mismo commit (`5fc995a`): la figura fue **eliminada
del libro**, así que el port fiel dejó de tener original al que ser fiel. Sale de `test/run.sh`
y del corpus (18→**17** ejemplos, `ok=54`→**`ok=51`**); el puntero de `sines.mg` se reapuntó.
Las notas de sesiones anteriores que lo describen en el corpus siguen siendo válidas como
historia; su cobertura **no se perdió** (verificado por grep sobre el corpus): sine-como-path
→ `franck_condon`/`sines`, `concat` → `franck_condon`/`curvas3`, `reverse` → `curvas3`/`fig2-5`,
structs parametrizadas por path (`&`, `path_width`) → `franck_condon`.

`ok=51`. `especificacion_mg.md` §9 documenta `path +=`.

## Code style

[Orthodox C++](https://gist.github.com/bkaradzic/2e39896bc7d8c34e042b): no RTTI, no exceptions; `std::unique_ptr` for ownership, raw pointers non-owning. `-Wall -Wpedantic -Wsuggest-override`, warnings-clean. In headers: fully qualified `std::` (no `using` at namespace scope), `override` on all overrides, include guards `MG_*_H` (never `__*`), in-class member initializers. Project language for comments/messages is Spanish; keep new features in the compiler itself (no external preprocessors).
