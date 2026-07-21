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

bash test/run.sh check    # golden (EPS+SVG+PDF) + gs + paridad + docs/img: ok=57 … imgfail=0
bash test/run.sh capture  # re-bless goldens (only after verifying changes are intended)
bash test/run.sh images   # regenera docs/img/*.svg (salida PUBLICADA; capture NO la toca)
```

**Harness golden ACTIVO (reactivado 2026-07-11; ampliado 2026-07-14/15/17).** Corre el corpus
de `examples/` (19 `.mg` × EPS/SVG/**PDF** = 57 goldens) y compara contra la red golden
(salida del propio renderer V3, regresión — no el oráculo V1). Tras tocar el motor:
`make` y `bash test/run.sh check` (debe dar **ok=57 fail=0 error=0 psfail=0 c3fail=0 imgfail=0**);
re-bendecir con `capture` solo tras verificar que los cambios son intencionales. Golden
files (`test/golden/`) **no están en git** (se regeneran con `capture`).

**Cuatro compuertas, cada una caza una clase distinta** (razonadas en `plan_plot.md`,
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
- **`docs/img` al día** (`imgfail`, nueva 2026-07-21) — caza que la salida **publicada** se
  quede RANCIA. Esos `.svg` **sí están en git** (GitHub los muestra en la portada del README)
  y se regeneran a mano; nada los vigilaba, y entre el 2026-07-17 y el 2026-07-21 la portada
  mostró la tipografía matemática *anterior* a la migración a LM Math — anunciaba una mejora
  que ella misma no exhibía. Un ejemplo entra a la compuerta por el hecho de tener un `.svg`
  con su nombre en `docs/img/`: la presencia del archivo ES la declaración, no hay lista que
  mantener. ⚠️ **`capture` NO los regenera, a propósito** — `test/golden` es borrador local
  sin trackear (bendecir es barato), pero `docs/img` es salida publicada y bendecirla tiene
  que ser un commit consciente. Para eso está `bash test/run.sh images`, modo aparte.

Las cuatro compuertas se verificaron reintroduciendo a propósito los bugs que deben cazar
(la de `docs/img`, con el archivo rancio **real** de `e9198c0`: lo caza, y el golden sigue
dando `ok=57` — que es justo la prueba de que el golden no puede verlo).

Toolchain: `clang++`/`g++` (C++14, `-fno-rtti -fno-exceptions`), `flex` (regenerates `src/lexmg.cpp` from `src/mgpp.l`), `pandoc` (man page). Do not edit `src/lexmg.cpp` by hand (flex generates it); `include/version.h` **is** edited by hand. libharu is vendored in `third_party/` for PDF.

## Layout

Headers in `include/`, sources in `src/`, binary in `bin/`, regression harness in `test/`. **Herramientas Python en `tools/`** (movidas de la raíz el 2026-07-21): el traductor `mg1to2.py` (§20) y el puente de datos `hist2mg.py` (CSV/XLSX → histogramas y estadísticas en `.mg` incluible). Son auxiliares **fuera del compilador** — no se ligan a `bin/mg` ni el lenguaje depende de ellas; la política de "sin preprocesadores externos" del Code style se refiere a la compilación de un `.mg`, no a preparar datos antes. `test/run_translator.sh` apunta a `tools/mg1to2.py`. Design/working notes — the `plan_*.md` files plus `audit_text_parser.md` and `notas_at_anchor.md` — live in **`docs/plans/`** (moved out of root 2026-07-17; `docs/` also holds published source PDFs). References throughout the tree cite them **by bare filename** (grep the name, e.g. `plan_plot.md`), not by path. The forward-looking spec (`especificacion_mg.md`) and the pending-work board (`PENDIENTES.md`) stay in root.

**Política V1 (2026-07-15, endurecida 2026-07-20):** todo el trabajo actual es desarrollo de **V3**; **no se trackea material V1 nuevo**. Los `.mg` crudos de V1 y sus traducciones literales se quedaban en el árbol sin commitear; desde el 2026-07-20 **se borran** en cuanto el port V3 está cerrado (así se fueron `fig4-8.mg`, `exp.mg`, `fp3i2dat.mg` y el `fig4-8.eps` de 1998). No estaban en git en **ninguna** rama —`origin/v1-legacy` incluida—, así que el borrado es definitivo: antes de borrar, lo que debe sobrevivir son las **medidas**, transcritas al `.mg` o a un `plan_*.md`, no el archivo (ver el encabezado de `turning_points.mg`, que conserva cómo re-medir el PDF vectorial de Cambridge y los nodos del V(x) manual). Lo ya trackeado en `examples/v1/` (31 archivos: corpus congelado + oráculo) **se queda como está**.

The example corpus is split for the V1→V3 transition (see `examples/v1/README.md`):
- **`examples/v1/`** — frozen V1-syntax corpus (two-letter commands). Serves as translator fixtures + provenance. `examples/v1/reference/*.svg` are the committed **migration oracle**: renders produced while the compiler still parses V1 (SVG chosen for size; SVG/EPS/PDF match). These SVGs are force-included past the `*.svg` gitignore.
- **`examples/`** (raíz) — corpus V3 **compilable** con `bin/mg` (18 `.mg`: curvas3, fig1, fig2-1, fig2-5, fig4-1, fig4-4, fig6-4, fig_polybar, fill_styles, franck_condon, line_patterns, markers-demo, primitives, quickstart, rpstest, sines, texto, turning_points). El corpus es una **lista explícita** en `test/run.sh`, no un glob: un `.mg` nuevo en la carpeta no entra solo. **Nomenclatura (2026-07-20):** los nombres siguen a la **edición de Cambridge 2025** (descargable gratis → la referencia más fácil de verificar por un lector), no a los nombres de archivo de V1 ni a ediciones previas. Por eso el 2026-07-20 `fig4-5`→**`fig4-4`** (Fig. 4.4, p. 78): **va en DOS FASES o colisiona**, y la guardia es que el renombre sea PURO (los goldens de `fig4-4` salieron byte-idénticos a los del antiguo `fig4-5`). **Y al revés:** un ejemplo que deja de reproducir su figura publicada pierde el número y toma nombre de la física (`turning_points`, como `franck_condon`) — el número de figura es una promesa de fidelidad. Se movió aquí desde `examples/v3/` el 2026-07-09; sus salidas **ya no están atadas** al oráculo V1 (dejan de ser traducción 1:1 y pasan a ejercitar/mostrar la gramática V3). Es el corpus de la red golden (`test/run.sh`, reactivada 2026-07-11). **Poda 2026-07-17** (`arrow`, `fig2-3`, `fig4-10`, `fig6-1`, `fig6-10` eliminados: redundantes o `arrow.mg` que renderizaba vacío tras migrar sus flechas a marcadores built-in). `fig6-4` (renombrado desde `fig6-4v3-clean` el 2026-07-15) entró el 2026-07-14: es el único que ejercita eje **log** + `fit(stretch)` + math con superíndices + `extend` + ticks-in, y el único **sin `font` explícito** — por eso es el que caza el bug de cara ambiente en PDF.

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

### Cerrado en la sesión del 2026-07-15 (`base=` de ejes + `fig4-4` a 3 paneles)

- **`base=` en `xaxis`/`yaxis` dentro de `plot`** (§13.7): el eje cruza en el valor dado, en
  unidades de datos del eje **perpendicular**, en vez de quedar clavado al borde de `box`
  (`by0`/`bx0` literales). Reusa el `mapAxis` de `PlotStmt` → log gratis; `grid=` NO se mueve
  (barre la caja completa). ~20 líneas, cero churn en el corpus.
- **`examples/fig4-4.mg`** (Fig. 4.4 de Cambridge 2025 p. 78; entonces llamada fig4-5 — potenciales y estructura del espectro), en el
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
- **Aquí SÍ se vale digitalizar** (a diferencia de fig4-4): es de **1988**, V0 imprimía directo
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
  3 rótulos `x` de fig4-4.
- 🐞 **Bug del motor destapado por ese port:** `/i` y `/b` se tragaban **en silencio** sobre cara
  heredada. Las caras componen por OR de bits pero el centinela es `FN_NOFACE = -1`, que ya tiene
  **todos** los bits → `-1 | FN_ITALIC = -1`. Solo llegan con FN_NOFACE los rótulos de
  axis/numbers/grid; se escondía porque fig6-4 usa `/i` dentro de `$…$` (math *asigna* itálica) y
  funciona. Fix en `change_font_face`. **Lección: un centinela `-1` y un modelo de bits no conviven.**
- ⚠️ **Trampa documentada (§13.7), demostrada con medición:** `yaxis(base=0, label="V")` manda el
  nombre del eje **encima de los datos** (el label cuelga de la LÍNEA, y la línea se fue al centro).
  Ligado a la pregunta abierta de §13.5: matplotlib ancla el label a la **caja** (`set_ylabel` es
  método del Axes, no del spine). Coinciden salvo con `base=`. **No se cambió porque el anclaje a
  la caja no rescata su propio caso** (los `V(x)` de fig4-4 quieren altura común, que ningún
  anclaje da). Lo decide la primera figura donde estorbe.

**`plot` EN PAUSA (2026-07-16), con ejemplos completos y funcionales:** fig2-3 (lineal), fig6-4
(log), fig4-4 (3 paneles + `base=` + `label_at`), fig_polybar (`polybar` + 3 pasadas). Aparcado en
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

### Cerrado en la sesión del 2026-07-20 (`spline` retirada, `smooth` maduro, Fig. 4.5)

**`spline` y las splines cónicas RETIRADAS (§9.1).** `spline` era la misma curva que
`smooth` con otro nombre —Catmull-Rom *pasa por* sus puntos de control, así que la
distinción que la spec afirmaba no existía en la geometría— pero descartando los extremos
en vez de recuperarlos por reflexión. Las cónicas no se pierden nada: la cuadrática es
subconjunto **exacto** de la cúbica por elevación de grado, los tres backends hablan cúbica
nativa y los círculos exactos ya los da `arc`/`ellipse`. Dato de archivo: **`$S 1` nunca se
implementó** (el `switch` de `Parser.cpp` solo atiende `n==0` y `n>1`), o sea que las cónicas
murieron con el paso a EPS de 1991. El parser V3 **nunca** aceptó `spline`, así que la
retirada fue puramente documental. Del modo `nodes=n` sobrevive la idea de un `sample(&p,n)`
del álgebra §9 (PENDIENTES): muestrear produce **datos**, no tinta.

**Tres defectos del motor en `splines.cpp`**, dormidos porque nada del corpus usaba `smooth`:
- **Parametrización:** `alpha=0.5` se aplicaba a la distancia **al cuadrado** → exponente 1
  sobre la distancia = **cordal**, no centrípeta como decían el comentario y la spec. El
  exponente pasa a `alpha*0.5`. Afecta a `get_bezier_tangents`, la única función viva.
- **`conversion_factor` 10 → 6.** No era ajustable: es el límite uniforme de la fórmula no
  uniforme que vive doce líneas más abajo *en el mismo archivo*. Con 10 la curva salía
  aplanada y no coincidía con la que `splines()` daba sobre los mismos puntos.
- **Guardas de puntos repetidos** en `get_bezier_tangents` (las que `get_spline_coefficients`
  siempre tuvo). Sin ellas un nodo duplicado daba `-nan` en el EPS **con código de salida 0**.
  El caso llega solo: los paths V1 de `SP`/`GNBZPATH` **duplican los extremos a propósito**
  (era su convención), así que la primera traducción literal los trae.

**`smooth` gana la forma de PRIMITIVA (§9.2), como `sine`.** Antes solo era expresión de path,
así que dibujar exigía `bezier(smooth { … })` — que filtraba al documento el detalle de que
`smooth` produce puntos de control. `smooth { nodos }`, `smooth(attrs) { … }` y `smooth(&p)`;
la forma de expresión se queda (la necesitan `concat`/`fit`). Trabajo de parser, **cero motor
y cero backends**; salida byte-idéntica al envoltorio. 💡 La diferencia entre las hermanas es
**de quién calcula las tangentes**: en `bezier` el bloque son puntos de CONTROL y el autor las
pone (puede hacer picos, tiradores asimétricos — es estrictamente más expresiva); en `smooth`
son NODOS y el compilador las deriva, garantizando paso por todos y empalme liso.

**`bezier` valida el conteo 3k+1** (`checkBezierControlCount`, parse-time, por subtrayecto).
Un conteo que no cerrara se **descartaba en silencio** (5 o 6 puntos dibujaban lo mismo que 4)
— el mismo hueco que motivó `checkCoordPairs`. 🐞 **Cazó un bug en trabajo ya bendecido:**
`fig1.mg` tenía 32 puntos de control y su último punto (`12.0 0`) nunca se dibujaba. Se quitó;
el golden no se movió un byte, lo que **prueba** que era dato muerto.

**`examples/turning_points.mg`** — contraparte paramétrica de la Fig. 4.5 de *Quantum
Mechanics* (Cambridge, 2025) p. 80. Corpus 17→18, **`ok=54`**. Único que ejercita `smooth`
(cierra el hueco de cobertura de PENDIENTES), 2º con `exp` tras `franck_condon` y 2º
enteramente paramétrico. Se dan asíntotas, mínimo, los tres retornos, las tres energías y el
nº de nodos del estado ligado; salen V(x), longitudes de onda, amplitudes y colas.
- **V(x) en forma cerrada** `V∞−(V∞−Vm)·exp(−|(x−xm)/w|^s)`, con anchos y exponentes
  **despejados de los retornos**, no ajustados: el lado izquierdo tiene dos condiciones
  (V(x₀)=E_b, V(x₁)=E_c) que fijan sus dos parámetros → la curva pasa por los retornos
  rotulados *por construcción*. Reproduce los 14 nodos que se dibujaban a mano con **0.9 pt**.
- **La constante de fase sale de Bohr-Sommerfeld** de ψ_c (cuadratura con un `for`), así que
  las otras dos ondas **no tienen libertad**: misma partícula ⇒ misma constante. Verificado
  que acopla: `nodos` 3→4 lleva ψ_c 3→4 lóbulos y arrastra ψ_a 23→30, ψ_b 13→16.
- 🔎 **Por eso NO es un port fiel:** medido sobre la figura publicada, sus tres ondas **no
  pueden ser la misma partícula**. La fase por lóbulo varía hasta 3× dentro de una onda (cada
  región se dibujó repitiendo un ciclo a escala elegida a ojo) y ninguna constante reproduce
  las tres densidades a la vez: con ψ_c de 4 antinodos la física exige ψ_a≈24 y ψ_b≈13
  lóbulos, contra 13 y 8 dibujados.
- **Colas por la escala de Airy** `d ∝ |V'(retorno)|^(−⅓)` (V' por diferencia centrada). ⚠️ NO
  es 1/√(V−E), que **diverge** justo en el retorno donde WKB se rompe — misma lección que
  `franck_condon`. Solo `path +=` puede graficar una función cuyo nº de puntos es variable:
  un bloque de coordenadas no lleva `for`; las piezas se acumulan RELATIVAS y concat encadena.
- El port fiel intermedio (`fig4-5.mg`) se **borró**: al derivar todo de la física dejó de
  reproducir la figura, y su información (cómo re-medir el PDF vectorial de Cambridge con
  `mutool draw -F trace`, el sistema de coordenadas, los 14 nodos) está transcrita en el
  encabezado del paramétrico.
- 🔎 **El PDF de Cambridge es oráculo VECTORIAL** (`mutool draw -F trace`), mejor que el EPS
  de 1998: público, permanente, verificable por cualquier lector. Y resultó **el mismo
  dibujo**, no uno redibujado: mismos conteos de segmentos (ψ_a=13, ψ_b=9, ψ_c=5) y mismos
  extremos dentro de **0.002 unidades**. Su sistema de coordenadas se deduce de las rectas
  (x=0→245.89 pt, x=1→308.25; y=5.2→77.99, y=4.9→85.67) → anisotropía **2.4359**, que es
  exactamente el aspecto de la ventana (9.5/3.9).
- **Encuadre:** la figura publicada estira los ejes por separado y V3 es isométrico, así que
  letterboxearía. Se reproduce con `scale` anisótropo sobre una ventana ya ensanchada en x.
  **Es seguro para los rótulos:** bajo un transform V3 mueve el **ancla** del texto, no los
  glifos (verificado: el `scalefont` sale idéntico) — dato para la pregunta abierta de §19.
- ⚠️ **Trampa al construir ondas con `path +=`:** la semilla **no puede llevar variables**.
  `path w = <expr>` es **diferido** (guarda el árbol, se evalúa al dibujar), así que un
  `sine(phase=ph, amplitude=amp)` como semilla lee los valores que el lazo ya pisó. Un literal
  (`path w = { 0 0 }`) es seguro, y `+=` sí evalúa YA. `franck_condon.mg` lo esquiva sin
  decirlo porque siembra con constantes.
- **Semántica V1 recuperada midiendo** (antes de borrar el material): `PWST x0 y0 x1 y1` es
  `fit` a un rectángulo (`StructureRectangle`), y con las coordenadas **invertidas espeja** la
  struct; `MKST` solo selecciona. `SEN1`/`COS1`/`SEN45UP` se reconstruyeron de la geometría del
  render (`seno.mg` nunca estuvo en el árbol): SEN1 = medio arco en 1 segmento bezier,
  COS1 = ciclo completo en 2 → 5·1 + 4·2 = **13**, los que mide el oráculo.

**Diagnósticos del parser — tres arreglos, dos de ellos silencios.** Los mensajes decían qué
se ESPERABA y nunca qué se ENCONTRÓ (el lexer sí lo hacía: «carácter inesperado '@'»).
- `describeTok()` + `parseError` → «se esperaba 'to' en el for, **pero se encontró 'too'**».
- 🐞 Eso no bastaba para el caso que duele —`polylnie { … }`—, porque la palabra ya se
  consumió y el error señalaba al `{`. Raíz: un identificador desconocido cae al *catch-all*
  de sentencia de estado y solo revienta al parsear su supuesto argumento. Como **ninguna
  sentencia de estado toma un bloque**, un `{` ahí prueba que el identificador no era un
  comando → «'polylnie' no es un comando conocido (¿primitiva mal escrita?)», señalando a la
  posición del NOMBRE. Va en la rama de aridad, no antes: `outlinefill { … }` (0 args +
  bloque) sí es legítimo.
- 🐞 **`colour 0.5` compilaba, no hacía nada y no avisaba.** `emitStyleAttr` YA devolvía
  `bool` diciendo si reconoció el nombre y `StateStmt::exec` **descartaba el retorno**. Ahora
  es `evalError`. Verificadas una por una las diez sentencias reales (color, fill, line_width,
  font_size, font, align, valign, dash, hatch, outlinefill).
- Queda flojo `circl(2) { … }`: se parsea como invocación de struct válida y falla en el
  bloque. No se tocó porque invocación+bloque es legítimo en otros contextos.

💡 **Patrón de la sesión (tres hallazgos, una misma lente):** *el compilador descarta algo en
silencio y produce una figura plausible*. Los tres se cazaron preguntando «¿qué se está
tirando sin avisar?» — coordenadas sobrantes de `bezier`, el punto muerto de `fig1`, el
retorno de `emitStyleAttr`. Vale repetir la búsqueda: **valores de retorno ignorados y ramas
que hacen `return` sin mensaje** son el filón.

⚠️ **El golden en disco se queda RANCIO entre sesiones** (no está en git): traía
`fig6-4v3-clean`, nombre retirado el 2026-07-15, y le faltaba `franck_condon`. `check` daba
`fail=6` **antes** de tocar nada. Un `check` con fallos puede ser vejez, no regresión — se
distingue con `git stash` + recompilar. Y para verificar un cambio sin depender de él:
comparar las 54 salidas contra las del binario anterior, ignorando la línea `%%Title` (lleva
la ruta de salida).

### Cerrado en la sesión del 2026-07-21 (`rule` §13.8 + leyenda automática + puente pandas→MG)

**`rule` (§13.8) IMPLEMENTADO**, con la figura que llevaba un año esperándolo: `figure_02` del
artículo de detección de ceniza (GOES-16/ABI), cinco paneles de histogramas con umbrales. Era
el **punto 2 de las condiciones para el 1.0**; queda solo `table` (§13.10, sin diseñar).
Trabajo de **parser puro** (`RuleStmt`, entre `LegendStmt` y `PlotStmt`): cero elementos
gráficos nuevos, cero cambios en los tres backends. La geometría llega ya mapeada desde
`PlotStmt` → **log gratis**, el mismo reuso que le salió a `base=`.
- **Leyenda AUTOMÁTICA desbloqueada** (§13.9 fuente 1): los `rule` con `label_at="legend"` se
  autocoleccionan y su muestra **es su propia línea**, así que no puede divergir de lo que
  señala. `legend(at=…)` **sin bloque** ahora es válido = "aquí va, las entradas las ponen los
  rule". Anclajes `center-left`/`center-right` añadidos (3 de los 5 paneles los usan).
- Diferidos a propósito: `marker=`, el **borrado de colisiones** malla↔notable (el premio que
  §13.8 promete; `figure_02` no lo ejercita porque manda sus nombres a la leyenda) y los dos
  rótulos a la vez.
- ⚠️ **La figura es LOCAL** (`local/`, con entrada propia en `.gitignore`): datos de artículo
  sin publicar. **No** entra al corpus golden. Es excepción por confidencialidad, distinta de
  la política de "lo no trackeado se borra", que es sobre residuo de V1.

**🐞 Bug del motor destapado, de la familia `FN_NOFACE`, y ROMPÍA LA PARIDAD DE BACKENDS.**
"Heredar la cara ambiente" estaba implementado en `Text::draw` como **no tocar el
dispositivo**, lo cual solo equivale a "la ambiente" mientras nadie la haya cambiado — y dentro
de una línea sí la cambian. Dos síntomas, **una raíz**:
- la prosa tras un `$…$` salía **en la fuente del math**;
- la prosa tras un **sub/superíndice** salía al **tamaño del índice** — porque
  `setRelFontSize` *invalida* la cara para forzar el refresco, así que saltarse `setFontFace`
  **también se salta el refresco de TAMAÑO**. Perseguirlos por separado habría sido perder el
  tiempo dos veces.
- **Fix:** `TextLine::draw` guarda la ambiente antes de dibujar (`Display::setInheritedFace`) y
  `Text::draw` resuelve contra ella; sin cara fijada en el documento, cae a la default — el
  criterio que ya usaban los guards de `EPSDisplay::text`/`PDFDisplay::text`.
- ⚠️ **Lo instructivo:** solo el **PDF** heredaba mal (EPS re-emite `setfont` desde `dspstate`).
  `franck_condon.pdf` traía sus rótulos `"distancia internuclear  $r$"` y `"energía  $E$"` en
  **Times-Italic**, y en EPS/SVG en Times-Roman — divergencia vieja que el golden por bytes
  había **bendecido** y que la **Capa 3 no ve**: sus invariantes cuentan operaciones de texto,
  no fuentes. Único golden re-bendecido, verificado a ojo y contra el EPS/SVG. **Se encontró
  portando una figura nueva, no auditando** — el argumento a favor de seguir portando material
  real.

**El puente pandas→MG (`tools/hist2mg.py`), commiteable y genérico.** MG grafica lo que le
escriben (`polybar` recibe bins, no observaciones) y ese corte es deliberado, pero entre un CSV
de 54 205 filas y un `.mg` hay una reducción que alguien tiene que hacer. El script la hace y
solo la hace: pandas → `np.histogram(bins=N)` (idéntico a `sns.histplot`) + estadísticas → un
`.mg` **incluible** (§15). De 54 mil filas salen 30 barras; los datos crudos no entran a MG.
- **Verificador que salió gratis:** las estadísticas de los 5 paneles cuadran **al último
  decimal** con las tablas impresas en el PDF publicado (`-0.876 / 1.759 / -12.080 / 7.111`…).
  Se validó el camino de datos **antes** de dibujar una sola barra.
- Dos restricciones arbitrarias del parser cayeron de paso, ambas genuinas: `polyline(&p)`
  **no admitía atributos** tras la expresión de path (obligaba a envolver en bloque solo para
  dar estilo), y el álgebra §9 **solo se podía dibujar** con polyline/polygon/bezier — cuando
  un path es una secuencia de puntos y `polybar`/`circle`/`rectangle` también consumen puntos.
- 💡 **Reflejo que valió la pena:** al ver que `color="orange"` salía rojo, comparar la tabla
  ENTERA contra CSS en vez de arreglar la entrada. Salieron **dos** divergencias, no una
  (`orange` y `green`), documentadas en el parser como valores históricos de V1 — así que ese
  día se respetaron y la figura usó hex explícito. **Revertido el 2026-07-21** (ver la sesión
  siguiente): que estuviera documentado explicaba el origen, no lo justificaba.

⚠️ **Lección 7 otra vez** (bloques de coordenadas): `{ 12 ytop0-11 }` son **tres** términos, no
dos. Lo cazó `checkCoordPairs` con línea y columna — justo para lo que se construyó.

### Cerrado en la sesión del 2026-07-21 (colores CSS sin excepciones)

**`orange` y `green` pasan a sus valores CSS** (`0xffa500` y `0x008000`). Eran los dos
últimos nombres que conservaban el valor histórico de V1 y **mentían**: `orange` era un
rojo ladrillo y `green` era el verde puro, que en CSS se llama `lime`. Estaba documentado
en el parser — pero *documentar una trampa no la desactiva*: el día anterior el propio
`figure_02` tuvo que rodearla con hex explícito, que es la señal de que el default está mal.
- **El argumento es la 4ª condición para el 1.0** (§22.7): que un usuario nuevo encuentre lo
  que espera. Misma clase que el renombre de §13 —un nombre que significa otra cosa que en
  el resto del mundo— y **mientras la beta dure cuesta un `sed`**.
- **No se pierde nada:** el verde puro sigue disponible como `lime`, su nombre CSS correcto.
  La tabla es ahora CSS **sin excepciones** (verificado con las 148 entradas), que es una
  regla menos que recordar: antes había que saberse cuáles dos no lo eran.
- Churn: 7 ejemplos (goldens re-bendecidos tras revisar los renders). En `fill_styles` —que
  es el **catálogo de colores**— `orange` y `red` eran dos rojos casi idénticos lado a lado,
  en el documento cuyo trabajo es mostrar qué significa cada nombre.

🐞 **Y destapó un bug de FIDELIDAD del traductor.** `mg1to2.py` pasaba los nombres de color
tal cual, así que un `LCOLOR green` de V1 (verde puro) habría salido verde oscuro en V3: un
cambio de color **en silencio**, justo lo que un traductor no puede hacer. Fix:
`V1_COLOR_RENAME` (`green`→`lime`, `orange`→`#cc3232`, el ladrillo que no tiene nombre CSS).
- **Verificado sin re-bendecir:** los 14 goldens del traductor se capturaron con la tabla
  VIEJA y siguen pasando byte a byte — que es la prueba de que el remapeo restituye la
  apariencia V1 exactamente.
- 💡 **El reflejo otra vez:** comparar las **dos tablas enteras** (V1 tiene 11 colores) en vez
  de parchear los dos que ya conocía. Confirmó que son los únicos que cambian de significado
  y que ninguno de V1 desapareció. `src/Parser.cpp` (front-end V1, fuera del build) conserva
  sus valores a propósito: documenta lo que V1 significaba.

**Aparte, dos ajustes de Alejandro:** `dot(2.5)`→`dot(2)` en `fig4-4` (se veían gordos) y las
ondas de `franck_condon` en color. ⚠️ Al revisarlo salió que **`dot(n)` es el RADIO en pt**
(la spec §4.6 lo dice, el código lo hace, ningún commit lo cambió — `9ba0d63` fue solo el
renombre `dot(marker=)`→`marker(shape=)`). O sea `dot(2.5)` son **5 pt de diámetro**. No hay
desfase sistemático en el corpus; estaban puestos generosos. Mismo aviso para
`marker(size=8)` de `markers-demo`: 16 pt de ancho.

### Cerrado en la sesión del 2026-07-21 (`table` §13.10 — el punto 2 completo)

**`table` implementado**, con los cinco recuadros Mean/SD/Min/Max de `figure_02`. Era lo
último que bloqueaba la **condición 2** de las cinco para el 1.0, que queda **completa**
(`rule`, `legend` automática y `table`, los tres trabajo de **parser puro** — cero motor,
cero backends).

Dos decisiones de diseño que valen más que el código:
- **NO es hija exclusiva de `plot`, a diferencia de `rule`.** `rule` no puede existir fuera
  porque su significado *es* un valor en unidades de datos; una tabla no depende del mapeo,
  ni del rango, ni de los ejes — solo necesita **un rectángulo**. Restringirla habría sido
  copiar la forma de `legend` sin su justificación. `at=` queda sobrecargado (como `hatch` o
  `grid=`): esquina nombrada = ancla a la caja del plot; `(x,y)` = coords de mundo. Son dos
  ramas que resuelven el mismo rectángulo; el resto del código es común.
- **Sí lleva marco, que `legend` no pudo tener.** La leyenda se quedó sin él porque el ancho
  de una entrada es incognoscible en parse-time (su muestra es un bloque arbitrario). Una
  tabla **declara** `col_widths=` en pt, así que su caja mide `sum(anchos) × filas·alto` y
  bordes/fondos/centrado son aritmética conocida. 💡 **Lo que parecía la restricción —tener
  que dar los anchos— es justo lo que compra el marco.**
- `row(...)` y no `labels=[…], values=[…]`: dos listas paralelas se desincronizan en cuanto
  la tabla crece (es la razón de que la invocación de structs use un solo `Arg`, §8.x).
- Números: se reusa `decimals=` de `axis`; `str(x,n)` sigue disponible por celda. Sin
  `format=` propio. Y **la tabla no calcula nada** — la pregunta de alcance que §13.10 dejaba
  abierta ya estaba contestada por la frontera de `tools/hist2mg.py`: MG recibe los números.

🐞 **Fuga de cara, tercera variante de la familia `FN_NOFACE`.** `label_font="bold"` se
fugaba a las celdas de valor de la misma fila: `parse_text` hornea la cara en el `Text` y
`Text::draw` la deja puesta en el dispositivo, así que la celda siguiente —con `FN_NOFACE` =
heredar la ambiente— heredaba la negrita. Se acota con `GS_PUSHSTATE`/`GS_POPSTATE` alrededor
de la celda de rótulo (`popDrawState` restaura el `dspstate` entero, cara incluida).

🐞 **Doble redondeo en el puente de datos, cazado comparando con la fuente.** El panel d
mostraba `0.010` en la figura publicada y `0.009` en la nuestra. La media real es
`0.00951836`; `hist2mg.py` la guardaba con **4** decimales (`0.0095`) y la tabla la redondeaba
otra vez a 3 → `0.009`, cuando redondear UNA sola vez da `0.010`. **Un archivo intermedio no
puede redondear a la precisión de presentación**: su default sube a 6 decimales, que es
precisión de TRANSPORTE. Verificados después los **20 números** de los cinco paneles contra
los impresos en el PDF publicado: coinciden todos.

⚠️ **Trampa de edición por lote (me pasó a mí):** insertar las 5 tablas buscando cada vez la
*primera* `legend` las apila todas en el primer panel — y el render solo delata la última. Al
insertar N bloques hay que anclar cada uno a **su** sección (aquí, el `polybar` del panel).
Y un `re.sub` cuyo patrón empieza en `\n` no encuentra bloques ADYACENTES: el primero se
come el salto que el segundo necesita.

## Code style

[Orthodox C++](https://gist.github.com/bkaradzic/2e39896bc7d8c34e042b): no RTTI, no exceptions; `std::unique_ptr` for ownership, raw pointers non-owning. `-Wall -Wpedantic -Wsuggest-override`, warnings-clean. In headers: fully qualified `std::` (no `using` at namespace scope), `override` on all overrides, include guards `MG_*_H` (never `__*`), in-class member initializers. Project language for comments/messages is Spanish; keep new features in the compiler itself (no external preprocessors).
