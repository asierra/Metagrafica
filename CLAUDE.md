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

bash test/run.sh check    # golden + gs + paridad + docs/img + negativas + galería: ok=66 … galfail=0
bash test/run.sh capture  # re-bless goldens (only after verifying changes are intended)
bash test/run.sh images   # regenera docs/img/*.svg + docs/galeria.html (PUBLICADO; capture NO los toca)
```

**Harness golden ACTIVO (reactivado 2026-07-11; ampliado 2026-07-14/15/17).** Corre el corpus
de `examples/` (22 `.mg` × EPS/SVG/**PDF** = 66 goldens) y compara contra la red golden
(salida del propio renderer V3, regresión — no el oráculo V1). Tras tocar el motor:
`make` y `bash test/run.sh check` (debe dar **ok=66 fail=0 error=0 psfail=0 c3fail=0 imgfail=0 errfail=0 galfail=0**);
re-bendecir con `capture` solo tras verificar que los cambios son intencionales. Golden
files (`test/golden/`) **no están en git** (se regeneran con `capture`).

**Seis compuertas, cada una caza una clase distinta** (razonadas en `plan_plot.md`,
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
  **Cobertura ampliada el 2026-07-23** de 6 ejemplos a **21** (todo el corpus salvo
  `curvas3`, que es biblioteca de datos y compila en blanco), al generar los renders que
  necesitaba la galería: la galería **pagó** cobertura de pruebas.
- **La GALERÍA al día** (`galfail`, nueva 2026-07-23) — `docs/galeria.html` es salida
  publicada (la sirve GitHub Pages) y **derivada**, y lo que la vuelve rancia no es tocar
  el motor sino **editar un comentario**: la página lleva incrustados el encabezado y el
  código fuente completo de cada ejemplo. Ninguna de las otras cinco puede verlo — un
  cambio de comentario no mueve un byte de ningún `.svg` ni de ningún golden. Se compara
  regenerando en memoria (`tools/galeria.py --check`), se omite con aviso si no hay
  `python3`, y se regenera con `images` y no con `capture`, por la misma razón que
  `docs/img`.

Las seis compuertas se verificaron reintroduciendo a propósito los bugs que deben cazar
(la de `docs/img`, con el archivo rancio **real** de `e9198c0`: lo caza, y el golden sigue
dando `ok=57` — que es justo la prueba de que el golden no puede verlo; la de la galería,
cambiando el título de `sines.mg` sin regenerar: `galfail=1` con las otras cinco en cero).

Toolchain: `clang++`/`g++` (C++14, `-fno-rtti -fno-exceptions`), `flex` (regenerates `src/lexmg.cpp` from `src/mgpp.l`), `pandoc` (man page). Do not edit `src/lexmg.cpp` by hand (flex generates it); `include/version.h` **is** edited by hand. libharu is vendored in `third_party/` for PDF.

## Layout

Headers in `include/`, sources in `src/`, binary in `bin/`, regression harness in `test/`. **Herramientas Python en `tools/`** (movidas de la raíz el 2026-07-21): el traductor `mg1to2.py` (§20), el puente de datos `hist2mg.py` (CSV/XLSX → histogramas y estadísticas en `.mg` incluible) y el generador de la galería `galeria.py` (2026-07-23). Son auxiliares **fuera del compilador** — no se ligan a `bin/mg` ni el lenguaje depende de ellas; la política de "sin preprocesadores externos" del Code style se refiere a la compilación de un `.mg`, no a preparar datos antes. `test/run_translator.sh` apunta a `tools/mg1to2.py`. Design/working notes — the `plan_*.md` files plus `audit_text_parser.md` and `notas_at_anchor.md` — live in **`docs/plans/`** (moved out of root 2026-07-17; `docs/` also holds published source PDFs). References throughout the tree cite them **by bare filename** (grep the name, e.g. `plan_plot.md`), not by path. The forward-looking spec (`especificacion_mg.md`) and the pending-work board (`PENDIENTES.md`) stay in root.

**Política V1 (2026-07-15, endurecida 2026-07-20):** todo el trabajo actual es desarrollo de **V3**; **no se trackea material V1 nuevo**. Los `.mg` crudos de V1 y sus traducciones literales se quedaban en el árbol sin commitear; desde el 2026-07-20 **se borran** en cuanto el port V3 está cerrado (así se fueron `fig4-8.mg`, `exp.mg`, `fp3i2dat.mg` y el `fig4-8.eps` de 1998). No estaban en git en **ninguna** rama —`origin/v1-legacy` incluida—, así que el borrado es definitivo: antes de borrar, lo que debe sobrevivir son las **medidas**, transcritas al `.mg` o a un `plan_*.md`, no el archivo (ver el encabezado de `turning_points.mg`, que conserva cómo re-medir el PDF vectorial de Cambridge y los nodos del V(x) manual). Lo ya trackeado en `examples/v1/` (31 archivos: corpus congelado + oráculo) **se queda como está**.

The example corpus is split for the V1→V3 transition (see `examples/v1/README.md`):
- **`examples/v1/`** — frozen V1-syntax corpus (two-letter commands). Serves as translator fixtures + provenance. `examples/v1/reference/*.svg` are the committed **migration oracle**: renders produced while the compiler still parses V1 (SVG chosen for size; SVG/EPS/PDF match). These SVGs are force-included past the `*.svg` gitignore.
- **`examples/`** (raíz) — corpus V3 **compilable** con `bin/mg` (22 `.mg`: curvas3, fig1, fig2-1, fig2-5, fig4-1, fig4-4, fig6-4, fig_polybar, fill_styles, fractal_tree, franck_condon, line_patterns, markers-demo, path_sample, primitives, quickstart, rpstest, sines, symbols, texto, tiro_parabolico, turning_points). El corpus es una **lista explícita** en `test/run.sh`, no un glob: un `.mg` nuevo en la carpeta no entra solo. **`gravitacion_orbita.mg` (2026-07-23) es exactamente ese caso:** el 23º `.mg` de `examples/`, **FUERA del golden a propósito** —no está en `test/run.sh`— hasta que exista `\frac` (sus fórmulas fingen la fracción con `/n`; ver `plan_frac.md`). Es la primera figura del corpus que **compone con una biblioteca** (`include "../lib/satellite.mg"`, §15) y la que motiva `\frac`, `rectangle(w,h,at)` y la búsqueda `include` local→lib. **Nomenclatura (2026-07-20):** los nombres siguen a la **edición de Cambridge 2025** (descargable gratis → la referencia más fácil de verificar por un lector), no a los nombres de archivo de V1 ni a ediciones previas. Por eso el 2026-07-20 `fig4-5`→**`fig4-4`** (Fig. 4.4, p. 78): **va en DOS FASES o colisiona**, y la guardia es que el renombre sea PURO (los goldens de `fig4-4` salieron byte-idénticos a los del antiguo `fig4-5`). **Y al revés:** un ejemplo que deja de reproducir su figura publicada pierde el número y toma nombre de la física (`turning_points`, como `franck_condon`) — el número de figura es una promesa de fidelidad. Se movió aquí desde `examples/v3/` el 2026-07-09; sus salidas **ya no están atadas** al oráculo V1 (dejan de ser traducción 1:1 y pasan a ejercitar/mostrar la gramática V3). Es el corpus de la red golden (`test/run.sh`, reactivada 2026-07-11). **Poda 2026-07-17** (`arrow`, `fig2-3`, `fig4-10`, `fig6-1`, `fig6-10` eliminados: redundantes o `arrow.mg` que renderizaba vacío tras migrar sus flechas a marcadores built-in). `fig6-4` (renombrado desde `fig6-4v3-clean` el 2026-07-15) entró el 2026-07-14: es el único que ejercita eje **log** + `fit(stretch)` + math con superíndices + `extend` + ticks-in, y el único **sin `font` explícito** — por eso es el que caza el bug de cara ambiente en PDF.

**Encabezado de un ejemplo — convención (2026-07-23).** Los 23 `.mg` de `examples/` (22 del
golden + `gravitacion_orbita`) abren
con: **primera línea = título**, párrafo siguiente = **descripción** (2-5 líneas de qué es y
qué enseña del lenguaje), y a partir de `% NOTAS ———` todo lo que le sirve a **quien
mantiene** (procedencia bibliográfica, mediciones, verificadores, avisos de cobertura
exclusiva). `tools/galeria.py` publica el título y la descripción, y **descarta las NOTAS**;
por eso un ejemplo nuevo aparece solo en la galería, sin lista que tocar, pero **un
encabezado mal formado sale publicado**. La limpieza del 2026-07-23 barrió además la
arqueología V1 de los cuerpos (`FPATRN`, `SCST/RTST`, `LWIDTH`…) y **todas** las referencias
`§n` a la especificación: un ejemplo se lee solo. Se conservan las medidas (el verificador
`0.374 µm` de `fig_polybar`, la receta de `mutool` de `turning_points`) y el listado V0 de
1991 en `fractal_tree`, que es la fuente de la que se reconstruyó la figura. En esa pasada
se cazaron cuatro afirmaciones ya falsas —el «AÚN NO COMPILABLE» de `rpstest`, el font
Symbol de `symbols`, los «21 ejemplos» de `primitives` y la anécdota del man en
`quickstart`—: los comentarios envejecen y **nada los vigilaba** hasta `galfail`.

**Galería (2026-07-23):** `docs/galeria.html`, generada por `tools/galeria.py`, publicada en
GitHub Pages (fuente: `main`, raíz) → <https://asierra.github.io/Metagrafica/docs/galeria.html>.
21 tarjetas con figura, título, descripción y el código completo en un desplegable; enlazada
desde ambos README. ⚠️ **Qué entra NO es la misma regla que `imgfail`:** la galería itera
sobre `examples/*.mg` que tengan `docs/img/X.svg`, mientras que la compuerta itera al revés
—sobre `docs/img`— y por eso vigila además las tres variantes que existen solo para los
ensayos (`franck_condon_anarm`, `turning_points_nodos`, `parabola_vs_arco`), que no son
corpus. Son dos reglas parecidas y distintas; no las unifiques.

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

## Roadmap state (act. 2026-07-23)

El parser V3 (`src/parserv3.cpp`) compila los 22 ejemplos del golden de `examples/` a
EPS/SVG/PDF. Grande hecho: expresiones+control de flujo (§5-6), structs+invocación+place/fit/repeat
(§8/§10/§17), generadores §13 (numbers/ticks/axis/grid), primitivas geométricas+bezier+
sine, texto con markup, estado color/fill/line_width/dash/font/align/valign + atributos
por-primitiva (§7.5) con alcance correcto (gsave/grestore en EPS/PDF), transform §11.1.

**Añadido 2026-07-23** (todo con cero churn en el golden): **`hatch_angle`** (orientación de
trama desacoplada del tipo; `crosshatch` enderezable a rejilla recta), **`rectangle(w,h,at)`**
(forma centro+tamaño alterna a las dos esquinas), y **`lib/` instalable** con búsqueda de
`include` **local→lib** (§15; `make install` copia `lib/*.mg`, la ruta se hornea con
`-DMG_LIBDIR`). ⏳ **WIP sin terminar:** el paquete de **tipografía math** (`plan_frac.md`) —
`\frac` (SPIKE hecho, en el árbol como base) + espaciado automático estilo TeX (diagnosticado).
Es lo que falta para que `examples/gravitacion_orbita.mg` entre al golden.

Cerrado en cada sesión: **[`docs/bitacora.md`](docs/bitacora.md)** — el registro de qué se
cambió y por qué, sesión por sesión (24 y subiendo). ⚠️ **Léelo antes de tocar el motor o de
cambiar una decisión de diseño:** muchas entradas traen la medición que la sostiene, y varias
registran el camino que se probó primero y no funcionó. Vivía aquí dentro hasta el
2026-07-22; se mudó porque también le sirve a un colaborador humano.

## Code style

**C++14, y el estilo se describe en vez de etiquetarse** (act. 2026-07-22; antes decía
«[Orthodox C++](https://gist.github.com/bkaradzic/2e39896bc7d8c34e042b)», que sigue siendo la
inspiración pero describía mal lo que el código hace):

- **Sin excepciones ni RTTI** (`-fno-rtti -fno-exceptions`). Esa parte no se ha movido, y es
  la que condiciona el resto: por eso el manejo de errores es `evalError`/`parseError` con
  `exit`, y por eso los guardados de estado son save/restore explícito y no RAII.
- **STL para estructura, con soltura:** `std::unique_ptr` para propiedad (punteros crudos =
  no-propietarios), `std::vector`/`map`/`string` sin reparos. Aquí *sí* se diverge de
  Orthodox C++, que pide usar la STL con moderación; la divergencia es deliberada y no hay
  intención de revertirla.
- **Salida formateada con `printf`, no con iostreams**, en los tres backends — y eso *coincide*
  con Orthodox C++, aunque por razones propias: los backends emiten un **formato de cable**
  (operadores PostScript, atributos SVG), donde la cadena de formato muestra la línea que va a
  salir; y el código entremezcla `%f` (decimales fijos) con `%g` (cifras significativas), que
  iostreams modelan con manipuladores **pegajosos** — un `fixed` sin restaurar cambiaría en
  silencio los números del otro formato. `-Wformat` (dentro de `-Wall`) recupera la seguridad
  de tipos que se le suele reprochar a `printf`. **Los streams se usan solo para ARMAR
  cadenas** (`std::ostringstream` en `SVGDisplay` para atributos y listas de puntos), nunca
  para escribir el archivo: son dos capas, no una mezcla. Revisado en 2026-07-22 al preguntarse
  si convenía convertir: no — reescribiría todos los números de los tres backends (`%f` = 6
  decimales fijos vs `<<` = 6 cifras significativas) y movería los 66 goldens a cambio de nada.
  En C++14 tampoco está `std::format`, que sería la respuesta moderna a las dos opciones.
- **Locale:** `main` fija `setlocale(LC_NUMERIC, "C")`. `printf` respeta `LC_NUMERIC`, así que
  una coma decimal corrompería EPS, SVG y PDF a la vez; el entorno no puede provocarlo (un
  programa C arranca en locale «C») pero una biblioteca que llame a `setlocale` sí. ⚠️ La red
  de pruebas **no** lo cazaría: `test/run.sh` exporta `LC_ALL=C`.
- **Compilación limpia:** `-Wall -Wpedantic -Wsuggest-override`, sin warnings.
- **En headers:** `std::` cualificado (nada de `using` en ámbito de espacio de nombres),
  `override` en todas las sobrescrituras, guardas `MG_*_H` (nunca `__*`), inicializadores de
  miembro en clase.
- Idioma de comentarios y mensajes: **español**. Las características nuevas van **en el
  compilador** (nada de preprocesadores externos).
