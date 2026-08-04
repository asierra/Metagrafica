# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this project is

MetaGráfica is a 2D descriptive vector graphics language. The `mg` binary compiles `.mg` source files into EPS, SVG or PDF (chosen by output extension). Versioning follows the project's publication history (see `include/structures.h` header): V1 (grammar of two-letter commands, 1999–2024) is frozen on the `v1-legacy` branch; `main` is the **V3** development line. `include/version.h` holds the version **by hand** (`MG_VERSION "3.1.0-beta"`, nothing is derived from git) — and it is the **only** place in the code that carries it. ⚠️ **Do not reintroduce a `Version:` line into per-file banners** (removed 2026-08-04): sixteen files used to repeat it and it had already drifted into **four** conventions — twelve headers on the current version, `src/Display.cpp` a release behind, three `src/*.cpp` still on `2024`, `main.cpp` on a prose variant — while the other twelve files, `parserv3.cpp` and `version.h` among them, never had one. A per-file version is decoration shaped like data: nothing reads it, nothing checks it, and a release sweep misses a copy (this one did, on the first pass). The `Antecedents` block and the copyright line stay; they are history, and history does not go stale. Bumping a release is now `include/version.h` plus the prose that quotes it: `README.md`/`README.es.md` (badge and status section), `man/mg.1.md` (footer and the "This is version" line), `CLAUDE.md`, `especificacion_mg.md` §22.7 and `docs/plans/plan_promocion.md` §1. **None of it reaches the output** — `MG_VERSION` is printed only by `mg -v` — so bumping the version moves no golden. The release notes for the tag being cut live in `docs/notas_release.md`, which `.github/workflows/release.yml` publishes above its generic body. V3 is **beta**: the grammar can still change (it did on 2026-07-16: `title`→`label`, `labels`→`tick_labels`) and parts of the spec are unbuilt.

The forward-looking design lives in `especificacion_mg.md`: §3.1 (isometric space), §16 (nested windows), §22 (engine continuity plan), §22.6 (work order). Read §22 before large engine changes.

## Build and test

```bash
make                  # build bin/mg (the V3 compiler) and man page
make clean
./bin/mg examples/primitives.mg          # → primitives.eps
./bin/mg examples/fig2-5.mg out.svg      # backend by extension (.eps/.svg/.pdf)

bash test/run.sh check    # golden + gs + paridad + docs/img + negativas + galería + traducción + bloques de doc: ok=93 … docfail=0
bash test/run.sh capture  # re-bless goldens (only after verifying changes are intended)
bash test/run.sh images   # regenera docs/img/*.svg + la galería es/en (PUBLICADO; capture NO los toca)

bash test/run_translator.sh check    # traductor V1→V3 (tools/mg1to2.py): ok=14
```

**`make install` (2026-07-30)** reparte, además del binario y el man: `lib/*.mg` en
`$(LIBDIR)` = `share/metagrafica/lib` (la ruta horneada en `-DMG_LIBDIR`), los 31 ejemplos en
`share/metagrafica/examples` —**hermanos de `lib/` a propósito**: así el `include "../lib/x.mg"`
de un ejemplo resuelve igual instalado que en el árbol, sin editar un `.mg`— y la doc legible
(referencia es/en, galería es/en + `docs/img/*.svg`, el ensayo) en `share/doc/metagrafica`. La
bitácora y `plans/` **NO se instalan** (mantenedor). `uninstall` lo refleja. Ver `docs/bitacora.md`
2026-07-30 por qué los ejemplos van bajo `share/metagrafica` y no `share/doc`.

