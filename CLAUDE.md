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

bash test/run.sh check    # golden (EPS+SVG+PDF) + gs + paridad: ok=54 fail=0 error=0 psfail=0 c3fail=0
bash test/run.sh capture  # re-bless goldens (only after verifying changes are intended)
```

**Harness golden ACTIVO (reactivado 2026-07-11; ampliado 2026-07-14/15).** Corre el corpus
de `examples/` (18 `.mg` × EPS/SVG/**PDF** = 54 goldens) y compara contra la red golden
(salida del propio renderer V3, regresión — no el oráculo V1). Tras tocar el motor:
`make` y `bash test/run.sh check` (debe dar **ok=54 fail=0 error=0 psfail=0 c3fail=0**);
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

Toolchain: `clang++`/`g++` (C++14, `-fno-rtti -fno-exceptions`), `flex` (regenerates `src/lexmg.cpp` from `src/mgpp.l`), `pandoc` (man page). Do not edit `src/lexmg.cpp` or `include/version.h` content by hand. libharu is vendored in `third_party/` for PDF.

## Layout

Headers in `include/`, sources in `src/`, binary in `bin/`, regression harness in `test/`.

**Política V1 (2026-07-15):** todo el trabajo actual es desarrollo de **V3**; **no se trackea material V1 nuevo**. Los `.mg` crudos de V1 y sus traducciones literales se quedan en el árbol **sin commitear** (p. ej. `examples/fig4-5v1.mg`, `fig4-5v3.mg`, y los de fig6-4). Lo ya trackeado en `examples/v1/` (31 archivos: corpus congelado + oráculo) **se queda como está** — la política aplica de aquí en adelante, no se borra nada.

The example corpus is split for the V1→V3 transition (see `examples/v1/README.md`):
- **`examples/v1/`** — frozen V1-syntax corpus (two-letter commands). Serves as translator fixtures + provenance. `examples/v1/reference/*.svg` are the committed **migration oracle**: renders produced while the compiler still parses V1 (SVG chosen for size; SVG/EPS/PDF match). These SVGs are force-included past the `*.svg` gitignore.
- **`examples/`** (raíz) — corpus V3 **compilable** con `bin/mg` (18 `.mg`: arrow, curvas3, fig2-1/2-3/2-6, fig4-1/4-5/4-10, fig6-1/6-10, fig6-4v3-clean, fill_styles, line_patterns, markers-demo, primitives, rpstest, sines, texto). El corpus es una **lista explícita** en `test/run.sh`, no un glob: un `.mg` nuevo en la carpeta no entra solo (por eso conviven ahí archivos crudos sin commitear, p.ej. `fig4-5v1.mg` en sintaxis V1, que no compila con V3). Se movió aquí desde `examples/v3/` el 2026-07-09; sus salidas **ya no están atadas** al oráculo V1 (dejan de ser traducción 1:1 y pasan a ejercitar/mostrar la gramática V3). Es el corpus de la red golden (`test/run.sh`, reactivada 2026-07-11). `fig6-4v3-clean` entró el 2026-07-14: es el único que ejercita eje **log** + `fit(stretch)` + math con superíndices + `extend` + ticks-in, y el único **sin `font` explícito** — por eso es el que caza el bug de cara ambiente en PDF (fig2-3 no lo detecta: fija `font "italic"`). Su oráculo de calibración es `examples/fig6-4.png`.

**Cutover hecho (§22.6):** `bin/mg` en `main` **es el compilador V3** (se arma de `src/parserv3.cpp` + `src/lexv3.cpp` + motor + PDF/haru). `test/run.sh` compila el corpus de `examples/` con la salida del propio renderer V3 como red golden (regresión, no el oráculo V1); **reactivado 2026-07-11** (`ok=32`, ver "Build and test"). `src/main.cpp`, `src/Parser.cpp`, `src/lexmg.cpp` (front-end V1) quedan en el árbol pero **fuera del build**; V1 sigue congelado en `v1-legacy`. `make v3test` es un alias (`cp bin/mg bin/v3test`).

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

Siguiente concreto — el traductor **`mg1to2.py`** (`plan_mg1to2.md`, actualizado 2026-07-11 con
los mapeos correctos: GNPATH+DOT→for/dot, SCST, LNST gap, aspecto de ventana) es el gran
pendiente para migrar el material V1. Otros: `spline`/`smooth` §9 (motor `splines.cpp` listo,
bajo costo); Math P1/P2 de `plan_lmmath.md` (símbolos `map_symbol`→LM Math; latino math→itálica
LM Math en vez de Times-Italic); `marker_start/mid/end` en polygon/bezier; ventanas anidadas §16.

## Code style

[Orthodox C++](https://gist.github.com/bkaradzic/2e39896bc7d8c34e042b): no RTTI, no exceptions; `std::unique_ptr` for ownership, raw pointers non-owning. `-Wall -Wpedantic -Wsuggest-override`, warnings-clean. In headers: fully qualified `std::` (no `using` at namespace scope), `override` on all overrides, include guards `MG_*_H` (never `__*`), in-class member initializers. Project language for comments/messages is Spanish; keep new features in the compiler itself (no external preprocessors).