**Harness golden ACTIVO (reactivado 2026-07-11; ampliado 2026-07-14/15/17).** Corre el corpus
de `examples/` (31 `.mg` × EPS/SVG/**PDF** = 93 goldens) y compara contra la red golden
(salida del propio renderer V3, regresión — no el oráculo V1). Tras tocar el motor:
`make` y `bash test/run.sh check` (debe dar **ok=93 fail=0 error=0 psfail=0 c3fail=0 imgfail=0 errfail=0 galfail=0 trfail=0 docfail=0**);
re-bendecir con `capture` solo tras verificar que los cambios son intencionales. Golden
files (`test/golden/`) **no están en git** (se regeneran con `capture`).

⚠️ **`test/run_translator.sh` NO lo corre `test/run.sh`, y por eso se pudre.** Su red golden
es «traductor + `bin/mg`», así que la mueve **cualquier** cambio del motor, igual que la del
corpus — pero nadie la corre, y el 2026-07-27 apareció con seis días de atraso (`ok=9 fail=5`:
la bandera *large-arc* de los arcos de 180° tras la reconstrucción de arcos, y un `<text>` con
un espacio suelto que dejó de emitirse con `plan_text_space`). Ninguno era un fallo del
traductor. **Tras tocar el motor, corre también `bash test/run_translator.sh check` y
re-captúralo** si el cambio era intencional.

**Ocho compuertas, cada una caza una clase distinta** (razonadas en `plan_plot.md`,
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
  **preexistentes** que el golden bendice porque un backend omite algo en silencio. Tres
  invariantes robustos (cero falsos positivos en el corpus; el PDF de libharu no está
  comprimido → sus operadores son parseables directo): **(a)** nº de
  operaciones de texto `EPS(show) == SVG(<text>) == PDF(Tj)` (caza rótulos en blanco); **(b)**
  ningún path SVG de un solo segmento (`M..L..`) puede ir `fill=color stroke=none` = línea
  de área nula invisible (caza ejes sin trazo por fuga de fill, Lección 6); **(c)** la
  **geometría** de arcos y elipses coincide en los tres (`tools/arcparity.py`, 2026-07-27:
  muestrea cada arco del EPS y exige que SVG y PDF contengan esa curva, más el conteo de
  comandos `A`; se omite con aviso si no hay `python3`); **(d)** nº de **rellenos degradados**
  `EPS(shfill) == SVG(url(#mggrad…)) == PDF(/ShN sh)` (nueva 2026-07-28, §4.14: un degradado es
  justo lo que un backend puede omitir en silencio — si SVG lo pinta y PDF sale plano, cada
  salida es byte-estable y el golden bendice las dos; verificada enmudeciendo el SVG, que da
  `c3fail=1` mientras `capture` bendice sin protestar). Corre también en
  `capture`. Es la única capa que caza un bug preexistente; las otras dos lo bendecen.

  ⚠️ **(c) es la única invariante SIN ESCAPATORIA POR BENDICIÓN**, y por eso existe: las demás
  compuertas comparan contra un golden, pero el flujo normal tras tocar el motor es
  **re-bendecir**, así que un cambio equivocado se bendice solo. Eso pasó entre el 2026-07-26 y
  el 2026-07-27 (elipses y arcos girados mal en EPS y SVG, `ok=69` todo el tiempo). (c) compara
  backend contra backend: no hay nada que bendecir. **Y entran los TRES, no dos:** durante todo
  ese bug EPS y SVG coincidían *entre sí* y ambos estaban mal — el PDF es la tercera opinión
  independiente porque no decide ejes ni ángulos, transforma puntos de control de Bézier.
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
- **Pruebas NEGATIVAS** (`errfail`) — las demás miran salida EXITOSA, así que los ~150 caminos
  de error del compilador no tenían una sola prueba, y su regresión natural es la peor: volver
  al SILENCIO. Cada `test/errors/*.mg` declara **en sí mismo** lo que espera (van en git, no
  hay dos listas que desincronizar) y se exige exit **1 exacto** —un segfault también «falla»—,
  el fragmento de mensaje, y que **no** haya archivo de salida. **Ampliada el 2026-07-28 a los
  diagnósticos NO fatales** con dos marcadores más: `% EXPECT_WARN:` (compila, deja salida,
  avisa) y `% EXPECT_NO_WARN:` (caso legítimo que NO debe disparar el aviso). ⚠️ **Un aviso es
  lo MÁS expuesto a esta clase de regresión, no lo menos:** si deja de darse, la salida sigue
  byte-idéntica y las otras seis compuertas siguen verdes — no se pierde nada salvo la única
  pista que tenía el usuario. `EXPECT_NO_WARN` cubre la otra forma de matar un aviso: llenarlo
  de falsos positivos hasta que se ignore. ⚠️ **`EXPECT_NO_WARN` es el único que se compila a los
  TRES backends** (ampliación 2026-08-01; los fatales abortan antes de que el backend importe).
  Su afirmación es «esto es legítimo y compila limpio», y eso tiene que valer en los tres: un
  backend que ABORTA donde los otros dos toleran no lo alcanza ninguna otra compuerta —sin
  archivo no hay golden que comparar ni tres salidas que confrontar en la Capa 3—. Es lo que
  pasaba con el arco de barrido cero, que tumbaba el PDF entero.
- **La GALERÍA al día** (`galfail`, nueva 2026-07-23) — `docs/galeria.html` es salida
  publicada (la sirve GitHub Pages) y **derivada**, y lo que la vuelve rancia no es tocar
  el motor sino **editar un comentario**: la página lleva incrustados el encabezado y el
  código fuente completo de cada ejemplo. Ninguna de las otras cinco puede verlo — un
  cambio de comentario no mueve un byte de ningún `.svg` ni de ningún golden. Se compara
  regenerando en memoria (`tools/galeria.py --check`), se omite con aviso si no hay
  `python3`, y se regenera con `images` y no con `capture`, por la misma razón que
  `docs/img`.

- **La REFERENCIA EN INGLÉS al día** (`trfail`, nueva 2026-07-28) — `docs/reference.md` es la
  traducción de `docs/referencia.md` y, a diferencia de `docs/img` o la galería, **no se puede
  regenerar**: traducir es trabajo humano. Así que no se compara el contenido sino la
  **procedencia**: el archivo inglés lleva grabado, en un comentario al final, el
  `git hash-object` del español del que salió, y la compuerta comprueba que siga siendo el
  vigente. No dice si la traducción es buena; dice si es **vieja**. Nació de encontrarla con 88
  líneas de atraso en 5 commits, y lo que le faltaba era lo más nuevo —`marker_at`, arcos
  elípticos, la regla de la ruta log—, o sea justo lo que querría leer alguien de fuera. Se
  re-sella **a mano** a propósito: así sellar significa «ya traduje» y no es el efecto colateral
  de otro comando.

- **Los BLOQUES DE CÓDIGO de la documentación** (`docfail`, nueva 2026-07-29,
  `tools/docblocks.py`) — las otras siete vigilan la **salida** del compilador; ninguna mira lo
  que la documentación **afirma**, y una afirmación falsa es peor que un bug: es un bug que el
  lector copia con confianza. Compila los bloques ```octave de `docs/referencia.md` y
  `docs/reference.md`. Nació preguntándose qué contexto necesita un **agente externo** para
  escribir una figura, y en su primera corrida encontró un ⚠️ de §10 que enseñaba
  `smooth(&nodos)` como **la** forma de partir de un trayecto que ya tienes —y no compila: los
  generadores exigen bloque literal—; llevaba ahí, en los dos idiomas, sin que nada lo viera.
  ⚠️ **Un humano tropieza y desconfía del documento; un modelo de lenguaje obedece**, así que
  para él la referencia es la única fuente de verdad y un error ahí se vuelve código roto con
  seguridad. Cada bloque declara **en el propio `.md`** lo que espera (`<!-- mg-noexec: … -->` =
  notación, no código; `<!-- mg-expect-error -->` = contraejemplo ❌ deliberado, que **debe**
  fallar y que también rompe la compuerta si algún día compila) — no hay lista aparte que
  desincronizar. Los bloques que solo usan una `struct`, un `path` o una variable que el texto
  definió antes se cuentan como **fragmentos** y no fallan: sus errores son de EVALUACIÓN, o sea
  que el parseo —lo único que esta compuerta juzga— ya pasó. Se omite con aviso si no hay
  `python3`.

Las compuertas se verificaron reintroduciendo a propósito los bugs que deben cazar
(la de `docs/img`, con el archivo rancio **real** de `e9198c0`: lo caza, y el golden sigue
dando `ok=57` — que es justo la prueba de que el golden no puede verlo; la de la galería,
cambiando el título de `sines.mg` sin regenerar: `galfail=1` con las otras cinco en cero).

Toolchain: `clang++`/`g++` (C++14, `-fno-rtti -fno-exceptions`), `flex` (regenerates `src/lexv3.cpp` from `src/lexer.l`), `pandoc` (man page). Do not edit `src/lexv3.cpp` by hand (flex generates it); `include/version.h` **is** edited by hand. libharu (2.4.6) is vendored in `third_party/` for PDF.

⚠️ **La copia vendorizada llegó INCOMPLETA y se completó el 2026-07-28.** Le faltaba
`src/hpdf_shading.c` —el que define `HPDF_Shading_New`/`HPDF_Shading_AddVertexRGB`— aunque
`hpdf.h` las declara y la mitad consumidora (`HPDF_Page_SetShading`, `HPDF_Page_GetShadingName`,
el dict `/Shading` de `hpdf_pages.c`) sí estaba compilada: el árbol era incoherente consigo
mismo y solo se notó al necesitar degradados. Se restauró el archivo de upstream v2.4.6 tal
cual (misma licencia ZLIB, sin editar una línea; el `wildcard` del Makefile lo toma solo). **No
es un parche a libharu, y la política de no parchearla sigue en pie:** si algo más falta, se
restaura de upstream y se anota aquí. Lo que libharu de verdad NO implementa es el sombreado
axial (tipo 2) ni el radial (tipo 3) — solo la malla de triángulos (tipo 4), que para un
degradado lineal basta y para uno radial no (ver `plan_gradientes.md`).

**Windows (2026-07-27):** `make CROSS=x86_64-w64-mingw32` → `bin/mg.exe`, enlazado estático,
cruzado desde Linux con MinGW-w64 (necesita `mingw-w64` + `libz-mingw-w64-dev`). El build
nativo no cambia. Lo único exclusivo de Windows en el código es `exeDir()` en `parserv3.cpp`
(bajo `#ifdef _WIN32`): añade `<dir del .exe>/lib/` a la búsqueda de `include`, porque allí no
hay `make install` ni ruta que hornear en `-DMG_LIBDIR` — el reparto es un `.zip`.
`.github/workflows/release.yml` compila los tres sistemas al empujar una etiqueta `v*`, y un
trabajo aparte **ejecuta el `.exe` en Windows** (SVG/PDF/EPS + `include` sin ruta): el binario
cruzado no se corre en la máquina que lo produce, y publicar lo que nadie ejecutó sería el
mismo hueco que las compuertas existen para cerrar. En Linux `test/run.sh capture` bloquea el
release; en macOS es informativo (libm puede mover el último dígito).

⚠️ **La salida es IDÉNTICA BYTE A BYTE en Linux y Windows** (72/72, medido el 2026-07-27 con
el `.exe` bajo wine), y el workflow lo **vigila** en cada release comparando las dos salidas
del mismo commit. Costó una línea de motor: `PDFDisplay::deviceRotate` redondea a cero el
coseno de un giro recto —`cos(pi/2)` da 6.123e-17, y quien lo imprimía era `HPDF_FToA` de
libharu, que hereda la libm de la plataforma—. Redondear ACERCA al valor exacto (`0 1 -1 0`),
no se aleja. Es una divergencia que se reintroduce sola con cualquier `printf` que dependa de
la plataforma, y **no la caza ningún golden**, porque el golden se genera en una sola.
(El 72/72 es la medición de esa fecha, con 24 ejemplos; hoy el workflow compara **93**.)

⚠️ **La cabecera DSC del EPS: `%%Title` lleva el `.mg` de ORIGEN, `%%Creator` la versión**
(2026-08-04). **No devuelvas `%%Title` a la ruta de salida**, que es lo que llevaba antes: se
midió que **Ghostscript propaga `%%Title` a los metadatos `/Title` del PDF**, o sea que la ruta
absoluta del disco de quien compilaba acababa dentro de la figura publicada —y este proyecto
existe para producir figuras de libro—. Además hacía la salida dependiente del directorio, y por
eso había **dos rodeos**: `normalize()` en `test/run.sh` reescribía la línea para poder comparar
goldens (retirado: mientras estuvo, era la única línea que el golden no podía ver) y
`release.yml` exigía rutas relativas idénticas entre plataformas. Con el nombre del fuente, el
mismo `.mg` da el mismo EPS se escriba donde se escriba y se llame como se llame —verificado
compilando a dos rutas y nombres distintos y comparando con `cmp`—. En la misma pasada se borró
`ps_creator`, una variable global muerta que declaraba `MetaGrafica v4.0 2023`, una versión que
nunca existió. ⚠️ **Consecuencia asumida de `%%Creator`:** al llevar `MG_VERSION`, **cada subida
de versión mueve los 31 goldens EPS** (SVG y PDF no lo emiten y no se mueven); es el precio de
que una figura publicada diga con qué se hizo.

## Layout

Headers in `include/`, sources in `src/`, binary in `bin/`, regression harness in `test/`. **Herramientas Python en `tools/`** (movidas de la raíz el 2026-07-21): el traductor `mg1to2.py` (§20), el puente de datos `hist2mg.py` (CSV/XLSX → histogramas y estadísticas en `.mg` incluible), el generador de la galería `galeria.py` (2026-07-23), el puente geográfico `geo2mg.py` (2026-07-24: Natural Earth → `struct` de mapa icónico, proyección ortográfica/full-disk, line-art o relleno; generó `lib/polar_map.mg` y `lib/fulldisk_map.mg`) y el verificador de paridad geométrica `arcparity.py` (2026-07-27: invariante (c) de la Capa 3, lo invoca `test/run.sh`; solo biblioteca estándar) y el compilador de bloques de documentación `docblocks.py` (2026-07-29: la compuerta `docfail`, lo invoca `test/run.sh`; solo biblioteca estándar). Son auxiliares **fuera del compilador** — no se ligan a `bin/mg` ni el lenguaje depende de ellas; la política de "sin preprocesadores externos" del Code style se refiere a la compilación de un `.mg`, no a preparar datos antes. ⚠️ **`geo2mg.py` es OPCIONAL y el más pesado**: requiere `geopandas`/`pyproj`/`shapely` (los otros usan solo `pandas`) y datos Natural Earth 1:110m que **NO van en el repo** (se bajan de naturalearthdata.com; el header del tool y de cada `.mg` generado dicen cómo). Los mapas de `lib/` son assets GENERADOS committeados (como `docs/img`), con su comando de regeneración en el encabezado. `test/run_translator.sh` apunta a `tools/mg1to2.py`.

👁️ **`tools/ver.sh` (2026-07-31) — la única que NO es Python y la única que no vigila nada: sirve para MIRAR.** `tools/ver.sh figura.mg` compila y rasteriza los tres formatos; `tools/ver.sh --diff a b` dice cuántos píxeles cambiaron y deja la imagen de diferencia. Existe porque las ocho compuertas cazan clases de fallo y ninguna contesta «¿se ve bien?», que es la pregunta que destapó más defectos que las compuertas en verde (bitácora 2026-07-20 y 2026-07-28). Dos cosas medidas que codifica y conviene no re-litigar: **(1) un SVG de MetaGráfica hay que rasterizarlo con un NAVEGADOR** — `rsvg-convert` e Inkscape ignoran el `@font-face` de LM Math y caen a una sans, así que mienten sobre la tipografía matemática (una ρ correcta se ve como «ø», y es facilísimo diagnosticarlo como bug del compilador); **(2) `--diff` es del MISMO formato a propósito** — entre backends el antialiasing deja 6333 px de ruido contra los 524 de un rótulo desplazado 1.4 pt, o sea que la señal queda debajo del ruido; por eso la paridad entre backends se hace contando operadores (Capa 3) y no mirando píxeles. Dentro de un formato el piso de ruido es **0 px exactos**. ⚠️ **No sustituye al golden, que es más sensible**: contesta la pregunta que el golden no puede —«los bytes cambiaron a propósito, ¿cambió el dibujo?»—, que es justo el criterio de aceptación de la Fase C de `plan_pseudo3d.md`. Design/working notes — the `plan_*.md` files plus `audit_text_parser.md` and `notas_at_anchor.md` — live in **`docs/plans/`** (moved out of root 2026-07-17; `docs/` also holds published source PDFs). References throughout the tree cite them **by bare filename** (grep the name, e.g. `plan_plot.md`), not by path. The forward-looking spec (`especificacion_mg.md`) and the pending-work board (`PENDIENTES.md`) stay in root.

**Política V1 (2026-07-15, endurecida 2026-07-20):** todo el trabajo actual es desarrollo de **V3**; **no se trackea material V1 nuevo**. Los `.mg` crudos de V1 y sus traducciones literales se quedaban en el árbol sin commitear; desde el 2026-07-20 **se borran** en cuanto el port V3 está cerrado (así se fueron `fig4-8.mg`, `exp.mg`, `fp3i2dat.mg` y el `fig4-8.eps` de 1998). No estaban en git en **ninguna** rama —`origin/v1-legacy` incluida—, así que el borrado es definitivo: antes de borrar, lo que debe sobrevivir son las **medidas**, transcritas al `.mg` o a un `plan_*.md`, no el archivo (ver el encabezado de `turning_points.mg`, que conserva cómo re-medir el PDF vectorial de Cambridge y los nodos del V(x) manual). Lo ya trackeado en `examples/v1/` (31 archivos: corpus congelado + oráculo) **se queda como está**.

The example corpus is split for the V1→V3 transition (see `examples/v1/README.md`):
- **`examples/v1/`** — frozen V1-syntax corpus (two-letter commands). Serves as translator fixtures + provenance. `examples/v1/reference/*.svg` are the committed **migration oracle**: renders produced while the compiler still parses V1 (SVG chosen for size; SVG/EPS/PDF match). These SVGs are force-included past the `*.svg` gitignore.
- **`examples/`** (raíz) — corpus V3 **compilable** con `bin/mg` (31 `.mg`: angulo_solido, curvas3, elevacion_solar, espectro, fig1, fig2-1, fig2-5, fig4-1, fig4-4, fig6-4, fig_polybar, fill_styles, fractal_tree, franck_condon, gravitacion_orbita, irradiancia, line_patterns, markers-demo, multietapa, onda_electromagnetica, orbita_polar, path_sample, primitives, quickstart, rpstest, seccion_eficaz, sines, symbols, texto, tiro_parabolico, turning_points). El corpus es una **lista explícita** en `test/run.sh`, no un glob: un `.mg` nuevo en la carpeta no entra solo. **`gravitacion_orbita.mg` ENTRÓ al golden el 2026-07-24** (estuvo fuera a propósito hasta que `\frac` quedó completo): es la única figura que ejercita **`\frac`** (fracción math 2-D, inline y con extent vertical medido), **`include` de una biblioteca** (`include "../lib/satellite.mg"`, §15), **`rectangle(w,h,at)`**, la búsqueda `include` local→lib y el **default de marcador-hereda-color-de-línea** (flechas roja/verde sin `marker_color`). **Nomenclatura (2026-07-20):** los nombres siguen a la **edición de Cambridge 2025** (descargable gratis → la referencia más fácil de verificar por un lector), no a los nombres de archivo de V1 ni a ediciones previas. Por eso el 2026-07-20 `fig4-5`→**`fig4-4`** (Fig. 4.4, p. 78): **va en DOS FASES o colisiona**, y la guardia es que el renombre sea PURO (los goldens de `fig4-4` salieron byte-idénticos a los del antiguo `fig4-5`). **Y al revés:** un ejemplo que deja de reproducir su figura publicada pierde el número y toma nombre de la física (`turning_points`, como `franck_condon`) — el número de figura es una promesa de fidelidad. Se movió aquí desde `examples/v3/` el 2026-07-09; sus salidas **ya no están atadas** al oráculo V1 (dejan de ser traducción 1:1 y pasan a ejercitar/mostrar la gramática V3). Es el corpus de la red golden (`test/run.sh`, reactivada 2026-07-11). **`orbita_polar.mg` ENTRÓ el 2026-07-27**, al cerrarse la oclusión de la mitad trasera de la órbita: es el único que ejercita **`arc(rx, ry)`**, **`marker_at`** (marcadores en ángulos paramétricos) y **`place(..., rx/ry, at=)`** (struct posada sobre un arco elíptico) — sin él esas tres se quedan sin pruebas —, y el primero con arcos elípticos **girados** en el corpus, que es justo lo que vigila la invariante (c) de la Capa 3. **`espectro.mg` ENTRÓ el 2026-07-28** con los degradados (§4.14): es el único con **relleno degradado**, o sea el único sujeto de la invariante (d) de la Capa 3 — si sale del corpus, esa compuerta se queda sin nada que mirar. **`elevacion_solar.mg` ENTRÓ el 2026-07-28**, y por una razón distinta a las demás: **no ejercita ni una característica exclusiva del lenguaje** (se revisó una por una: listas de cadenas indexadas ya están en `fill_styles`/`symbols`, el arco con flecha en los dos extremos y `marker_start_orient="reverse"` en `fig2-5:96`, el texto girado en los rótulos de eje de `fig1`/`quickstart` —mismo `<g transform="rotate()">`, solo cambia el ángulo—). Lo que aporta es **el único usuario de `lib/fulldisk_map.mg`**: era un asset generado, committeado y documentado en §12 de la referencia que ninguna compuerta miraba, y ahora se recompila en cada `check`. Es también la primera figura del corpus hecha para una **audiencia externa** (un curso), no para probar el motor. **`angulo_solido.mg` ENTRÓ el 2026-08-01**, primera figura pseudo-3D del corpus y única que ejercita **`view3d`**, **`plane3d`** y **`xyz()`** (§13 de la referencia): si sale, las tres se quedan sin prueba. Aporta además el caso duro de la invariante (c) de la Capa 3 —sus elipses tienen semidiámetros conjugados **genuinamente oblicuos** (`u·v ≠ 0`), que es donde EPS traza con matriz, SVG resuelve un SVD 2×2 y PDF transforma puntos de control: tres caminos distintos— y es el primer ejemplo con **arcos de barrido variable calculados por visibilidad**. ⚠️ **NO lleva prefijo ni subcarpeta**, y se decidió así el 2026-08-01: los ejemplos se nombran por su **asunto** y no por su técnica (si no, `quickstart` sería `plot_quickstart`), y una subcarpeta rompería cuatro piezas de maquinaria plana —el glob de `make install`, la profundidad `examples/../lib` que ese layout existe para garantizar, el `glob("*.mg")` de `galeria.py` y el `cd "$EXDIR"` de `run.sh`—. La familia se distingue donde un lector la consume: el **grupo editorial** «Escenas pseudo-3D» de la galería. **`onda_electromagnetica.mg` ENTRÓ el 2026-08-01** (Lira fig. II-1), segunda pseudo-3D: es el único con un **GENERADOR** y un **RELLENO dentro de un `plane3d`** —§13 promete que el dibujo 2-D corriente funciona ahí y sin esta figura la promesa no tenía prueba— y el único con **marcadores bajo una matriz no conforme**, o sea la familia de `plan_anisotropia.md`, que aquí sale limpia. Sus dos amplitudes son distintas **por derivación de la cámara**, no a ojo: el plano de B se escorza, y dibujar los dos campos con la misma amplitud diría algo falso. **`irradiancia.mg` ENTRÓ el 2026-08-01** (Lira fig. II-7), tercera pseudo-3D: es el único con una **SILUETA de revolución calculada** —las generatrices del cono son las tangentes desde el ápice al borde del disco, en forma cerrada— y el único que dibuja un **ángulo del espacio en su propio plano** en vez de anotarlo en el papel, que es una **mejora deliberada sobre el original**. También el único usuario de **`acos`**. ⚠️ Trae un **verificador incorporado**: `dΩ` es la sección del cono a una fracción del recorrido, así que tiene que salir TANGENTE a las generatrices por construcción; si la derivación se rompe, deja de estarlo y se ve. **`multietapa.mg` ENTRÓ el 2026-08-03**
(Lillesand, Kiefer & Chipman, fig. 1.25, RECONSTRUIDA porque el original es un escaneo
ilegible), y entra por la misma razón que `elevacion_solar`: **no ejercita ninguna
característica exclusiva del lenguaje** —lo aportan `include` de biblioteca, `polygon`,
`dashdot`, `gray()`, texto multilínea, todos ya cubiertos— sino que es el **único usuario de
`lib/aircraft.mg` y de `lib/people.mg`**, los dos iconos nuevos del 2026-08-02. Una biblioteca
sin ejemplo que la incluya no la compila ninguna compuerta: se pudre en silencio, y el aviso
llega el día que alguien la usa. **`seccion_eficaz.mg` ENTRÓ el 2026-08-03** y cierra esa
misma cuenta: es el **único usuario de `lib/pseudo3d.mg`** —y por tanto de `cono` y
`cilindro`—, la última biblioteca de `lib/` que no compilaba nadie; con ella **las ocho
tienen cliente en el corpus**. Es la cuarta pseudo-3D —**Fig. 20.5 de IMQ 3ª ed.**, «Significado geométrico de la
sección eficaz»; la edición de Cambridge publica la misma con rótulos en inglés— y **no
lleva número de figura a propósito**: se RECONSTRUYÓ, tomando del original la composición
y no las coordenadas, y el número es una promesa de fidelidad que una reconstrucción no
puede hacer. ⚠️ Su lección no es
gráfica sino del **lenguaje**: escribiéndola se encontró que **las asignaciones del cuerpo de
una struct escribían en el ámbito del llamador** (los parámetros no), así que un `.mg` que
incluyera una biblioteca perdía sin aviso cualquier variable cuyo nombre ésta reusara
—`pseudo3d.mg` asigna `qx`, `qy`, `cx`, `ux`…—. **ARREGLADO el mismo día** (2026-08-03): el
cuerpo de una struct es ahora **frontera para las escrituras** (`Scope::barrier` +
`findAssignable`), las lecturas la siguen cruzando, y no movió un solo golden.
🔒 **Y por eso este ejemplo es el GUARDIÁN de esa regresión: sus variables COLISIONAN A
PROPÓSITO con las de `lib/pseudo3d.mg` — no las renombres.** Si arreglar la fuga no movió un
golden, perderla tampoco lo movería: ninguna compuerta puede cazarla, y esta figura es la
única red. Verificado reintroduciendo el bug (falla en los tres backends). **Poda 2026-07-17** (`arrow`, `fig2-3`, `fig4-10`, `fig6-1`, `fig6-10` eliminados: redundantes o `arrow.mg` que renderizaba vacío tras migrar sus flechas a marcadores built-in). `fig6-4` (renombrado desde `fig6-4v3-clean` el 2026-07-15) entró el 2026-07-14: es el único que ejercita eje **log** + `fit(stretch)` + math con superíndices + `extend` + ticks-in, y el único **sin `font` explícito** — por eso es el que caza el bug de cara ambiente en PDF.

**Encabezado de un ejemplo — convención (2026-07-23).** Los 31 `.mg` del golden de `examples/` abren
con: **primera línea = título**, párrafo siguiente = **descripción** (2-5 líneas de qué es y
qué enseña del lenguaje), y a partir de `% NOTAS ———` todo lo que le sirve a **quien
mantiene** (procedencia bibliográfica, mediciones, verificadores, avisos de cobertura
exclusiva). `tools/galeria.py` publica el título y la descripción; por eso un ejemplo nuevo
aparece solo en la galería, sin lista que tocar, pero **un encabezado mal formado sale
publicado**. ⚠️ **Las NOTAS quedan fuera de la DESCRIPCIÓN de la tarjeta, pero NO fuera de la
página:** el desplegable lleva el fuente COMPLETO, así que todo lo que se escriba bajo
`% NOTAS` (procedencia, mediciones, arqueología) **es cara pública**. Corregido el 2026-08-01
—esta línea decía «descarta las NOTAS», que era cierto de la descripción y falso de la
página—. La limpieza del 2026-07-23 barrió además la
arqueología V1 de los cuerpos (`FPATRN`, `SCST/RTST`, `LWIDTH`…) y **todas** las referencias
`§n` a la especificación: un ejemplo se lee solo. Se conservan las medidas (el verificador
`0.374 µm` de `fig_polybar`, la receta de `mutool` de `turning_points`) y el listado V0 de
1991 en `fractal_tree`, que es la fuente de la que se reconstruyó la figura. En esa pasada
se cazaron cuatro afirmaciones ya falsas —el «AÚN NO COMPILABLE» de `rpstest`, el font
Symbol de `symbols`, los «21 ejemplos» de `primitives` y la anécdota del man en
`quickstart`—: los comentarios envejecen y **nada los vigilaba** hasta `galfail`.

**Galería (2026-07-23; BILINGÜE y reordenada el 2026-07-27):** `tools/galeria.py` genera **dos**
páginas —`docs/galeria.html` (es) y `docs/gallery.html` (en)— publicadas en GitHub Pages
(fuente: `main`, raíz) y enlazadas entre sí; cada README apunta a la de su idioma. 26 tarjetas
con figura, título, descripción y el código completo en un desplegable, más una caja «Pruébalo»
con los tres comandos de clonar-compilar-dibujar y el aviso de beta. ⚠️ **El texto en inglés NO
sale de los `.mg`** (van comentados en español, política del proyecto): vive en la tabla `TRAD`
del propio tool, y un ejemplo sin traducir sale igual, en español y con aviso por stderr — la
regla de que un ejemplo nuevo aparezca solo pesa más que la uniformidad del idioma.
⚠️ **El ORDEN es una decisión editorial, no alfabética:** abre con «Figuras que se calculan
solas» (`orbita_polar`, `gravitacion_orbita`, `fractal_tree`…) y deja los catálogos al final.
Antes abría con `quickstart` y las dos figuras más vistosas caían al pie, en «Más ejemplos»,
porque no estaban en ningún grupo — que sigue siendo el destino de lo que no se liste. ⚠️ **Qué entra NO es la misma regla que `imgfail`:** la galería itera
sobre `examples/*.mg` que tengan `docs/img/X.svg`, mientras que la compuerta itera al revés
—sobre `docs/img`— y por eso vigila además las tres variantes que existen solo para los
ensayos (`franck_condon_anarm`, `turning_points_nodos`, `parabola_vs_arco`), que no son
corpus. Son dos reglas parecidas y distintas; no las unifiques.

**Cutover hecho (§22.6):** `bin/mg` en `main` **es el compilador V3** (se arma de `src/parserv3.cpp` + `src/lexv3.cpp` + motor + PDF/haru). `test/run.sh` compila el corpus de `examples/` con la salida del propio renderer V3 como red golden (regresión, no el oráculo V1); **reactivado 2026-07-11** (ver "Build and test"). `src/main.cpp` **sí es el entry point V3** y está en el build (Makefile: `bin/mg` = `main.cpp` + `lexv3.cpp` + `parserv3.cpp` + motor + haru). **El front-end V1 se BORRÓ del árbol el 2026-07-27** (`src/Parser.cpp`, `src/mgpp.l`, `include/Parser.h`, `include/MGLexer.h`, `include/mgpp_tab.h`), una vez cerrado el traductor: estaba fuera del build y su papel era servir de referencia de la semántica V1, que ahora se consulta en la rama —`git show v1-legacy:src/Parser.cpp`—. En la misma pasada se fue `include/font_cmmi.h`, huérfano desde la migración a LM Math. `main` es ahora **solo V3**. `make v3test` es un alias (`cp bin/mg bin/v3test`).

## Architecture

Pipeline (V3, post-cutover): `.mg` → **lexer** (`src/lexer.l` → `src/lexv3.cpp`) → **Parser V3** (`src/parserv3.cpp`: descenso recursivo → AST de `Stmt` → `exec` emite `GraphicsItem`s) → in-memory tree (`MetaGrafica`) → **Display** backend → EPS/SVG/PDF. *(El pipeline V1 —`mgpp.l` → `Parser.cpp`— vive solo en la rama `v1-legacy` desde el 2026-07-27; es la referencia de la semántica V1 que consulta `tools/mg1to2.py`.)*

- **`GraphicsItem`** (`include/primitives.h`) — abstract base of every drawable; hierarchy is non-copyable (use-count bookkeeping in `StructureUser`). `Path` = `std::vector<point>`.
- **`Structure` / `MetaGrafica`** (`include/structures.h`) — named reusable groups; `MetaGrafica` is the document (dimensions `$D` in cm, world window `WW`, font size). `StructureLine/Arc/Path/Rectangle` place structs geometrically.
- **`Display`** (`include/Display.h`) — abstract backend + device-independent state machine. Implementations: `EPSDisplay`, `SVGDisplay`, `PDFDisplay`.
- **`Matrix`** (`include/matrix.h`) — 3×3 homogeneous transforms; post-multiplies (`translate(); scale();` ⇒ `T·S`). `ellipse_frame()` maps an arc/ellipse to its **device frame** (center + conjugate semi-diameters `u,v`, i.e. `P(t)=C+u·cos t+v·sin t`) — the only form closed under affinity, and what all three backends consume; `ellipse_axes()` is the closed-form 2×2 SVD that turns that frame into true axes+angle, needed **only** by SVG (its `A` command can't take a matrix). ⚠️ Do **not** reintroduce radii-by-column-norm (`transform_radii`, deleted 2026-07-27): column norms are the *conjugate* semi-diameters, not the axes, and they only coincide when `u⊥v` — see `docs/bitacora.md` 2026-07-27.

### Coordinate system (implemented 2026-07-05, spec §3.1)

The engine is **isometric by construction**: `Display::pushWorldMatrix()` builds the single world→device seed matrix in each backend's `start()` — uniform *meet* scale `s = min(W/wdx, H/wdy)` plus centered margin. `stretch_mode` (internal bool, no CLI flag) reproduces V1 per-axis stretching for the future translator. Key invariants:

- The parser does **not** normalize document-level coordinates; the real `WW` goes to `MetaGrafica::setWindow` → `Display::setWindow`. Struct bodies **do** keep their local window normalized to the unit box (V1 placement contract).
- cm→pt scale is exact (§3.2); the `+0.5` rounds only the printed `%%BoundingBox`.
- No per-primitive aspect compensations exist anymore (no `getRatio()`, no `w=h` forcing in `arc`). Do not reintroduce them.
- V1 front-end translates V1 placement semantics ("struct box = min(canvas)") via `docwmin = min(wdx,wdy)` applied to all placement kinds (`RPST`/`DPST`, `LNST`/`ARCST`, identifier). This factor dies with the V1 front-end.
- Anisotropic user transforms (`SCST x≠y`, shear) legitimately produce ellipses; the parser sets `flags.using_ellipse` so EPS emits its procedure.

### Adding a new primitive

1. `GI_*` enum + subclass in `include/primitives.h`; 2. despacho por nombre en `parserv3.cpp` (`isPrim()` + `PrimStmt`, o un `Stmt`/`parse*` propio para sintaxis con bloque, p. ej. `axis`/`compound`/`plot`); 3. `draw(Display&)` calling `Display` virtuals; 4. implement those in the three backends. *(V3 despacha las primitivas por su nombre-cadena en `parseStatement`, no por token del lexer; solo hace falta tocar `src/lexer.l` para símbolos/operadores nuevos, no para comandos.)*

## Roadmap state (act. 2026-07-24)

El parser V3 (`src/parserv3.cpp`) compila los 31 ejemplos del golden de `examples/` a
EPS/SVG/PDF. Grande hecho: expresiones+control de flujo (§5-6), structs+invocación+place/fit/repeat
(§8/§10/§17), generadores §13 (numbers/ticks/axis/grid), primitivas geométricas+bezier+
sine, texto con markup, estado color/fill/line_width/dash/font/align/valign + atributos
por-primitiva (§7.5) con alcance correcto (gsave/grestore en EPS/PDF), transform §11.1.

**Tipografía math — CERRADA (2026-07-24).** `plan_text_space` (medición precisa de `Text`,
Parte A + espaciado automático estilo TeX, Parte B) y `plan_frac` (**`\frac` COMPLETO**: inline
en fórmulas mayores, anidado, con **colocación vertical por extent MEDIDO** —num/den según su
altura/profundidad real, ya no rozan la raya— a **tamaño display** y con el espacio Ord→Inner)
están **hechos**. `\frac` estrenó la primitiva `Display::fracRule` (la raya, en los 3 backends)
y generalizó `TextLine` a contenedor de `GraphicsItem`. Con eso **`gravitacion_orbita` entró al
golden** (`ok=69`), la figura que motivó todo esto.

**Añadido 2026-07-24** (cero churn salvo lo que se nota): **marcador hereda el color de su línea**
por default (`marker_color` pasa a ser override; antes el relleno salía negro) — modo nuevo
`AT_FCOLOR_FROM_LINE` + accesor `Display::getLineColor()`. **Añadido 2026-07-23:** `hatch_angle`,
`rectangle(w,h,at)`, `lib/` instalable con `include` **local→lib** (§15, `-DMG_LIBDIR`), fix de
`xml:space` en SVG.

**Arcos y elipses — reconstruidos 2026-07-27.** La rotación de arcos y elipses era incorrecta
en EPS y SVG (correcta solo en PDF): tres bugs con **una causa raíz**, los backends recibían el
arco *descompuesto* (centro, radios, ángulos) y reparaban esa descomposición con reglas ad-hoc
que no son cerradas bajo afinidad. Ahora EPS recibe la **matriz** (`concat`) y traza el arco
unitario con los ángulos intactos; SVG —único que no admite matriz en `A`— decide los ejes con
un SVD 2×2 en forma cerrada; PDF ya transformaba puntos de control. `arc` acepta por fin
`rx`/`ry` (§4.5), y llegaron `marker_at` (marcadores en ángulos paramétricos de
`arc`/`ellipse`/`circle`) y `place(..., rx/ry, at=)` (struct completa sobre un arco elíptico).
⚠️ **Antes de tocar geometría lee `plan_anisotropia.md`**: describe la familia de bugs
«fórmula isótropa aplicada al caso anisótropo», sus cuatro tics reconocibles, las tres
instancias cerradas, dos abiertas *por decisión de semántica*, y lo ya verificado limpio.

**`orbita_polar` — CERRADA y EN EL CORPUS (2026-07-27).** La figura que pidió todo lo
anterior. La oclusión de la mitad trasera de la órbita **no necesitó motor booleano**: es
profundidad, no conjuntos en 2-D, y con órbita y globo concéntricos sale en forma cerrada
(`sin t = √((R²−a²)/(b²−a²))`) evaluada **en el propio `.mg`** con la trigonometría que el
evaluador ya tenía. Cada órbita es un `arc` cuyo barrido salta el tramo oculto; los cortes
caen sobre el limbo por construcción. Cuál mitad va detrás **es modelado**, no geometría:
se eligieron opuestas para que se lean como planos que se cruzan.

Cerrado en cada sesión: **[`docs/bitacora.md`](docs/bitacora.md)** — el registro de qué se
cambió y por qué, sesión por sesión (26 y subiendo). ⚠️ **Léelo antes de tocar el motor o de
cambiar una decisión de diseño:** muchas entradas traen la medición que la sostiene, y varias
registran el camino que se probó primero y no funcionó. Vivía aquí dentro hasta el
2026-07-22; se mudó porque también le sirve a un colaborador humano.

✅ **Hilo CERRADO (2026-07-27):** `plan_orbita_polar.md` y `plan_anisotropia.md`, los dos.
La figura quedó terminada y en el corpus, `docs/referencia.md` se actualizó con lo que
destapó, y las **dos decisiones de semántica** que quedaban se tomaron sin tocar código: en la
ruta **log** de `plot` se mapean **posiciones, no formas** (los tamaños quedan en coordenadas
de la página — `especificacion_mg.md` §13.7 y `docs/referencia.md` §11), y el «out» de las
marcas de eje es perpendicular **en el papel**. `plan_anisotropia.md` se conserva por «La
firma» (los cuatro tics de la familia) y «Cómo cazar más»: es el documento que hay que leer
antes de meter geometría nueva.

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
  decimales fijos vs `<<` = 6 cifras significativas) y movería los 69 goldens a cambio de nada.
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
