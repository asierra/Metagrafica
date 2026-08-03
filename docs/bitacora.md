# Bitácora de ingeniería — MetaGráfica V3

> **Qué es esto.** El registro, por sesión, de **qué se cambió y POR QUÉ**. Vivía dentro de
> `CLAUDE.md` —donde eran 1079 de sus 1225 líneas— y salió de ahí el 2026-07-22: es material
> para cualquiera que vaya a tocar el compilador, y un archivo con nombre de herramienta es
> el único que un colaborador humano no va a abrir nunca.
>
> **Para qué sirve, en concreto:** para no re-litigar. Antes de cambiar una decisión del
> motor, busca aquí si ya se discutió; muchas entradas traen la medición que la sostiene, y
> varias registran el camino que se probó primero y **no** funcionó.
>
> **Dónde va cada cosa** (el reparto se fijó el 2026-07-22):
>
> | documento | contesta |
> |---|---|
> | `README` | qué es y por qué importa |
> | `docs/referencia.md` | cómo se escribe X |
> | `CONTRIBUTING.md` | cómo trabajo en esto — **la regla** |
> | **este archivo** | por qué es la regla, y cuándo se decidió |
> | `especificacion_mg.md` | por qué el lenguaje es así, y a dónde va |
> | `PENDIENTES.md` | qué falta |
>
> Orden **cronológico**, lo más nuevo al final. Las entradas no se reescriben cuando algo
> cambia después: son el registro de lo que se sabía ese día. Si una entrada y el código se
> contradicen, gana el código — y conviene añadir una entrada nueva, no editar la vieja.

---

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

### Cerrado en la sesión del 2026-07-21 (texto multilínea §14.1 — `TextBlock`)

**`/n` rompe el renglón**, con `TextBlock` (`GI_TEXTBLOCK`) nuevo en el motor: una lista de
renglones ya construidos —cada uno `Text` o `TextLine`, que ya saben su ancho y su
alineación— más el interlineado. **Cero cambios en los tres backends.** `TextStruct` sigue
**reservado** y comentado: un bloque *apila*, una estructura *compone* (fracciones,
sumatorios, radicales, o insertar LaTeX) — otro problema.

💡 **Por qué en el motor y no en el parser** (Alejandro propuso resolverlo en parse-time, y
la observación de fondo era correcta: multilínea es una lista de `TextLine`, no un
`TextStruct`). El obstáculo es concreto: el desplazamiento entre renglones es
`leading · font_size`, y **el tamaño de fuente solo existe en draw-time** —
`MetaGrafica::getFontSize()` está comentado (`structures.h:213`) y `font_size` viaja como
`AT_THEIGHT` en el flujo de items, que el parser no sombrea. Resolverlo en parse-time
obligaría a *adivinar* el tamaño heredado: **el cuarto bug de la familia `FN_NOFACE`**, y del
peor tipo, porque un interlineado mal calculado se ve **plausible** y no salta como un rótulo
en blanco.

Detalles que costaron mirar el código antes de escribirlo:
- **El corte va DENTRO del bucle de `parse_text`**, no partiendo la cadena antes, para que el
  estado tipográfico siga vivo entre renglones: `"/bTítulo/nsigue"` sale toda en negrita.
- **`TextLine::draw` mueve la pluma** (corrimiento de alineación + avance), así que no se
  pueden encadenar renglones con movimientos relativos en x. Se relee el ancla con
  `getPlumePosition()` en cada uno. ⚠️ Y hay una asimetría que hay que conocer:
  `moveto_nopath` **transforma** (coords de mundo) y **`rmoveto` NO** (unidades de
  dispositivo) — que es justo lo que se necesita para bajar `leading·font_size` en pt.
- **`valign` aplica al BLOQUE**, no a cada renglón (`middle` centra el conjunto). Con un solo
  renglón el desplazamiento es cero → comportamiento anterior intacto.
- Un renglón vacío (`"a/n/nb"`) consume interlínea y no dibuja.

🐞 **Y el arreglo destapó su propia recaída, en la primera figura real.** `TextBlock` volvía a
filtrar la cara ENTRE renglones: cada `TextLine` toma como ambiente lo que dejó el anterior, así
que en `"$\Delta T_1$/n(BT 10.3…)"` —el caso corriente: fórmula arriba, unidades abajo— la
segunda línea salía en la fuente del math. Se acota con `pushDrawState`/`popDrawState` por
renglón. **"Heredar la ambiente" tiene que significar la del bloque, no la del vecino de
arriba.** Verificado que NO rompe la conducta contraria: `/b` sí sigue vigente tras el salto,
porque el parser hornea la cara en cada trozo y solo lo heredado (`FN_NOFACE`) se resuelve
contra el bloque. Churn: `texto.eps`/`.pdf` ganan 4 pares `gsave/grestore` y el ráster sale
**idéntico píxel a píxel** (SVG ni se movió).

**Cobertura:** `texto.mg` gana dos rótulos multilínea (único sitio del corpus que ejercita
`TextBlock`; sin ellos el interlineado y el valign de bloque quedan sin red). Único golden
re-bendecido. Verificado antes de tocar nada que **ningún `.mg` usaba `/n` literal**, que era
el riesgo del cambio de significado.

### Cerrado en la sesión del 2026-07-21 (`figure_02` retomada: rótulos a dos renglones)

Con `/n` disponible, los rótulos de eje pasan a **dos renglones** como el original
(`"$\Delta T_1$/n(BT 10.3 - 12.3 $\mu/rm$)"`), y se resuelve el encimado que quedaba.

💡 **La causa del encimado era de ESCALA, no de acomodo**, y conviene recordarla porque se
repetirá: el texto es **físico** (pt) y los paneles son **cantidad de mundo**, así que a 24 cm
cada panel medía 5.3 cm —la mitad de los ~11 cm del original de 18 pulgadas— mientras la
leyenda seguía midiendo lo mismo, y crecía hasta meterse bajo la tabla (que es opaca y la
cortaba). La solución es agrandar el lienzo (33 cm; el original mide 45), no apretar el
cuerpo de letra. **Para una figura más chica, lo que se baja es `font_size`, no
`display_size`.**

Único acomodo que diverge del original: la leyenda del panel b pasa a abajo-derecha, donde
cruza solo la cola delgada del histograma en vez de las barras altas.

### Cerrado en la sesión del 2026-07-21 (retícula por eje + `frame` de plot)

**`xaxis(grid=…)` / `yaxis(grid=…)`** (+ `grid_dash=`), con el `grid=` de `plot` como atajo
para los dos ejes y el eje **ganando** sobre él (`grid=false` en un eje apaga la del plot).
Cierra la última divergencia de `figure_02` con su original: retícula **solo horizontal y
punteada**, que es lo que usa cualquier gráfica de barras.

⚠️ **NO se hizo `grid="y"`, que era la forma obvia, y la razón vale para el resto del
lenguaje:** `grid=` ya está sobrecargado con color, así que la `"y"` se habría leído como
**nombre de color** — aviso «color desconocido» y malla **negra en los dos ejes**.
Silenciosamente casi-plausible. Además, pedirla por eje es donde conceptualmente vive: la
retícula **es la marca del eje barrida por el campo**, y el código literalmente reusa
`axis(ticks="grid")` con el `step` y el `start` de ese eje. *Antes de sobrecargar un
argumento, revisar con qué ya compite.*

**`plot(frame=true)`** (o un color): el rectángulo del `box=`. Existe para **no repetir el
`box=`** — antes había que poner un `rectangle` por panel con los mismos cuatro números, que
se desincroniza en cuanto un panel se mueve. Va después del contenido y antes de los ejes.

De paso, `dash` entra a los atributos de estilo por-eje de `AxisStmt` (estaba en la lista
`{"color","line_width"}`), que es lo que permite la retícula punteada. Verificado que **nadie
en el corpus pasaba `dash=` ni `grid=` a un eje** —donde se ignoraban en silencio—, así que el
cambio es aditivo puro: `ok=57` sin re-bendecir.

### Cerrado en la sesión del 2026-07-21 (`include` fatal; categorías en variables)

**Un `include` que no resuelve ahora ABORTA** (`parseFile`, `parserv3.cpp`). Antes devolvía
`{}` y seguía, así que si el archivo perdido solo aportaba cosas opcionales —colores, structs
no usadas en todas las ramas— la figura se generaba **a medias con código de salida 0**.
Salió al copiar un `.mg` a otro directorio: el `include` es relativo al archivo, falló, y el
documento siguió hasta reventar más tarde por otra causa. Mismo criterio que hace fatal a
`evalError`: un documento incompleto no debe producir salida.

**`figure_02`: las categorías pasan a variables** (`cat_ash`/`cat_prob`/`cat_both`) y las
etiquetas se arman con concatenación (§5.2). Cambiar el vocabulario de la figura es ahora
**una edición**, y comparar las dos versiones cuesta comentar dos renglones — que es lo que
convierte una propuesta de terminología en *evidencia* para discutirla con el autor
principal, el argumento de `calcular_en_vez_de_medir.md` aplicado a la práctica.

📐 **Y midiendo salió quién manda en el tamaño del lienzo.** Con las etiquetas cortas el panel
a queda cómodo a 28 cm, pero **el que fija el mínimo es el panel e**: su etiqueta es la más
larga (`Band 13 < 273.15 Probable ash`) y su histograma el más ancho, así que a 28 no libra ni
arriba-izquierda ni abajo-izquierda. Se queda en 33. **La restricción es de CONTENIDO, no de
acomodo:** el tamaño de la figura lo decide la etiqueta más larga, porque el texto es físico.

### Cerrado en la sesión del 2026-07-21 (`sample`/`point_at`/`angle_at`, familia de muestreo §9)

**Implementada la pieza-palanca que MetaPost sugería** (`point t of p`): leer geometría de un
path en un parámetro `t`. Tres nombres cortos —`sample(&p,n)` (path-valuado), `point_at(&p,t)`
(devuelve `[x,y]`), `angle_at(&p,t)` (número)— con el **flag `curve=`** que resuelve el modelo
α+β decidido antes: el path-valor es NEUTRO (probado: el mismo `&sine` se dibuja recto con
`polyline` y curvo con `bezier`), así que el flag carga la interpretación —`false`=vértices/
lineal, `true`=controles bézier/cúbica— en vez de que el valor lleve un tipo (γ, descartado).

- **Geometría en `splines.cpp`:** `bezier_point` (Bernstein cúbico, los ~6 renglones), y
  `arc_table` que **poliliniza y acumula longitud de arco** (SUB=24 tramos/segmento bézier),
  compartida por `path_point`/`path_sample`/`path_angle`. La **longitud de arco no es
  opcional** (spike previo): hace que `t=0.5` sea el medio geométrico. De una polilínea es
  trivial (sumar segmentos); de una curva, se teselaba igual que para dibujar.
- **Cableado en `parserv3.cpp`:** `point_at`/`angle_at` son `Expr` (puente Expr↔PathExpr como
  `path_width`); `sample` es operación de PathExpr (como `concat`/`reverse`). `curve=` acepta
  nombrado o posicional. **Cero motor nuevo en backends, cero elementos gráficos.**
- **Verificado visualmente** —lo que cierra el hilo de pegaso—: los puntos `curve=true` caen
  SOBRE la curva; los `curve=false` (default), sobre la ENVOLVENTE (más lejos en los picos,
  donde la curva se aparta de sus controles). Golden `ok=60` (aditivo puro).

⏳ **Diferido / pendiente:** (1) las reducciones `path_x_*_at_y` ganarán el mismo `curve=` en
otra tanda —hoy siguen tocando la envolvente—; (2) **SIN cobertura en el corpus**: función de
lenguaje sin figura que la ejercite = puede romperse en silencio, cerrar con un ejemplo;
(3) `point_at` devuelve `[x,y]`, que va en `at=`/`box=` pero aún no en un bloque `{ }`.
`path_width` **conserva** su prefijo (colisión con el atributo `width=`).

### Cerrado en la sesión del 2026-07-21 (el bloque de coordenadas acepta un punto)

**`marker { point_at(...) }` ahora dibuja** — el bloque `{ }` de una primitiva acepta un
**punto `[x,y]`** donde iría un par de escalares, así que `point_at` (que devuelve un punto)
compone en cualquier primitiva sin envolverla en struct. Cierra el hueco que destapó
`path_sample`: **las primitivas no tienen `at=`** (es de structs), y `marker(at=…)` compilaba
sin dibujar nada — un `at=` ignorado en silencio, de la peor clase.

- **Un solo sitio:** `PrimStmt::evalPath` pasó de `for(i+=2)` a una máquina de estados
  escalar-o-punto — un término que evalúa a `Value::LIST` de 2 aporta el par entero; un
  escalar se empareja con el siguiente. Cubre TODAS las primitivas (comparten `evalPath`):
  `marker`/`dot`/`polyline`/`polygon`/`rectangle`/`circle`/`arc`/`ellipse`/`polybar`. Mezcla
  libre: `polyline { 0 0  (p)  5 5 }`; literales: `dot { [3,4] }`.
- ⚠️ **La paridad se movió a EVAL-TIME.** `checkCoordPairs` la validaba en parse-time contando
  términos, pero un punto vale 2 coordenadas siendo 1 término, y **un punto en variable no se
  distingue de un número hasta evaluar**. Flag `allowsPoints` que difiere el chequeo en las
  primitivas; los bloques que NO aceptan puntos (smooth/place/literal de path) conservan el
  estricto con línea:columna. **Único costo:** el error de coordenada impar en una primitiva
  ya no trae línea:columna (sí nombra la primitiva) — se sigue cazando, y el bug del footgun
  de paréntesis (`{ 1/u (u*u-u) }`) también, solo que en eval-time.
- `bezier` NO acepta puntos (controles escalares; el conteo 3k+1 sigue en parse-time).
- `path_sample.mg` gana un `marker` suelto orientado con `rotate=angle_at(...)` **además** de
  la flecha-struct, para mostrar los dos caminos. Golden `ok=63`.

### Cerrado en la sesión del 2026-07-22 (recursión + 5ª compuerta + barrido de silencios)

**`max_depth` (§18) implementado** — la recursión de structs (§8.1) funcionaba pero sin
tope agotaba la pila: `struct r(n) { … r(n+1) }` daba **SIGSEGV (139)**, el único modo de
falla que no pasaba por `evalError`. Default 32. 💡 **La decisión fue DÓNDE poner la
guarda:** hay **cinco** sitios que expanden un cuerpo de struct (`InvokeStmt`, `RepeatStmt`,
`FitStmt`, `buildStructure`, volcado de plot-log) y con la guarda solo en `InvokeStmt` tres
de las cuatro vías de invocación seguían muriendo → helper único `execStructBody`. Cuenta
**anidamiento, no invocaciones** (verificado: 40 colocaciones planas no gastan profundidad).
`max_depth n` es **control de documento** (`isConfig`), no sentencia de estado: no es estado
gráfico. Dato de archivo: `MAXDEEP` sobrevivía en el léxico de V1 (`src/mgpp.l:43`) pero
`parseDef` no tenía caso para ella — se ignoraba, igual que el `$S 1` de las cónicas.

**`examples/fractal_tree.mg`** (corpus 21→22, `ok=63`→**`ok=66`**) — Fig. 4 del artículo de
V0 (*Ciencias* 21, 1991; `docs/11195-10937-0-PB.pdf`), reconstruida del listado impreso en
su Apéndice 1. **Único ejemplo con recursión** (barrido: cero en todo el árbol antes de él).
🔎 **Lo que decidió construirlo:** ese listado **no tiene condición de paro** —V0 no tenía
condicionales— así que el límite de profundidad era **infraestructura de carga**, no una red.
El mapeo V0→V3 es casi 1:1 y revela que **V0 ya tenía structs parametrizadas en 1991**
(`VAR THETA PHI`). ⚠️ Al encuadrar: si `world_window` deja de tener el aspecto de
`display_size`, el meet **encoge el dibujo entero** (2.72 de ancho → 88.2%) — se lee como
«salió chica», no como «se deformó», que es lo que lo hace difícil de diagnosticar.

**5ª COMPUERTA: pruebas negativas (`errfail`, `test/errors/`).** Las otras cuatro miran
salida EXITOSA, así que los **152 caminos de error** del compilador (51 `evalError`, 94
`parseError`, 7 `exit`) no tenían **ninguna** prueba — y su regresión natural es volver al
**silencio**, que no mueve un byte de ningún golden. Cada fixture declara lo que espera en
su propio encabezado (`% EXPECT:` + `% EXPECT_AT:` opcional), va en git y no hay lista que
tocar. Tres aserciones: **`exit == 1` exacto** (no «≠ 0»: un segfault también «falla» — es
la que caza el modo de falla de `max_depth`), el fragmento aparece, y **no se creó archivo
de salida** (la política de que un documento roto no produce salida). Se compara **fragmento
y no mensaje completo a propósito**: los mensajes son prosa que se reescribe, y un golden
por bytes castigaría las mejoras de redacción. Verificada como las otras: con el silencio de
`emitStyleAttr` reintroducido, el golden da `ok=66 fail=0` y las cuatro viejas quedan
**ciegas**.

**Barrido de silencios — cinco cerrados**, todos con cero churn:
- **Aridad de structs** (la destapó la compuerta al sembrarla): `S(1)` sobre `S(a,b)` dejaba
  **`b = 0`** y dibujaba a (1,0) en vez de (1,3) — figura **plausible**, la peor variante.
  Cuatro casos en `bindStructParams`: falta argumento, sobran posicionales, **nombrado
  desconocido** (typo) y duplicado posición+nombre.
- **`scale sx sy`** descartaba el 2º factor si era variable. No se parcheó, se **decidió**:
  un identificador seguido de **fin de sentencia** no puede *ser* una sentencia (las de
  estado piden argumento, una invocación pide `(`), luego es el 2º factor. Excepción con
  nombre: `outlinefill`, la única sentencia de cero argumentos. ⚠️ `scale s (q)` **no** es la
  salida: choca con el footgun de que `ident (` se parsea como llamada.
- **Atributos de primitiva y de `text()`**: `marker(rotate=90)`, `polyline(colour="red")`,
  `text("h", tamano=20)` compilaban mudos. Listas **separadas** (los ejes no se solapan).
  💡 **El corpus cazó mi lista incompleta al primer intento** (`marker_start_orient=` de
  `fig2-5`, que existe y está en la spec pero se pasa a un helper): **una lista blanca sacada
  de los accesos DIRECTOS está incompleta por construcción.**
- **`exit` (§18)** implementado, en `parseProgram` —que ES el nivel de archivo, y lo usan
  tanto el documento como cada `include`—. Anidado es error (parse-time: dentro de un `if`
  no sería condicional). Corta errores de **sintaxis** posteriores pero no **léxicos**: el
  lexer tokeniza el archivo entero antes.

⏳ **Queda abierto de la misma familia:** los **generadores** (`axis`/`numbers`/`grid`, y
previsiblemente `plot`/`legend`/`table`/`rule`/`place`/`repeat`/`fit`) siguen tragando
nombres desconocidos. Es el mismo bucle de una línea, pero **el riesgo es la LISTA, no el
código** (ver la lección de arriba), así que conviene hacerlo **al escribir la referencia**
(condición 5), que es el ejercicio de enumerar qué acepta cada constructo.

**Retirados:** `ideas.txt` (borrador fundacional; 14 de 18 puntos superados, 2 resueltos por
decisión) y el `TODO` de 2024 (4 de 5 cerrados). Lo que debía sobrevivir está transcrito en
`PENDIENTES.md`. El **editor web** queda **condicionado a la condición 4**, ni descartado ni
abierto sin fecha (`plan_interactivo.md`): su valor no se puede evaluar sin usuarios. 🔎 Y el
dato para cuando reaparezca: **la barrera medida no es instalar** — ocho tropiezos
documentados (cuatro del autor, cuatro del agente) y ninguno fue «no pude compilar».

### Cerrado en la sesión del 2026-07-23 (`hatch_angle`: desacoplar orientación de tipo)

**`hatch_angle` — orientación de trama como perilla propia.** Surgió armando
`lib/satellite.mg` (un icono de satélite para clase, fuera del corpus): la rejilla recta de
los paneles solares obligaba a superponer **dos** tramas simples (`hatch=0` + `hatch=90`)
porque `crosshatch` estaba cableado a 45°/135°. 💡 **La forma correcta la propuso Alejandro
y era mejor que la mía:** yo había anotado `crosshatch=<ángulo>` (otra sobrecarga); él pidió
separar tipo de ángulo. Resultado: `hatch` = **qué** trama, `hatch_angle` = **a qué** ángulo,
`hatch_gap` = **a qué** paso — tres perillas ortogonales. `hatch_angle=0` endereza el
`crosshatch` a 0°+90°; el default (45°) queda igual.

**El reparto de trabajo entre backends fue lo instructivo.** EPS y PDF ya iteraban sobre el
ángulo de cada `HatchLine` (`th = 90 - h.angle`), así que soportaban cualquier orientación
**gratis** — cero cambios. El único rezagado era SVG: su emisor de familia **simple** ya
giraba con `patternTransform="rotate(90-θ)"`, pero el de familia **doble** (crosshatch) estaba
cableado a dos diagonales `√2` a 45°, ignorando los ángulos. 🔑 **La generalización salió más
simple que el caso especial:** como un `crosshatch` es por construcción `{θ, θ+90}`, una
**rejilla cuadrada girada θ** ES exactamente eso — tile `gap×gap` con una horizontal y una
vertical, orientado con `patternTransform="rotate(θ)"`. Eso cubre CUALQUIER ángulo, así que el
"nivel 3" que se había estimado difícil (rejilla oblicua arbitraria) cayó junto con el fácil.
El único caso genuinamente duro —una malla **no** ortogonal— no es producible hoy.

**Cero churn, verificado contra el binario pre-cambio.** La rama por defecto (45/135) conserva
su `fprintf` histórico tras una guardia explícita, así que `fill_styles` (único que usa
crosshatch) sale byte-idéntico en los tres backends. Trampa de método anotada: el EPS incrusta
el **nombre del archivo de salida** en `%%Title`, así que `cmp` de dos archivos con nombres
distintos «difiere» sin que cambie nada — con nombre fijo, old==new idéntico.

**`hatch_angle` entró a `isKnownPrimAttr`** (la 5ª compuerta lo valida contra typos).
Spec §4.11 y `docs/referencia.md` actualizadas; de paso se documentó la ergonomía que destapó
la sesión: el color de la trama sale de `fill=` (la trama ES el relleno), no de `color=` (que
contornea). ⚠️ **Cobertura de harness pendiente:** la rama SVG girada no tiene golden (el
satélite vive en `lib/`); el corpus solo ejercita el 45° legacy. Anotado en `PENDIENTES.md`.

### Cerrado en la sesión del 2026-07-23 (`rectangle(w,h,at)` — centro + tamaño)

**Forma alterna de `rectangle` (§4.4).** Además de las dos esquinas del bloque, ahora acepta
`rectangle(w=, h=, at=)`: `at` es el **centro** (como el de `circle`/`dot`), `w`/`h` el tamaño,
dar solo `w` hace un cuadrado, y omitir `at` lo pone en el origen. Lo pidió Alejandro colocando
el icono del satélite: calcular las dos esquinas para *ubicar* una caja es incómodo. Sintetiza
las esquinas en `PrimStmt` (`parserv3.cpp`), así que hereda todo lo demás (transformable,
tramado, reflejo). **Cero motor nuevo en backends**, cero churn (`ok=66`: ningún ejemplo lo
usa aún). `at` es el centro y no una esquina por consistencia con las primitivas centradas.

Dos guardas, con fixture cada una (`err_ok=36`→**38**): dar bloque **y** `w/h/at` es error
(ambiguo), y `at` sin tamaño también (era el no-op mudo que la política del proyecto persigue).
El nombre es `w`/`h` y no `width`/`height` porque `width` ya es el ancho de barra de `polybar`.
🔎 Detalle: los tres nombres entran a la lista COMÚN `isKnownPrimAttr`, así que `circle(w=2)`
los acepta y los ignora —la misma holgura que `circle(from=90)` hoy—; afinar por-primitiva es
la vuelta posterior de siempre.

### Cerrado en la sesión del 2026-07-23 (`lib/` instalable + búsqueda `include` local→lib)

**La biblioteca deja de ser una carpeta suelta.** Motivado por el icono del satélite: para
que `include "satellite.mg"` sirva desde cualquier figura sin ruta relativa, `lib/` pasa a ser
la biblioteca estándar **instalable**. `make install` copia `lib/*.mg` a
`$PREFIX/share/metagrafica/lib`, y esa ruta se **hornea en el binario** vía `-DMG_LIBDIR`
(CPPFLAGS). `parseInclude` busca en orden: ruta **absoluta** tal cual → **local** (junto al
archivo principal, `g_baseDir`) → **lib instalada**. **Lo local pisa lo instalado** (tantear
una variante sin reinstalar). Sin la macro (build sin `MG_LIBDIR`) solo mira lo local, así que
el árbol de desarrollo sigue usando rutas relativas (`../lib/satellite.mg`).

🔎 **Trampa de método anotada:** `make LIBDIR=/tmp/…` NO recompila —make no rastrea cambios de
variable, y `parserv3.cpp` se compila en la regla de ENLACE—; hay que `touch src/parserv3.cpp`
para forzar el relink con el nuevo `MG_LIBDIR`. Verificado así el fallback (cae a la lib) y el
override (una copia local gana). El mensaje de `include` perdido ahora **lista dónde buscó**
(mejor diagnóstico); se actualizó su fixture (`include_perdido`, la 5ª compuerta lo cazó al
cambiar el texto). Cero churn en el golden (`ok=66`); la resolución de includes no toca la
salida. Cierra la 3ª de las tres necesidades que destapó `gravitacion_orbita`.

### Cerrado en la sesión del 2026-07-23 (medición precisa de `Text` — `plan_text_space` Parte A)

**`text_width` mide ahora lo que se dibuja.** Un run `FN_TEX_CMMI` se DIBUJA partido —griego
(∈ `cmmiUnicode`) en LM Math, no-griego (`E`, dígitos, `=`, espacio) en Times-Italic— pero se
MEDÍA todo con `cmmi_metrics_map`. La medida no cuadraba con el render y el centrado/`fit`/
`\frac` se iban a la deriva. Ahora `text_width` parte por `cmmiUnicode` y mide cada byte con el
mapa de su fuente real (`text.cpp` alcanza `cmmiUnicode()` con `#include "text_parser.h"`).

🔎 **Requisito de Alejandro** («los `Text` deben dar sus medidas precisas»): es **la fundación
de `\frac`**, que dimensiona la fracción con `TextLine::width()`. El spike (mismo día) corrigió
mi diagnóstico —el respaldo es Times-Italic (`=`=675), no Times-Roman (564), así que sobrestimé
el error ~2×— y reveló que **EPS/PDF centran con operadores de fuente**, no con `text_width`, así
que la imprecisión **solo mordía al SVG**; el beneficiario real de medir bien es `\frac`.

**Churn: solo `sines.svg`** (una corrección; la math queda bien posicionada). El cambio de
`fmmap[ch]` por `find()`+fallback —endurecimiento contra el 0 mudo de un glifo ausente— se midió
aparte y **no agregó churn**. **No** se movió la partición al parser (el supuesto que daba miedo):
se arregló `text_width` para que CUADRE con lo que los backends ya hacen, dejando la partición
donde está. Falta la Parte B (espaciado automático) y bendecir `sines.svg` (`capture`+`images`).

### Cerrado en la sesión del 2026-07-23 (espaciado math automático estilo TeX — `plan_text_space` Parte B)

**El modo math ya no imprime los espacios del fuente: los pone la tabla de clases.** Motivado
por `gravitacion_orbita` (`$F = G m_1 m_2$` con espacios de más). Ahora, dentro de `$…$`, los
espacios del fuente se **consumen** (puro TeX) y el espaciado sale de clasificar cada átomo
—Ord/Op/Bin/Rel/Open/Close/Punct— e insertar entre pares el glue de TeX (thin=3/18,
med=4/18, thick=5/18 em). El espacio viaja como `pre_space` (em) en `TextState`;
`TextLine::width()` lo suma y `TextLine::draw()` lo aplica con un `rmoveto`.

🔎 **Tres decisiones de diseño, cerradas con Alejandro antes de escribir** (la política de
"discutir el lenguaje antes de implementar"): **(1)** puro TeX —consumir todos los espacios—,
con la consecuencia asumida de que las variables yuxtapuestas quedan pegadas (`G m_1 m_2` →
`Gm₁m₂`, tipografía matemática estándar); **(2)** anular el espaciado dentro de sub/super-
índices (como TeX en script style: `x^{a=b}` sin thick interno); **(3)** regla TeX del unario
—un `+/−` al inicio o tras Bin/Op/Rel/Open/Punct es Ord— así `-U_A` queda pegado y `a-b` con
med a ambos lados.

🔧 **Piezas nuevas** (`text_parser.cpp`): `mathClassOfByte`/`mathClassOfName`, la tabla
`mathGlue`, y `mathAtomSpace` (reclasa el unario, anula en índices, suma los overrides,
avanza `math_prev_class`). `mathSeal()` sella el run previo para que **dos entradillas no se
fusionen** —`=` y la `y` que le sigue, ambas con thick, colapsarían en `=y` perdiendo un
espacio—. `textflush()` **resetea `pre_space` tras hornearlo** (si no, el siguiente run lo
heredaba: un espacio espurio tras un símbolo). Overrides explícitos `\,` `\;` `\!` `\quad`
(aditivos). `add_symbol`/`add_word` reciben el `pre_space`.

**Churn: 6 ejemplos** (quickstart, sines, franck_condon, texto, fig6-4, fig_polybar), los 3
backends, **verificados a ojo uno por uno** (rasterizados vs. el golden limpio): todos mejoran
a espaciado TeX-correcto —`y = x²`, `φ = sin(nπx), n = 1..4`, `v' ≈ 6`, `λ⁻¹(s)`—. `fig1`
quedó **byte-idéntico** (el unario hizo `-U_A` igual que antes) y `symbols`/`turning_points`
tampoco se movieron (átomos sueltos). **c3fail=0** en todo (la paridad de operaciones de texto
entre backends aguantó) y **psfail=0**. Smoke test aparte —overrides, unario/binario en
todas las posiciones, consumo de espacios, varios `$…$` en un `text()`— correcto.

⚠️ **Trampa de método anotada:** el golden LOCAL estaba **rancio** (8 fallos de base antes de
tocar nada: `fill_styles`, `fig1.svg`… por ediciones de fuente posteriores a su última
captura). No se puede medir el churn contra una red rancia: hubo que **capturar un baseline
limpio** (con el árbol stasheado) y recién entonces diferenciar. Bendecido con `capture` +
`images` (los 6 renders públicos y la galería regenerados). Con A y B hechas, lo único que le
falta a `gravitacion_orbita` para entrar al golden es `\frac` inline (`plan_frac.md`), que ya
hereda medición (A) y espaciado (B).

### Cerrado en la sesión del 2026-07-24 (`\frac` inline en producción — `plan_frac.md`)

**El `\frac` del spike pasó de standalone a INLINE y compone igual en los tres backends.** El
spike (2026-07-23) solo detectaba `\frac` cuando era *todo* el `text()`; ahora vive **dentro**
de una fórmula (`$F = \frac{G m_1 m_2}{r^2}$`), se anida (`$\frac{1}{1+\frac{1}{x}}$`) y lleva
espaciado binario correcto alrededor (`$x + \frac{a}{b} - y$`). Golden **`ok=66` intacto** en
todo momento (ningún ejemplo usa `\frac` todavía → es puramente aditivo).

🧱 **Paso 1 — `TextLine` generalizado** de `vector<unique_ptr<Text>>` a
`vector<unique_ptr<GraphicsItem>>`, para que un `Fraction` conviva como átomo inline entre
trozos de texto. Refactor **puro**: la ruta `GI_TEXT` de `width()`/`draw()` quedó idéntica y el
golden salió byte-idéntico. `addText` sigue aceptando `Text` (upcast); `addItem` entra para lo
demás. El ancho por item se despacha por `getType()` (sin RTTI) en `TextLine::itemWidth`.

🧩 **Paso 2 — detección inline** en el bucle math (`case '\\'`, junto a los símbolos): al ver
`\frac` se extraen los dos `{..}` balanceados y se recursa cada uno como math. Dos trampas que
el standalone esquivaba y el inline no podía: **(a) reentrancia** —`parse_text` usa estado
global de archivo y una llamada recursiva lo reinicia—, resuelta partiéndolo en *wrapper
(codifica UTF-8 una vez)* + `parse_text_core` (ya-codificado) y un `parse_sub` que **salva y
restaura** todo el estado global alrededor de la recursión; **(b) doble codificación** —extraer
`A/B` del `input` ya-codificado y re-pasarlos por `UTF8toISO8859_1` corrompería los acentos—,
evitada porque `parse_sub` procesa el cuerpo ya codificado sin re-codificar. La fracción se
clasifica **Inner** (`mathAtomSpace(MC_INNER)`) para heredar el glue de la Parte B; su
entradilla viaja como `Fraction::pre_space`, que `TextLine` suma en `width()` y aplica en
`draw()`, igual que el `pre_space` de un `Text`. El bloque standalone del spike se **eliminó**:
`$\frac{1}{2}$` es ahora un `TextLine` de un solo `Fraction`, y el centrado pasa por `width()`.

🎯 **Paso 4 — avance de pluma y la primitiva `fracRule`.** `Fraction::draw` se reescribió para
inline: arranca en la pluma actual (que `TextLine` ya colocó, alineación incluida) y la deja
**avanzada por `W`**, con movimientos device-relativos de **neto cero** alrededor de cada parte.
La razón de que sean neto-cero y no `gsave/grestore`: **el SVG no salva la pluma simulada
(`cur_x/cur_y`) en push/popDrawState**, a diferencia del `currentpoint` nativo de PS/PDF —
apoyarse en `gsave` divergía entre backends (era la raíz del "SVG roto" del spike). La **raya**
no se puede trazar con `rmoveto`/`rlineto` en SVG (su `rmoveto` no escribe al path-builder, a
propósito, así que el path empezaría con `l` sin `M`): se añadió una **primitiva `fracRule(dy,
len, lw)`** a `Display` + los 3 backends, que cada uno resuelve en su modelo nativo (`currentpoint`
en PS; `cur_x/cur_y` en PDF/SVG) sin tocar la pluma. Es un método nuevo en los backends —el
spike presumía "cero"— pero es la resolución limpia del subproblema que el plan reconocía como
propio del SVG.

✅ **Verificación visual (EPS/SVG/PDF rasterizados y comparados):** composición estructural
correcta y **idéntica en los tres** —centrado, avance, anidado, espaciado binario—; el
**placement/centrado del SVG quedó ARREGLADO** (el subproblema abierto del plan), porque al
unificar bajo `TextLine` el `dx` de alineación ya cuenta el ancho real de la línea.

📐 **Métricas verticales, primera pasada "barata".** Las constantes del spike descolgaban mal
el denominador (`axis - denRaise` = 0.02·fs → **atravesaba la raya** incluso en `1/2`). Un ajuste
intermedio (`axis=0.30`, `numDrop=0.35`, `denRaise=0.70·fs`) limpió las fracciones sin scripts,
pero las holguras fijas **no miran el extent real**, así que `\frac{m_1 m_2}{r^2}` seguía
rozando: el superíndice `²` del denominador subía 0.65·fs y cruzaba la barra. Se dejó anotado y
se atacó a continuación.

### Cerrado en la sesión del 2026-07-24 (`\frac`: extensión vertical MEDIDA — cierra el punto 3)

**El numerador y el denominador se colocan ahora según su altura/profundidad REAL, no con
holguras fijas.** Motivado por `gravitacion_orbita` reescrita con `\frac` (la dejó Alejandro):
`$F = G \frac{m_1 m_2}{r^2}$` mostraba el `²` de `r²` atravesando la raya, porque las holguras
fijas ignoran que el denominador lleva un superíndice alto. Ahora `Fraction::draw` **mide** cuánto
baja el numerador de su línea base (incluyendo subíndices) y cuánto sube el denominador
(incluyendo superíndices), y coloca cada uno para que su borde hacia la raya la libre por
`kFracGap`.

🔧 **Piezas** (`text.cpp`/`text.h`): `runVExtent` (extensión de UN run: corrimiento de script
—los MISMOS 0.56/0.14 que aplica `TextLine::draw`— + altura de mayúscula / profundidad de
descendente aproximadas), `childVExtent` (despacha Text/TextLine/Fraction, **mutuamente
recursiva** con `Fraction::vExtent` → fracciones anidadas miden su altura real), `TextLine::vExtent`
y `Fraction::vExtent`. Constantes compartidas `kFracAxis/kFracGap/kGlyphAscent/kGlyphDescent`
para que medir (vExtent) y dibujar (draw) concuerden. **No hay métricas verticales por glifo**
(los mapas solo dan ancho): la altura de mayúscula (0.70) y la profundidad (0.18) son aproximadas,
así que en glifos sin ascendente/descendente plenos la holgura es un pelo generosa — pero **ya no
hay colisión**, que era el defecto.

✅ **Verificado a ojo (EPS+SVG, rasterizados):** `\frac{m_1 m_2}{r^2}` con subíndices sobre la
raya y superíndice debajo; el anidado `\frac{1}{1+\frac{1}{x}}` se descuelga para librar la raya
externa; `gravitacion_orbita` (ambas fórmulas) queda **limpia y con paridad EPS/SVG**. Cambio solo
visual (ningún ejemplo del golden usa `\frac`): **golden `ok=66` intacto**.

🔤 **Espacio antes de la fracción (Ord→Inner).** Comparando contra las referencias que dejó
Alejandro (`Fuerza_gravitacion.png`, y las de alta resolución `gravitacional.png`/`centripeta.png`),
faltaba el fino entre `G` y la fracción: `G\frac…` salía pegado. Se añadió **`MC_INNER`** al enum
de clases math y a `mathGlue` con las reglas del TeXbook (Ord/Close/Punct/Inner ponen thin ANTES
de un Inner; Inner pone thin/med/thick según el vecino), y la fracción se reclasificó de `MC_ORD`
a `MC_INNER`. `G \frac{…}` ya lleva su fino.

📏 **Tamaño: display style (pleno), NO scriptstyle.** Se probó encoger num/den a scriptstyle
(≈0.75, reduciendo el tamaño del DISPOSITIVO —no cada run— para que el corrimiento de script
`0.56·getFontSize()` escalara solo y el anidamiento cayera en scriptscript). Pero las referencias
de alta resolución muestran num/den **a tamaño pleno** (display style, como en `\[…\]` de LaTeX,
no inline). Se **revirtió** a `kFracScript = 1.0`; la maquinaria de escala queda como knob (un
constexpr) por si algún día se quiere la variante text-style inline. Con pleno, las dos fórmulas
de `gravitacion_orbita` **cuadran con las referencias** (tamaño, espacio y estructura; el `mv^2`
con el `²` alto y `r`/`r^2` bajo la raya). El denominador con superíndice cuelga un pelín más que
LaTeX porque el corrimiento de superíndice (`0.56·getFontSize()`) es constante GLOBAL del motor
—la comparten todos los `x²`/`λ⁻¹` del corpus— y bajarla los movería a todos: no se tocó.

⏭️ **Lo que falta para cerrar `gravitacion_orbita`:** ya solo es meterla en `test/run.sh` y
bendecir golden + `docs/img` + galería (decisión de commit consciente — toca ejemplo publicado y
red golden). La composición de `\frac` (inline + SVG + extent vertical + espacio Inner +
tamaño display) está **completa**.

### Cerrado en la sesión del 2026-07-24 (marcador hereda el color de la línea por default)

**Un marcador sobre una línea toma AHORA el color de esa línea sin pedir nada.** Lo destapó
`gravitacion_orbita` en clase: para una flecha roja había que escribir `color="red",
marker_color="red"` —redundante—, y la intuición (bien planteada por Alejandro) es que
`marker_color` debería ser el OVERRIDE, no un requisito para lo esperable. Antes, en `wrapMarkers`,
el relleno del marcador salía **negro por default** (heredado de `dot` suelto) y el trazo heredaba
el ambiente.

🔎 **Sutileza de orden que corrigió el diagnóstico:** el `color=` por-primitiva (§7.5) se envuelve
en push/**pop alrededor del TRAZO**, y `emitMarkers` corre DESPUÉS del pop — así que al dibujarse
el marcador ese color ya NO está vigente en el dispositivo. Por eso el marcador ni siquiera
heredaba el `color=` de su polyline (salía negro entero, no solo el relleno). El «general de
draw-time» ingenuo (leer `dspstate.linecolor`) habría leído el AMBIENTE, no el rojo.

🔧 **Solución (dos rutas, la correcta):** en `wrapMarkers`, sin `marker_color` el color del
marcador (trazo+relleno) sale de **`named["color"]`** (el `color=` por-primitiva, reusado
explícitamente porque ya se popeó); y si no hay `color=` propio, el relleno **sigue al color de
línea AMBIENTE en draw-time** vía un modo nuevo `AT_FCOLOR_FROM_LINE` (`Attribute::draw` →
`g.setFillColor(g.getLineColor())`; se añadió el accesor `Display::getLineColor()`). El trazo ya
heredaba el ambiente. `marker_color`/`marker_fill` siguen igual como overrides.

**Churn: CERO** (golden `ok=66`). Las flechas del corpus van sobre líneas negras (relleno negro =
color de línea, idéntico) o traen `marker_color` explícito. Verificado en los 3 backends: la
figura da flechas roja («Atracción») y verde («Velocidad») sin `marker_color`, y un smoke test del
caso ambiente (`color "blue"` → flecha azul; negra → negra) cuadra EPS=SVG. **`gravitacion_orbita`
quedó más limpia**: se cae el `marker_color="red"` redundante.

### Cerrado en la sesión del 2026-07-24 (`gravitacion_orbita` ENTRA al golden — corpus a 23)

**La figura que motivó `\frac` ya es parte de la red golden.** Con `\frac` completo (inline +
extent vertical + display + Inner) y el marcador heredando el color de línea, se integró
`gravitacion_orbita` al corpus: se añadió a la lista explícita de `test/run.sh` (23 ejemplos ×
EPS/SVG/PDF = **69 goldens**), se limpió su encabezado (fuera el aviso «FUERA DEL GOLDEN / `\frac`
no existe»; la descripción dice ahora fuerza roja / velocidad verde y estrena `\frac`+`include`+
marcador-hereda-color), se creó `docs/img/gravitacion_orbita.svg` (la presencia del archivo es la
declaración de la compuerta `imgfail`) y se bendijo con `capture` + `images` (galería incluida →
22 tarjetas). **`check` → `ok=69 fail=0 psfail=0 c3fail=0 imgfail=0 errfail=0 galfail=0`.** CLAUDE.md
al día (conteos 22→23 / 66→69, narrativa de `gravitacion_orbita`, roadmap de tipografía cerrado).
La figura va a una clase real de Alejandro; los tres PNG de referencia que dejó (`gravitacional.png`,
`centripeta.png`, `Fuerza_gravitacion.png`) eran solo de la sesión.

### Cerrado en la sesión del 2026-07-24 (llamada = `nombre(` PEGADO — se acaba el footgun del paréntesis en coordenadas)

**Una variable pegada a un `(` ya NO se traga el término siguiente como llamada.** Lo levantó
Alejandro mejorando `fill_styles`: en un bloque de coordenadas `{ a b (c) (d) }`, la `b` seguida
de `(c)` se parseaba como `b(c)` («llamada inválida a función b»), obligando a parentizar TAMBIÉN
las variables sueltas (`{ (a) (b) (c) (d) }`) aunque no fueran expresiones. Choca con el objetivo
de que el lenguaje sea intuitivo.

🔎 **Causa:** `parseAtom`, al leer un identificador, aceptaba con avidez el `(` siguiente como
llamada (`if (lx.accept(T_LPAREN))`) **ignorando el espacio**. Y en este lenguaje el espacio
SEPARA (config, coordenadas). El propio `fig4-4` traía dos comentarios documentando el workaround
a mano (`% sin paréntesis: pondrían u (1/v) = llamada`) — prueba de que la molestia era real y
recurrente.

🔧 **Arreglo (decisión de Alejandro, confirmada en el hilo):** una llamada es `nombre(` **pegado**;
con un espacio, `nombre (…)`, el `(` es un término aparte y —donde no quepa— error. Es
**adyacencia por columnas del token**: `lx.peek().col == idt.col + name.length()` (mismo renglón).
Los tokens ya llevaban `line`/`col`. Cambio contenido a `parseAtom` (case `T_IDENTIFIER`), **sin
plumbing de contexto** — vale igual en expresiones (`sqrt (n)` con espacio deja de ser llamada y
da error, que es prohibirla, como se quería).

**Churn: CERO** (golden `ok=69`). Se verificó que el corpus no tiene ni una llamada con espacio
(los hits del grep eran comentarios/cadenas). **Prueba concreta:** `fill_styles` reescrito con la
sintaxis natural (`{ x yp (x+0.13) (yp+0.1) }` en vez de `{ (x) (yp) … }`) da EPS **byte-idéntico**
salvo el `%%Title`. `fill_styles` (Alejandro) y `fig4-4` (comentarios corregidos + `(1/u)`→`1/u`)
quedaron más limpios; galería regenerada (incrusta el fuente). La referencia (ES+EN, §7 y §14) se
corrigió: el aviso viejo «un identificador seguido de `(` es una llamada / `dx (h+dy)` = `dx(h+dy)`»
era ahora FALSO; la regla nueva deja claro el uso correcto de paréntesis (encerrar solo lo que suma
o resta; las llamadas van pegadas; variables sueltas conviven con coordenadas entre paréntesis).
El `+`/`-` binario sigue necesitando `( )` (inherente al modelo de «el espacio separa»); no se tocó.

### Cerrado en la sesión del 2026-07-26 (rotación en elipses y arcos)

**Rotación de elipses y arcos CORRECTA y alineada con la transformación global.** Las primitivas ahora responden de manera natural a las directivas como `rotate`, manteniendo el principio agnóstico de que los ejes y dimensiones originales se deforman según la matriz afín para todos los tipos de formas. La rotación se extrae limpiamente y se resuelve por backend:
* **Motor (`matrix.cpp` / `matrix.h`)**: Se agregó `Matrix::get_rotation()` para extraer el ángulo directamente de la matriz global usando `atan2(M[1][0], M[0][0])`. El proyecto compila limpio tras ajustar la privacidad de este miembro.
* **EPS (`EPSDisplay.cpp`)**: Se modificó el diccionario y el procedimiento PostScript `/ellipse` para que lea y aplique el ángulo en tiempo de pintado. `EPSDisplay::arc` extrae este valor con `get_rotation()` y lo imprime en el macro.
* **SVG (`SVGDisplay.cpp`)**: Transformación exacta de los puntos de control. Las coordenadas de inicio y fin se calculan primero sobre los ejes originales alineados y luego pasan por `mt.transform()` (reaccionando bien bajo reflexión y sesgo). El ángulo resultante se emite en la propiedad nativa `x-axis-rotation` (tercer parámetro) del comando `A`.
* **PDF (`PDFDisplay.cpp`)**: Se reescribió `arc_bezier` para que la aproximación de libharu acepte la matriz global `mt`. La curva se calcula en el espacio base y cada punto de control se transforma limpiamente antes de enviarlo al PDF, garantizando deformaciones arbitrarias exactas.
**Churn masivo esperado y bendecido (corpus intacto a `ok=69`).** Al tocar el prólogo de EPS y las coordenadas resultantes de PDF y SVG, hubo que regenerar la red. Se ejecutó `test/run.sh capture` para los goldens y `test/run.sh images` para los renders de prueba. La validación corrió sin errores.
**Cobertura visual:** Verificado con `examples/elipse.mg`; al inyectar un `rotate 90`, la elipse obedece a la directiva global y se alinea de forma exacta con primitivas envolventes como `rectangle`. Ese ejemplo era de prueba y se eliminó pero se agregó una elipse al ejemplo rpstest que ahora ilustra la rotación de elipses.

### Cerrado en la sesión del 2026-07-27 (arcos elípticos, y la rotación de arcos/elipses de verdad)

⚠️ **Corrige la entrada anterior.** La sesión del 2026-07-26 dio la rotación por «CORRECTA y
alineada» en los tres backends. Lo era **solo en PDF**. En EPS y SVG el cambio fue un refactor
puro —`atan2(mt.M[1][0], mt.M[0][0])*180/M_PI` → `mt.get_rotation()`, matemática byte-idéntica—,
así que PDF quedó exacto y los otros dos con la aproximación vieja: **los backends discrepaban**,
y el golden lo bendijo porque es autorreferencial y la Capa 3 solo mira conteo de texto y paths
de un segmento. Ninguna compuerta compara *geometría* entre backends.

🔎 **Una sola causa raíz para tres síntomas.** Los backends recibían el arco DESCOMPUESTO (centro,
radios, `from`/`to`) y reparaban esa descomposición en dispositivo con reglas ad-hoc. Esa
descomposición **no es cerrada bajo afinidad**. De ahí salían:

1. **Elipse rotada mal.** `transform_radii` daba las normas de columna, que son los semidiámetros
   **conjugados**, no los **ejes** — solo coinciden cuando u⊥v. Medido en `rpstest` (copia 1 del
   caso 3): EPS/SVG decían `20.888 × 13.049 @ 47.27°` cuando la verdad es `21.757 × 11.541 @ 36.77°`
   (13% de error en el eje menor, 10.5° en el ángulo). Caso extremo: con `scale 2 1` seguido de
   `rotate 45` sobre un círculo unitario, las dos normas salen iguales y EPS dibujaba **un círculo
   donde va una elipse 2:1**.
2. **Arco circular girado perdía el giro.** Con `w==h` se tomaba la rama del operador `arc` nativo
   de PostScript, que **no tiene parámetro de rotación**. La antena de `lib/satellite.mg` no giraba.
3. **Arco reflejado se volvía disco.** Los tres bloques de «corrección de signo» hacían
   `ea = sa + endAng`, tratando el ángulo final ABSOLUTO como si fuera el BARRIDO: la antena
   (`from=190, to=350`, barrido 160°) salía `370 720` = **350°**, casi el círculo entero. Nunca se
   había visto porque **todo el corpus eran círculos completos**, donde 360 = 360 por ambos caminos.

🔧 **Arreglo: dejar de reparar la descomposición.** `Matrix::ellipse_frame` devuelve el centro y los
dos semidiámetros conjugados u,v — la forma `P(t) = C + u·cos t + v·sin t`, que **sí** es cerrada
bajo afinidad. Con eso:

* **EPS** — el proc del prólogo pasa a recibir la **matriz** (`[ux uy vx vy Cx Cy] mgarc`) y traza el
  arco UNITARIO con los ángulos **originales, intactos**; `concat` hace el resto. Los tres bloques de
  signo **desaparecen** en vez de arreglarse. `savematrix setmatrix` ya protegía el grosor de línea.
  Se conserva un atajo al `arc` nativo cuando la matriz es escala uniforme sin rotación ni reflejo
  (casi todos los círculos del corpus no mueven un byte).
* **SVG** — único que no puede recibir una matriz (`A` exige rx/ry/rotación), así que aquí sí se
  DECIDEN los ejes, con el SVD 2×2 en forma cerrada (`Matrix::ellipse_axes`). Las banderas salen del
  barrido real y del signo del determinante, no de ángulos «corregidos».
* **PDF** — ya era correcto; se unificó `arc_bezier` al mismo marco y se arregló el **bounding box**
  (eran las normas de columna; ahora `hypot(ux,vx)`, `hypot(uy,vy)`, exacto).
* Mueren `transform_radii` y `get_rotation`: sin usos, y su comentario («un círculo sigue siendo
  círculo bajo isometría+rotación») era precisamente la premisa falsa.

**`arc` acepta por fin `rx`/`ry`** (§4.5 los documentaba desde siempre; no había cliente hasta
`orbita_polar`). Un resolvedor compartido `resolveRadii` atiende `circle`/`ellipse`/`arc`, con el
nombre del radio opcional como promete §4.1, y **un radio ausente es error** en vez de default 1.
`emitArcMarkers` pasaba `r, r`; su cuerpo ya sabía de elipses (la tangente es la derivada
paramétrica), solo el sitio de llamada lo estrangulaba.

📐 **Cómo se verificó, que es la parte que faltó la vez pasada.** Se rasterizó el corpus entero
(25 ejemplos × 3 backends) ANTES y DESPUÉS y se comparó píxel a píxel con `compare -metric AE`:
* **PDF: cero cambios en los 25** — el refactor es exactamente equivalente, y PDF sigue de referencia.
* Cambian 4 EPS y 6 SVG. De ellos, `primitives` y `tiro_parabolico` solo mueven `largeArcFlag` de 1
  a 0 en arcos de **exactamente 180°**, donde esa bandera es indiferente: se muestrearon los puntos
  medios de los arcos y son idénticos. `orbita_polar` y `gravitacion_orbita` cambian a la forma
  canónica (`96.378 175.748 @ 15` → `175.748 96.378 @ -75`, con el eje mayor primero): se muestrearon
  ambas parametrizaciones y la desviación máxima es **0.000000 pt**. Lo demás son `rpstest` y el
  satélite, que estaban rotos.
* Y el número que cierra el caso: SVG emite hoy `21.757 11.5411 @ 36.7585` para la elipse de
  `rpstest`, **exactamente** lo que se había extraído de las Béziers del PDF. Los tres coinciden.

🕸️ **Cobertura nueva.** El corpus no ejercitaba ni un arco **parcial** girado, que es el hueco por el
que se coló el bug del barrido; solo lo hacía un `test_sat.mg` efímero. Se dobló en `rpstest`
(`arc(.3, .15, from=200, to=340)` dentro de `Cuadro`), que ya hace el giro acumulado y llega a
matrices que reflejan: los 13 arcos conservan `200 340`, antes las copias reflejadas salían `380 720`.
**`check` → `ok=69 fail=0 psfail=0 c3fail=0 imgfail=0 errfail=0 galfail=0`**; re-bendecido y
`images` regenerado (4 SVG publicados + galería).

**Pendiente propuesto, NO hecho:** una «Capa 4» que rasterice EPS y PDF con `gs` y los compare. Es la
única que caza esta clase —discrepancia de geometría entre backends— y en esta sesión se usó a mano.

**Añadido en la misma sesión: la invariante (c) de Capa 3 — paridad GEOMÉTRICA de arcos.**
`tools/arcparity.py`, invocada desde `test/run.sh`. Muestrea cada arco del EPS y exige que SVG
y PDF contengan una curva que pase por esos puntos, más el conteo de comandos `A` del SVG (un
arco de 360° son DOS, porque SVG no admite el completo). Los tres backends comparten espacio de
dispositivo —el volteo de SVG vive en el `<g transform="scale(1,-1)">`, fuera de las coordenadas
del path—, así que se comparan coordenadas directas sin normalizar.

⚠️ **Es la única compuerta sin escapatoria por bendición, y esa es toda su razón de ser.** Las
demás comparan contra un golden, pero el flujo normal tras tocar el motor es **re-bendecir**, y
ahí un cambio equivocado se bendice solo — que es exactamente lo que pasó esta vez. Verificado
reintroduciendo el bug a propósito (`ellipse_axes` devolviendo normas de columna): tras
`capture`, el golden da **`ok=69 fail=0`** —el bug queda bendecido e invisible— mientras
arcparity reporta **12.540 pt** de desviación en SVG contra 0.006 del PDF. Cableada, `capture`
da `c3fail=1`.

⚠️ **Entran los TRES backends, no dos.** Durante todo el bug EPS y SVG **coincidían entre sí** y
los dos estaban mal (ambos derivaban los ejes de las normas de columna). El PDF es la tercera
opinión independiente porque no decide ejes ni ángulos: transforma puntos de control de Bézier.
Una compuerta EPS-vs-SVG habría dado verde de principio a fin.

**Dos tolerancias, ninguna a ojo** (se calibraron midiendo, y el primer intento dio 4 falsos
positivos que resultaron ser cosas reales):
* El PDF **no dibuja arcos**, los aproxima con Béziers cúbicas de 90°, cuyo error radial máximo
  conocido es ≈2.7e-4·R. Medido en el corpus: 0.029 pt para R=113 y 0.041 para R=176 — justo esa
  cota. Por eso la tolerancia **escala con el radio** (`max(0.02, 1e-3·R)`); una constante o se
  pasa de laxa en las figuras grandes o da falsos positivos en ellas.
* Los 4 falsos positivos restantes eran todos arcos de **exactamente 180°** (las mitades en que
  SVG parte un arco de 360°). Ahí la conversión extremos→centro está malísimamente condicionada:
  `SVGDisplay` imprime 6 cifras significativas, así que un extremo real de 155.9055 sale
  155.906, y ese error de 5e-4 pt se **amplifica ~150×** hasta correr el centro 0.075 pt. Es
  artefacto de precisión de impresión, no desacuerdo entre backends, así que en la vecindad
  degenerada (`lam > 0.999`) el centro se fija en el punto medio de la cuerda. Resultado: **cero
  falsos positivos en los 25 ejemplos**. *(Subir la precisión de impresión del SVG lo quitaría de
  raíz, pero movería TODOS los goldens SVG y todo `docs/img` por 0.075 pt invisibles — no se hizo.)*

**Alcance declarado:** solo arcos y elipses, que es donde vivió el bug. **No** cubre texto ni
tramado (los dos pendientes de `PENDIENTES.md`), a propósito: una compuerta que promete más de
lo que mide es peor que ninguna. Coste: la red completa sigue en ~3 s.

### Cerrado en la sesión del 2026-07-27 (bis) — marcadores paramétricos y `place` sobre elipse

Tres cosas de `orbita_polar`, la figura que motivó todo el trabajo de arcos de esta sesión.

**1. Las órbitas no eran concéntricas con la Tierra.** `rotate` gira el plano entero, así que
con el centro en `{0 earth_y}` el propio centro se desplazaba: las dos elipses quedaban
descentradas ~2.6 mm hacia lados opuestos (centros en x = 166.41 y 173.75 con la Tierra en
170.08). Se lleva el origen al centro con `translate 0 earth_y` y las elipses se colocan en
`{0 0}`. Ahora ambas comparten centro exacto con el globo.

**2. `marker_at`: marcadores en posiciones PARAMÉTRICAS de `arc`/`ellipse`/`circle`.** Resuelve
«flechas de sentido sobre la órbita». La maquinaria ya existía y solo estaba atada a los
extremos: `emitArcMarkers` calcula `centro + (rx·cosθ, ry·sinθ)` con la tangente paramétrica
correcta —y esta misma sesión se le arregló el `rx≠ry`—, pero solo se le llamaba con `from`/`to`.
Además `ellipse` y `circle` no admitían marcador alguno (§4.5 lo decía: «esperan una figura que
los pida»; ya la hay).

Los ángulos van en **grados**, el mismo espacio que `from`/`to` de la primitiva. Se descartó
`t ∈ [0,1]`: `point_at`/`sample` lo recorren por LONGITUD DE ARCO, y tener dos parametrizaciones
en un mismo comando confunde. ⚠️ El costo aceptado: en una elipse los ángulos paramétricos no
quedan equiespaciados, así que repartir marcadores «parejos» pedirá un `marker_count` por
longitud de arco cuando una figura lo pida. Con `marker_at` presente, `marker=` pasa a ser solo
la FORMA (si no, pedir una flecha a media órbita daría dos más en las puntas sin haberlas pedido).

**3. `place` sobre elipse, con `at=`: la struct COMPLETA sobre la órbita.** Resuelve «colocar el
satélite sin calcular la posición a mano».

🔎 **Se probó primero el marcador y NO sirve** —vale la pena registrarlo—: un marcador es
**monocromo por construcción** (recibe `marker_color`/`marker_fill` y estampa *una* forma), y el
satélite es un ícono de varios colores. Además `markerShapeFromStruct` solo extrae `GI_POLYLINE`
y `GI_POLYGON`, y `Satellite` está hecho de `rectangle` y `arc` → «marcador desconocido». Aunque
se arreglara la extracción, saldría aplastado a un color. El primitivo correcto es `place`, que
emite `g.structure()` y conserva todo.

`place` ya colocaba structs sobre un arco (el `ARCST` de V1), pero **solo circular**: `r` era un
único radio — exactamente la carencia que tenía `arc` (§4.5) hasta esta mañana. Se añadió
`rx`/`ry`, y `at=[…]` con los ángulos. Con `at` presente el locus **NO se dibuja**, siguiendo el
precedente del locus de path (que tampoco dibuja el path); el locus de línea y el de arco sí lo
dibujan, así que la regla ya estaba dividida.

🐛 **Bug encontrado de paso en `StructureArc`:** orientaba con `angf ± 90`, que es la tangente de
un **círculo**. Sobre una elipse deja la struct torcida. Ahora usa la tangente paramétrica real,
`atan2(dir·ry·cosθ, −dir·rx·sinθ)` (idéntica a la de `emitArcMarkers`). **Es la misma clase de
error que las normas de columna de la entrada anterior**: una fórmula válida solo en el caso
isótropo, aplicada al anisótropo. No se veía porque nadie había puesto una struct sobre una
elipse. Vale la pena buscar más casos de esta familia antes de que los encuentre una figura.

**Churn: CERO** (`ok=69`, atributos nuevos; ninguna salida existente se mueve).

**Pendiente en la figura:** ocultar la mitad trasera de la órbita. NO hace falta motor booleano
—órbita y globo son concéntricos, así que los cruces con el limbo salen de `|u·cosθ + v·sinθ|² = R²`
en forma cerrada—, pero el lenguaje **no tiene trigonometría**: no hay `sin`, `cos` ni `sqrt` en
el evaluador. O lo calcula el compilador, o el evaluador gana funciones matemáticas.

### Cerrado en la sesión del 2026-07-27 (ter) — `orbita_polar` terminada y EN EL CORPUS

La figura que motivó todo el trabajo de arcos queda cerrada, y con ella el `plan_orbita_polar`.

**1. Oclusión de la mitad trasera — cero compilador.** La entrada anterior cerró diciendo que
el lenguaje «no tiene trigonometría». Es falso: `sin`, `cos`, `tan`, `sqrt`, `abs`, `atan2`,
`exp`, `ln`, `mod` y `pi` llevan tiempo en `include/ast.h:137,228-243`. Con eso la oclusión se
resuelve **dentro del `.mg`**, sin tocar `bin/mg` y sin motor booleano: es profundidad, no
conjuntos en 2-D —una intersección de contornos no sabe qué mitad está detrás—, y con órbita y
globo concéntricos sale en forma cerrada. En el marco propio `P(t) = (a·cos t, b·sin t)` la
mitad lejana es `t ∈ (90°, 270°)`, y el cruce con el limbo sale de `|P(t)|² = R²`:

    a² + (b² − a²)·sin²t = R²   →   sin t = √((R² − a²)/(b² − a²))

Oculto = lejano ∩ dentro del disco = `t ∈ (180−tc, 180+tc)`. Con `a=3.4, b=6.2, R=5` la raíz da
`√½` y **`tc = 45°` exacto**. No hay `asin`, pero `atan2(s, √(1−s²))` lo expresa. Cada órbita
pasa de `ellipse` a `arc` con el barrido recortado.

📐 **Los cortes caen sobre el limbo por CONSTRUCCIÓN, no por ajuste**: el disco del mapa es un
`circle(1)` de `lib/mapa_p30_n55.mg` escalado por `scale=earth_radius`, o sea exactamente el
mismo `R` y el mismo centro que la ecuación. Verificado rasterizando a 1600 px: el trazo termina
dentro del ancho de línea del limbo, sin muñón.

**2. Cuál mitad va detrás es MODELADO, no geometría.** La 2-D admite las dos; hay que elegir y
**mirarlo**. Se eligieron opuestas —la de +15° esconde su izquierda, la de −15° su derecha— para
que se lean como dos planos que se cruzan y no como copias de uno solo. Al recortar el barrido
hay que reponer los `marker_at`: el de 180° caía en la zona oculta, así que queda **una** flecha
por órbita y en lados opuestos (`[360]` y `[180]`). `marker_at` no valida rango, solo evalúa
`cos`/`sin`, por eso 360 es legítimo dentro del barrido 225→495.

**3. Segundo satélite, a `at=[270]`.** Se pidió «abajo, mirando a Bolivia». Proyectando Bolivia
(lat −17, lon −65) en la ortográfica del mapa (vista lat 30, lon −55) cae en `(−0.83, −3.62)`
desde el centro del globo, o sea **−102.9°**; el punto `t=270` de esa órbita está a **−105°**.
Es además el extremo inferior del eje mayor, así que su tangente es horizontal y el satélite
queda acostado, en contrapunto al de arriba. Se compararon 215/240/250/270 sobre el render:
`250` se va 10° al suroeste y `240` monta sobre el limbo. La descartada interesante es `215`
—delante del globo, a la altura de Bolivia—, que exhibe que la mitad cercana pasa por DELANTE.

**4. La figura entra al corpus: `ok=72`** (24 ejemplos × 3 backends). Hasta hoy no la vigilaba
ninguna compuerta pese a ser el **único cliente** de `arc(rx, ry)`, `marker_at` y
`place(..., rx/ry, at=)`; y es el primer ejemplo con arcos elípticos **girados**, justo lo que
mira la invariante (c) de la Capa 3. Se le hizo `docs/img/orbita_polar.svg` (entra a `imgfail`)
y tarjeta 23 de la galería. Encabezado a la convención de 2026-07-23, y comentarios reescritos:
explican lo que hace la figura, sin arqueología de versiones anteriores ni referencias `§n`.
La limpieza se verificó **byte a byte** contra el render aprobado, y regenerar `docs/img`
entero no movió ninguno de los otros 25 SVG publicados.

**Churn: CERO en el motor** — no se tocó una línea de C++ en toda la entrada.

**Addendum (misma sesión) — `docs/referencia.md` descongelada.** Alejandro levantó el freno
en cuanto la figura cerró. Lo que se le añadió es lo que hoy quedó *usable*, no una lista de
cambios: **recortar un arco** por una condición geométrica calculada en el propio `.mg` (§4,
con la receta de la órbita y la nota de que esto NO es álgebra de trayectos: la oclusión es
profundidad); **`asin`/`acos` vía `atan2`** (§7); **arcos y elipses bajo transformación**, que
ahora dan la elipse girada correcta en los tres backends, y el aviso de que **`rotate` gira el
plano, no la figura** —con el `translate`-al-centro como receta— (§9); y los **tres mapas del
mundo de `lib/`**, que estaban sin documentar desde el 2026-07-24 (§12). Los cuatro fragmentos
nuevos se compilaron antes de escribirlos, para que la referencia no publique código que no
corre.

**Addendum 2 (misma sesión) — la órbita ahora sale de kilómetros.** Al revisar la figura
terminada apareció que **el rótulo no cuadraba con la geometría**: con la Tierra en 5 unidades
(6371 km), el semieje `b=6.2` corresponde a ~1500 km de altura, no a los 800 que decía el
letrero. Venía de la lámina de internet en que se inspira, y sobrevivió porque nadie lo
calculó — exactamente el defecto que MG existe para no tener.

Arreglado **declarando la física, no el dibujo**: `orbit_km = 800` y `earth_km = 6371` fijan
`axis_y = earth_radius*(earth_km+orbit_km)/earth_km`, el semieje menor sale de un
`plane_angle` explícito, y el rótulo se arma con `str(orbit_km)`. Ahora el número que
gobierna la geometría **es** el que se imprime: no pueden divergir.

📐 **La prueba de que la figura está calculada y no ilustrada**: la corrección fueron **dos
declaraciones**. `tc` pasó solo de 45° a 56.8° (el tramo oculto de 90° a 113.6°), y los dos
satélites, las flechas y el rótulo se recolocaron sin tocarlos. En la lámina original habría
sido volver a borrar a mano la mitad tapada.

⚠️ **Cambia la LECTURA, y se aceptó a sabiendas:** a 800 km la órbita está apenas 12.6% sobre
la superficie, así que abraza el globo y lo que asoma fuera del disco son dos gajos cerca de
los polos. La versión anterior comunicaba mejor «dos planos orbitales que se cruzan», pero
mentía en la altura. Se prefirió la honesta. La alternativa que no se tomó —exagerar la
escala **a propósito y declarándolo**, como hacen los libros— queda registrada aquí.

Único ajuste a ojo de toda la sesión: el rótulo se acercó de x=−3.5 a −2.9, porque al bajar la
órbita se le había quedado lejos.

### Cerrado en la sesión del 2026-07-27 (quater) — las dos decisiones de semántica anisótropa

Cierra `plan_anisotropia.md`, y con él el hilo entero abierto esta mañana. **Cero líneas de
C++**: lo que faltaba no era arreglar nada, era **elegir** y escribirlo. Se registran también
las opciones descartadas, que es la parte que no sobrevive sola.

**A. El radio de un arco en la ruta log de `plot`.** La ruta lineal envuelve el contenido en
una matriz; la log no puede (el log no es afín) y remapea punto por punto, así que el radio
—miembro del `Arc`, no punto del path— no se transforma. El mismo `circle(0.5)` medido en dos
marcos de idénticas proporciones daba elipse de 11:1 por la ruta lineal y círculo de 14.17 pt
por la log.

**Decidido: la ruta log mapea POSICIONES, no formas.** Los tamaños quedan en coordenadas de la
página. Lo que convenció no fue que sea lo que ya hacía, sino que es la **generalización de una
regla existente**: esa ruta ya dejaba intactos `line_width`, la tipografía, el radio de
`dot`/`marker` y `hatch_gap` —el propio código lo llama «invariante físico gratis»—, y el radio
de `circle`/`arc`/`ellipse` y el `width` de `polybar` no eran la excepción sino el mismo caso
sin enunciar. Enunciada así deja de ser un accidente y pasa a ser algo que se puede enseñar.

🔎 **Las dos descartadas, con su razón:**
- **Linealizar el radio en el centro** (dibujar la elipse de la derivada local del mapa log)
  daba consistencia entre las dos rutas —el argumento fuerte a su favor—, pero es exacta solo
  en el límite infinitesimal: bajo un logaritmo una circunferencia de datos no es ni círculo ni
  elipse, es un huevo asimétrico. Habría producido una figura que *parece* calculada sin
  estarlo, que es justo lo que este proyecto no hace.
- **Rechazarlo como error** encajaba con la tradición de la casa (38 pruebas negativas: aquí se
  falla fuerte y claro), pero bajo la regla elegida el resultado **sí** tiene significado —un
  círculo trazado sobre la hoja, en la posición que le toca—, así que la guarda pagaría el
  costo de prohibir sin el beneficio de resolver.

⚠️ El footgun queda y se asume: `circle(0.5)` mide distinto según el eje sea lineal o log.
**Avisa la documentación, no el compilador** (`especificacion_mg.md` §13.7 y
`docs/referencia.md` §11, esta última con el ejemplo compilado y verificado por render).

**B. La dirección «out» de las marcas de eje.** `px = uy, py = -ux` calcula la perpendicular en
**mundo** y `physOut` la usa en **dispositivo**; bajo `stretch` no son la misma.

**Decidido: perpendicular en el PAPEL**, y no como preferencia sino como corolario de algo ya
decidido: `tick_size`, el grosor y la tipografía de una marca ya son físicos. Una marca de eje
es **mobiliario** —un trazo legible que sale del eje—, no un vector con sentido en el espacio de
datos; una longitud física en dirección derivada del mundo es un híbrido sin dueño.

**Sin cambio de código, a propósito.** Para que muerda hacen falta DOS cosas a la vez:
`world_window … stretch=true` a nivel de documento —que ningún ejemplo usa; los tres que dicen
`stretch` lo hacen en `fit`, otro mecanismo— y un eje **diagonal**, que no existe. Cuando
aparezca la figura, el arreglo es calcular la perpendicular después de pasar a dispositivo.

📌 **Lo que queda vivo de `plan_anisotropia.md`** no es el inventario de bugs sino «La firma»
(los cuatro tics: radio por un solo factor, ±90 para una perpendicular, atan2 sobre una
columna, ángulo de mundo usado como de dispositivo) y «Cómo cazar más». Es el documento a leer
antes de meter geometría nueva.

### Cerrado en la sesión del 2026-07-27 (quinquies) — la puerta de entrada: galería bilingüe y READMEs

Trabajo de **atracción de usuarios**, que es la condición 4 del 1.0 («uso real por gente que no
es el autor»). No es marketing: la condición pide retroalimentación sobre **ergonomía y
nombres**, y esa es la única que el autor no puede darse a sí mismo —ya sabe cómo se llaman las
cosas—. Todo lo de esta entrada existe para bajar el costo de que alguien de fuera llegue,
compile una figura y opine.

**1. La galería ahora son DOS páginas**, `docs/galeria.html` (es) y `docs/gallery.html` (en),
generadas por el mismo `tools/galeria.py` y enlazadas entre sí; cada README apunta a la de su
idioma. El README ya venía en dos idiomas desde antes: la galería es la que de verdad se ve
primero, y estaba solo en español.

⚠️ **El inglés NO puede salir de los `.mg`.** Van comentados en español por política del
proyecto, y meterles un segundo encabezado los volvería ilegibles —además de romper la
convención de encabezado que publica la galería—. Las traducciones viven en una tabla `TRAD`
dentro del tool. **Un ejemplo sin traducir igual aparece**, en español y con aviso por stderr:
la regla de que un ejemplo nuevo salga solo pesa más que la uniformidad del idioma.

**2. El orden es ahora editorial, y ese era un bug de presentación real.** La página abría con
`quickstart` y las dos figuras más vistosas —`orbita_polar` y `gravitacion_orbita`— caían al
**pie**, en «Más ejemplos», porque nadie las había puesto en un grupo. Ahora abre con «Figuras
que se calculan solas» y los catálogos van al final. Lo que no se liste sigue cayendo al pie:
esa es la válvula que mantiene la propiedad de «aparece solo».

**3. Caja «Pruébalo» y aviso de beta en el encabezado.** Tres comandos —clonar, `make`,
compilar una figura— porque la tarjeta enseñaba el código y ahí se acababa el camino. El aviso
de beta está a propósito: baja el costo de las primeras roturas y ahuyenta a quien quiere algo
estable hoy, que no es el usuario que esta etapa necesita.

**4. Los READMEs decían «las 21 figuras de ejemplo» y son 23.** La cara pública, rancia, y
**nada la vigilaba** — la misma clase de bug que inventó `galfail`, un escalón más afuera. Se
quitó el número en vez de corregirlo: un número en prosa no tiene compuerta que lo cuide, y lo
que no puede envejecer no hay que vigilarlo.

**5. Sección «¿Por qué no TikZ o matplotlib?»** en ambos READMEs. Un usuario adopta una
herramienta cuando entiende qué se **niega** a hacer, y no lo decía ningún documento. Dice sin
adornos que si tu figura es una vista de un conjunto de datos uses matplotlib, y reconoce que
TikZ ya está instalado en todas partes y tiene una década de paquetes que MG no tendrá. La
ventaja que sí ofrece va medida, no adjetivada: **el corpus completo, 24 figuras, compila en
90 ms**.

**6. Canal de retroalimentación explícito** (el gestor de issues) en el punto que lo pedía: el
README ya decía «buscamos tu opinión» sin decir dónde. Con la nota de que un «no encontré cómo
hacer X» sirve tanto como un error — que es, literalmente, la evidencia que la condición 4
necesita recoger.

La compuerta `galfail` mira ahora **las dos** páginas (`galeria.py --check` compara ambas), sin
cambios en `test/run.sh` más allá de los comentarios y del texto del mensaje.

### Cerrado en la sesión del 2026-07-27 (sexies) — Windows: `mg.exe` cruzado con MinGW

Es el hueco que más usuarios cuesta y el que menos se puede tapar con documentación: en
Windows no hay compilador de sistema, así que o se reparte un `.exe` hecho o no hay usuarios.
Se cruza desde Linux con MinGW-w64 —como la versión de 1999— y **no hace falta una máquina
Windows para producirlo**, solo para probarlo.

🔎 **La auditoría de portabilidad salió mucho mejor de lo temido**, y conviene registrar por
qué, porque es mérito de decisiones viejas:
- **La fuente matemática va EMBEBIDA** (`g_lmmath_ttf` por memoria, `font_lmmath_eps.h` como
  texto): no se abre ningún archivo en tiempo de ejecución, que era el obstáculo grande.
- **Los tres backends abren con `"wb"`** (`EPSDisplay.cpp:187`, `SVGDisplay.cpp:195`, y
  libharu en `hpdf_streams.c:937`). En Windows el modo texto traduce `\n` a `\r\n`: habría
  corrompido el PDF en silencio. Ya estaba bien.
- **Cero POSIX en el código propio**: ni `unistd.h`, ni `dirent`, ni `getopt`, ni `fork`.
- `libharu` es C portable y va vendorizado; solo `zlib` es externa (`libz-mingw-w64-dev`).

Tres cambios, todos chicos:

**1. `src/lexer.l` gana `nounistd never-interactive`.** El único `#include <unistd.h>` del
árbol lo ponía flex. El lexer nunca lee de una terminal —siempre de un `.mg`—, así que la
prueba de interactividad no hace falta en ninguna plataforma. Verificado antes de tocarlo que
el `lexv3.cpp` committeado sale de este mismo flex 2.6.4 (el diff eran solo los `#line`).

**2. `Makefile`: `make CROSS=x86_64-w64-mingw32` → `bin/mg.exe`.** Fija CXX/CC/AR con el
prefijo, añade el sufijo `.exe` a los targets y enlaza **estático**
(`-static -static-libgcc -static-libstdc++`): el `.exe` tiene que correr recién
descomprimido, sin DLLs de gcc al lado. El build nativo no cambia (`EXE` vacío).

**3. `include` junto al `.exe`** (`parserv3.cpp`, `exeDir`, bajo `#ifdef _WIN32`). En Unix la
biblioteca §15 vive en una ruta horneada con `-DMG_LIBDIR`; en Windows no hay `make install`
ni `/usr/local`, el reparto es un `.zip` que el usuario descomprime donde quiera. Se añade
`<dir del .exe>/lib/` como último candidato, y de paso se reconocen `C:\…` y `\\servidor`
como rutas absolutas. Fuera de Windows no se compila ni una línea de esto, así que el golden
no se entera (`ok=72`).

**4. `.github/workflows/release.yml`** — no había CI de ninguna clase. Con una etiqueta `v*`
compila Linux, macOS y Windows, y publica los tres paquetes con `lib/`, los ejemplos y la
referencia dentro.

⚠️ **Lo importante del workflow no es que compile, es que EJECUTA el `.exe`.** El binario
cruzado no se corre en la máquina que lo produce, así que hay un trabajo `smoke-windows` en
`windows-latest` que lo desempaca y dibuja en SVG, PDF y EPS; comprueba que el PDF siga
empezando con `%PDF` (si Windows hubiera traducido saltos de línea, no lo haría) y compila un
`.mg` con **`include "satellite.mg"` sin ruta**, que es el único camino de código exclusivo
de Windows y el único que no se puede probar en otra plataforma. Publicar un binario que
nadie ejecutó sería justo el tipo de hueco que este proyecto cierra con compuertas.

📌 En Linux `test/run.sh capture` **bloquea** el release (cinco de las seis compuertas: todas
menos el golden por bytes, que necesita una red que no está en git). En macOS es informativo
a propósito: una diferencia de último dígito en `libm` movería los renders publicados sin que
nada esté roto.

**Addendum (misma sesión) — el `.exe` compilado y EJECUTADO de verdad.** Alejandro instaló el
toolchain, así que el build cruzado dejó de ser teoría. Cuatro hallazgos, tres arreglados aquí:

🐛 **`M_PI` no existe en MinGW.** `matrix.h:23` (`constexpr double deg2rad = M_PI / 180`) no
compila: `M_PI` no es de C++ estándar, glibc la da siempre y MinGW solo con
`_USE_MATH_DEFINES` definida **antes** de `<cmath>`. Va en `CPPFLAGS`, no en un header, porque
tiene que llegar antes que cualquier inclusión. Inocua fuera de Windows.

🐛 **Los objetos del cruce no pueden compartir `obj/` con los del build nativo.** Alternar
`make` y `make CROSS=…` dejaba objetos ELF que el `ld` de MinGW no lee, y el error que salía
—«undefined reference to kExtraTextGlyphs»— no dice nada de eso: parece un símbolo perdido.
El cruce usa ahora `obj-win/`, y `clean` borra los dos.

📏 **`-s` al enlazar: 15.6 MB → 1.9 MB.** El `.exe` se descarga, no se depura.

🧹 **Cinco warnings que clang no da y g++ sí** (`-Wmisleading-indentation` ×2,
`-Wsign-compare` ×3). Se arreglaron para conservar el «compilación limpia» del Code style
ahora que hay dos toolchains: dos `if` que compartían renglón, y tres comparaciones de un
`int` con `string::npos` que funcionaban por la truncación a −1 y ahora lo dicen con un cast
explícito. Cero cambio de salida (`ok=72`).

📐 **Y la medición que no se podía hacer de otro modo: la salida de Windows contra la de
Linux, byte a byte** (24 ejemplos × 3 formatos, el `.exe` corriendo bajo wine, normalizando el
`%%Title` del EPS igual que el harness):

    idénticos 68 / 72   ·   difieren 4   ·   errores 0

**Los 24 SVG y los 24 EPS son idénticos.** Difieren **cuatro PDF**, y los cuatro por lo mismo:
la cadena `0.000000000000000061232` (Linux) contra `0.00000000000000006123` (Windows). Es
`cos(90°)` —el cero numérico de `PDFDisplay::deviceRotate`, `PDFDisplay.cpp:736`— y los cuatro
archivos son justamente los que rotan un rótulo de eje 90°. La diferencia no es nuestra: la
pone `HPDF_FToA` (`hpdf_utils.c:182`), que deriva el número de decimales de un `log10()` y
emite los dígitos con `modff`, todo en **float**, así que hereda la libm de la plataforma
—glibc da 21 decimales, msvcrt 20—.

✅ **Verificado también el único código exclusivo de Windows**, `exeDir()`: con un `.mg` de
biblioteca que existe **solo** junto al `.exe` (no en `/usr/local`, para que wine no lo
resolviera por el camino de Unix y diera un falso positivo), invocado desde otro directorio de
trabajo. Resuelve.

**Addendum 2 (misma sesión) — cerrada la última diferencia: 72/72.** Se redondea a cero el
coseno de un giro recto en `PDFDisplay::deviceRotate`. **No es cosmética y conviene entender
por qué:** la matriz exacta de un cuarto de vuelta es `0 1 -1 0`, y `cos(pi/2) = 6.123e-17` es
el error de punto flotante — redondearlo **acerca** al valor verdadero, no se aleja de él. El
umbral (1e-12) es holgado: los ceros de verdad quedan a ~1e-16 y un giro de 1e-10 grados no
existe en ninguna figura.

Movió exactamente los cuatro goldens previstos (`fig1`, `fig_polybar`, `franck_condon`,
`quickstart`, todos en PDF: son los que rotan un rótulo de eje 90°), sin un solo cambio
visible. Re-medido después:

    Windows vs Linux, 24 ejemplos × 3 formatos:  idénticos 72 / 72

📌 **Y se vuelve invariante, no anécdota.** El workflow gana un paso que compila el corpus con
`mg.exe` sobre Windows y lo compara byte a byte contra la salida de Linux del mismo commit.
Una medición que nadie vuelve a hacer se pudre —es la lección que ya habían enseñado
`docs/img` y la galería—, y esta en particular es de las que se rompen en silencio: cualquier
`printf` que herede la libm de la plataforma reintroduce la divergencia sin mover un solo
golden, porque el golden se genera **en una sola** plataforma.

🔎 Detalle de implementación que importa: las salidas de referencia se escriben con una ruta
**relativa idéntica** en las dos plataformas. El EPS lleva su ruta de salida dentro
(`%%Title`), así que compararlas exige o normalizar —lo que hace `test/run.sh`— o, más simple,
pedirle a los dos la misma ruta.

### Cerrado en la sesión del 2026-07-27 (septies) — fuera el front-end V1: `main` es solo V3

Pregunta de Alejandro: ¿está cerrado el traductor, para poder borrar `Parser.cpp` y el lexer
viejo? Sí, y el borrado destapó de paso una red de pruebas podrida.

**El traductor está cerrado** (`tools/mg1to2.py`, `ok=14`) y **no depende del front-end V1 en
tiempo de ejecución**: es Python, lee los `.mg` de V1 directamente. `Parser.cpp` y `mgpp.l`
servían de **referencia de la semántica V1** —la aridad exacta de cada comando de dos letras—,
y esa referencia sigue viva en la rama: `git show v1-legacy:src/Parser.cpp`.

Borrados de `main`: `src/Parser.cpp`, `src/mgpp.l`, `include/Parser.h`, `include/MGLexer.h`,
`include/mgpp_tab.h`. Ninguno se compilaba desde el cutover —estaban en `SRCS` del Makefile
pero fuera de `V3_ENGINE_OBJS`, o sea que la lista *decía* que se construían y no era cierto,
que es peor que no estar—. Con ellos se fue **`include/font_cmmi.h`**, 1339 líneas de fuente
Type42 empotrada, **huérfano** desde la migración a LM Math del 2026-07-20: no lo incluía
nadie (`cmmiUnicode()` es otra cosa, vive en `text_parser.h` y sí se usa). Build limpio y
`ok=72` sin mover un byte.

🔎 **Y el hallazgo lateral, que vale más que el borrado: `test/golden_translator` llevaba SEIS
DÍAS rancia.** `bash test/run_translator.sh check` daba `ok=9 fail=5`. Ninguno era un fallo del
traductor —que no se ha tocado—: los cinco eran cambios **intencionales del motor** que nadie
había bendecido ahí, porque esa red se genera con «traductor + `bin/mg`» y la mueve cualquier
cambio del compilador:
- `fig2-3` y `primitives`: **toda** línea distinta era un arco. La bandera *large-arc* de un
  arco de 180° cambió con la reconstrucción de arcos de hoy; en 180° la elección es ambigua
  (las dos mitades miden lo mismo) y el dibujo es idéntico.
- `fig6-1` y `fig4-10`: un `<text>` con **un espacio suelto** que el golden traía y la salida
  de hoy ya no emite — lo quitó `plan_text_space`. Mejora, no regresión.
- `fig6-10`: idéntico ya, fallaba por arrastre de la captura vieja.

📌 **La causa raíz es estructural: `test/run.sh` no corre el harness del traductor**, así que no
existe el reflejo de re-bendecirlo. Es exactamente la lección de `docs/img` y de la galería, un
escalón más afuera. Queda documentado en `CLAUDE.md` («Build and test») como paso obligatorio
tras tocar el motor. **Si se repite, la respuesta es una séptima compuerta**, no otra nota.

**Addendum 3 (misma sesión) — la primera corrida de CI: el proyecto NO compilaba en macOS.**
La corrida en seco del workflow (`workflow_dispatch`, que valida sin publicar) dio Linux ✓ y
el cruce a Windows ✓, y **macOS falló al enlazar**: `ld: unknown options: --gc-sections`. El
`ld` de Apple no conoce esa opción de GNU ld —allí se llama `-dead_strip`—, así que
`LDFLAGS` se elige ahora por `uname -s`.

📌 **Es un defecto viejo que nadie podía ver**: `-Wl,--gc-sections` lleva en el Makefile desde
antes de esta sesión, y el proyecto nunca se había compilado en un Mac. El README ofrecía
macOS y no era cierto. La primera corrida de CI se pagó sola.

**Addendum 4 (misma sesión) — macOS pasa a BLOQUEAR, y se le mide la salida.** Alejandro
mencionó que ya no tiene la MacBook, y eso invierte una decisión del workflow: las pruebas de
macOS estaban como **informativas** (`continue-on-error`), razonando que una diferencia de
último dígito en `libm` movería los renders publicados sin que nada estuviera roto. Con un Mac
a la mano era lo prudente —si algo se veía raro, se miraba—. **Sin Mac es al revés: ese paso es
el único ojo que queda sobre la plataforma, y estaba sin poder decir que no.**

Dos cambios, y el segundo desactiva el miedo que justificaba el primero:
1. `test/run.sh capture` **bloquea en las dos plataformas nativas**, no solo en Linux.
2. **`smoke-macos`**, espejo de `smoke-windows`: desempaca el paquete, dibuja el corpus con el
   binario que se va a publicar y **exige que la salida sea idéntica byte a byte a la de
   Linux**. Si lo es —como resultó con Windows, 72/72—, no hay diferencias de `libm` que temer
   y bloquear es gratis. Si no lo es, eso es justo lo que hay que saber sin un Mac: que un
   usuario de Mac obtiene archivos distintos de los de la galería publicada.

⚠️ Puede fallar en la próxima corrida, y **si falla es un hallazgo, no un accidente**.
No se prueba ahí el `include` sin ruta: el rescate «biblioteca junto al ejecutable» es código
`#ifdef _WIN32`, y en Unix la ruta la hornea `-DMG_LIBDIR`.

📌 Anotado sin resolver: `macos-latest` es **arm64**, así que el paquete sirve a Apple Silicon
y no a los Mac Intel. Se dice en la tabla de la página de descarga; añadir un segundo binario
x86_64 espera a que alguien lo pida.

**Addendum 5 (misma sesión) — el primer CI en un Mac: 24 fallos falsos por el `wc` de BSD.**
Con el enlace ya arreglado, macOS compiló y **falló en las pruebas**: `c3fail=24`. No era el
compilador. El mensaje traía la pista completa:

    C3FAIL quickstart (texto EPS/SVG/PDF = 18/      18/18: un backend omite texto)

Los tres conteos son **iguales** —18, 18 y 18—, pero el de SVG viene con espacios delante. El
`wc -l` de BSD (macOS) **rellena a la izquierda** y el de GNU no, y la invariante compara
*cadenas*: `"18" != "      18"`. Se arregla con un `tr -d ' '`.

📌 **Lo que sí dice esa corrida, y es la buena noticia: `imgfail: 0` en macOS.** O sea que los
renders publicados en `docs/img` salieron **byte a byte idénticos** compilados en un Mac. El
miedo que motivó poner esas pruebas como informativas —que `libm` moviera el último dígito—
queda desmentido antes incluso de que corra `smoke-macos`.

🧹 **Y los cuatro avisos de la corrida**, ninguno de código propio: tres «Node.js 20 is
deprecated» de `actions/checkout@v4` y uno de `brew`. Se subieron las acciones a sus mayores
vigentes (checkout v5, upload-artifact v7, download-artifact v8, gh-release v3) y **se eliminó
el `brew install flex`**: macOS ya trae flex y `src/lexv3.cpp` va committeado, así que ese paso
solo aportaba el aviso. Aparte, `HARU_CFLAGS` gana `-Wno-deprecated-declarations`: libharu es
código vendorizado que no mantenemos y suelta 11 avisos de `sprintf` en clang; 11 líneas de
ruido ajeno tapan un aviso propio cuando aparezca.

**Addendum 6 (misma sesión) — la MISMA lección por tercera vez, y ahora en un solo sitio.**
`smoke-macos` dio **71 de 72** y el diagnóstico recién añadido dijo exactamente qué:

    Linux:  [-28.3465  6.88426e-15  -5.50741e-15  -8.85827  170.079 124.016]
    macOS:  [-28.3465  6.62074e-15  -5.87255e-15  -8.85827  170.079 124.016]

Los dos números que difieren son **ceros numéricos** junto a valores de 28: es la matriz de
`rpstest`, el único ejemplo con **rotación acumulada** (`repeat(..., transform=rotate(60))`
compuesto hasta 180°), donde los términos fuera de la diagonal deberían ser 0 exacto. Cada
`libm` deja un residuo distinto y `%g` lo imprime con seis cifras.

Es la tercera aparición del mismo fenómeno en el día —`cos(90°)` en el PDF, y antes la
familia entera de `plan_anisotropia`—, así que en vez de un tercer parche ad-hoc se puso
**`snap_zero(v, ref)` en `include/matrix.h`**, con la explicación completa en un solo lugar, y
lo usan los dos sitios: `PDFDisplay::deviceRotate` (referencia 1, porque `c` y `s` viven en
[-1,1]) y la emisión del arco en `EPSDisplay` (referencia = la escala de la matriz, la misma
`s` que ya usaba la prueba `plain` de ahí arriba). El umbral es **relativo**, no absoluto: en
coordenadas de dispositivo un 1e-9 legítimo existe.

Movió un solo golden —`rpstest.eps`, que ahora emite el `[-28.3465 0 0 -8.85827 …]` que
corresponde a media vuelta— y **nada más**. Re-medido después del cambio:

    Windows vs Linux:  idénticos 72 / 72

📌 Con esto las **tres** plataformas deberían coincidir byte a byte. Lo dirá la corrida, que es
precisamente el punto de tener la compuerta.

### Cerrado en la sesión del 2026-07-27 (octies) — la punta de la flecha, y dos bugs que destapó

Alejandro vio que la flecha gorda de rotación de `orbita_polar` salía **chata** y supuso que era
el ancla. No lo era —el ancla está en la punta, verificado en los bytes del SVG: el marcador
arranca exactamente en el extremo del arco—. Eran **tres defectos encadenados**, ninguno visible
a 0.2 pt y los tres obvios a 2 pt:

🐛 **1. Un lazo dibujado como camino ABIERTO.** El contorno de la flecha empieza y termina en
(0,0) (`markers.h`), pero los tres backends emitían `moveto`/`lineto` + `stroke` **sin cerrar**.
Donde debía haber una unión había **dos tapas planas enfrentadas**: una muesca, no un vértice.
Se cierra cuando el último punto repite el primero (`markerSubpathIsLoop`), y de paso mejora
`square`, `diamond` y `triangle`, que tenían la misma muesca escondida bajo su relleno. En SVG
la forma cerrada es `<polygon>` con `fill="none"`, no `<polyline>`.

🐛 **2. Los tres formatos NO dibujaban el mismo marcador, y llevaba así desde siempre.** El
`stroke-miterlimit` por default es **4 en SVG** y **10 en PostScript y PDF**. Esta flecha pide
**5.1** en la punta (1/sen 11.3°) y **6.25** en las lengüetas (1/sen 9.2°), o sea que caía
justo entre los dos defaults: SVG biselaba todo y EPS/PDF afilaban todo. Comprobado renderizando
el EPS con Ghostscript: con 10 a las lengüetas les salen **púas**. Los tres fijan ahora **5.5**,
elegido para caer entre ambas necesidades: punta en punta, lengüetas biseladas. Verificado a
500 % en los tres backends (rsvg, gs, pdftoppm).

📌 **La lección repetida:** un default heredado no es una decisión. Mientras los tres backends
usaran el suyo, la salida dependía del formato sin que nadie lo hubiera elegido.

**Y la figura, con toques de Alejandro:** órbitas `dash="dashed"` a 0.4 pt y eje en gris a 0.4.
El dash no es decorativo — `gravitacion_orbita` **ya dibuja su órbita punteada**, así que las
dos figuras comparten vocabulario: una órbita es un lugar geométrico, no un objeto. Se descartó
teñirlas de verde: en la otra figura el verde **significa velocidad** y el rojo fuerza, y
reusarlo aquí como adorno costaba esa consistencia. ⚠️ Se recomendó 0.4 pt y **no** `line_width=0`
(el hairline de `gravitacion_orbita`): 0 significa «lo más delgado que el dispositivo pueda», y
en una filmadora a 2400 dpi eso puede desaparecer.

### Cerrado en la sesión del 2026-07-27 (novies) — `limb` opcional, y la raya que cruzaba el globo

Dos cosas del generador de mapas, las dos destapadas por Alejandro afinando `orbita_polar`.

**1. `limb` es ahora un parámetro, y va en el GENERADOR.** Alejandro quería el limbo opcional y
lo había logrado metiéndolo dentro del `if grid` de `mapa_p30_n55.mg`. Tres objeciones, ninguna
estética: (a) el limbo **no es parte de la retícula** —acoplarlos borra dos combinaciones
legítimas, «mapa con limbo sin retícula», que es justo lo que esta figura tenía, y su recíproca—;
(b) solo se editó **uno** de los tres mapas, así que `Mapa(grid=false)` y `PolarMap(grid=false)`
dejaron de significar lo mismo; y (c) es un archivo **GENERADO**: el cambio moría en la siguiente
regeneración. Ahora `geo2mg.py` emite `struct X(grid=true, limb=true, …)` en sus dos modos, y
`orbita_polar` pide `limb=false`, que documenta la intención donde se lee. Churn cero:
`gravitacion_orbita` invoca `PolarMap` sin argumentos y su salida es byte-idéntica.

⚠️ Detalle del modo line-art: ahí el limbo se dibuja ANTES de las costas y las sentencias
`color "black"`/`line_width 0.8` que lo preceden también gobiernan lo que sigue. Meter las tres
en el `if` habría dejado las costas sin color con `limb=false`; solo entra el `circle(1)`.

🐛 **2. Una raya recta atravesando el globo, de borde a borde.** Con `grid=true` aparecía una
polilínea de **2 puntos** cruzando el disco. Reproduciendo el pipeline con pyproj/shapely línea
por línea, el culpable resultó ser **el paralelo de latitud −lat₀: el único que pasa por el
ANTÍPODA de la vista**. Y en AEQD el antípoda no es un punto, **es todo el círculo de radio πR**:
entre dos muestras consecutivas —lon +124 y +126 en la vista lat 30— la línea **salta 39 865 km**
de un borde al opuesto, y el segmento recto que las une cruza el disco visible. El recorte lo deja
en una cuerda de dos puntos.

Sistemático, no accidental: pasaba en la vista lat 30 (paralelo −30) y en la lat 0 (el ecuador);
la polar se salva porque sus meridianos **terminan** en el antípoda en vez de atravesarlo.

📌 **Y el arreglo ya estaba escrito: `partir_saltos()` existía en el archivo desde siempre y NO
SE LLAMABA DESDE NINGÚN LADO.** Código muerto que hacía exactamente esto —cortar donde hay un
salto mayor a 1500 km—. Conectado al bucle de la retícula, antes del recorte. Verificado:
17 piezas en lat 30, 11 en lat 0, ninguna degenerada; la polar no se mueve.

### Cerrado en la sesión del 2026-07-27 (decies) — el primer release con binarios

`v3.0.0-beta` publicado: `mg.exe` para Windows, `.tar.gz` para Linux y macOS, cada paquete con
`lib/`, los ejemplos y la referencia dentro. Es la primera vez que probar MetaGráfica **no
exige compilar**, que era el filtro que se llevaba por delante a casi todo el que llegaba de la
galería. El workflow corrió los seis trabajos en verde, incluido el que nunca se había
ejecutado (`publicar`), y los permisos alcanzaron: el bloque `permissions: contents: write` del
workflow **sobrescribe** el default de solo lectura del repositorio (verificado por API:
`default_workflow_permissions: "read"`).

📌 **Lo que confirmó la corrida sobre la etiqueta, y es el dato que vale:** los mismos **72
archivos idénticos byte a byte en Linux, Windows y macOS**, dicho por un Windows y un Mac
reales, no por la máquina de desarrollo.

🔎 **Y una lección que no es de código: la bandera `prerelease` degradaba justo lo que queremos
que la gente descargue.** Al marcar `v3.0.0-beta` como pre-release, GitHub movió el título
«Latest» al release de 2024 —que **no tiene un solo adjunto** y cuyas propias notas dicen «this
version is still beta»—, o sea que la portada mandaba a un tarball de código de hace dos años.
La bandera significa «existe una versión estable, usa esa»; como no existe, solo hacía daño. Se
invirtió: el nuevo es Latest y el viejo queda marcado como lo que siempre dijo ser.

Las notas del release se reescribieron con el mismo criterio. Decían «la gramática todavía
puede cambiar, y cambia cuando una figura nueva lo pide» —una advertencia que espanta y que
además era inexacta: lo de esta versión fueron **adiciones** con su hueco ya en la
especificación (`limb=`, `marker_at`, `arc(rx,ry)`, `place(rx/ry, at=)`), no cambios de
gramática; el último renombre real fue el `title`→`label` del 2026-07-16—. Ahora dicen **qué**
está en beta (los nombres, no la salida), que los nombres viejos fallan ruidosamente y nunca en
silencio, y que la salida está medida en tres plataformas. La misma información, pero como
argumento en vez de como aviso.

### Cerrado en la sesión del 2026-07-28 — la referencia inglesa, y una compuerta para lo que no se regenera

`docs/reference.md` llevaba **88 líneas de atraso en 5 commits** —del 24 al 27 de julio—, y lo que
le faltaba era, con precisión incómoda, **lo más nuevo**: `marker_at`, `arc(rx, ry)` y el recorte
de arcos, `asin`/`acos` vía `atan2`, arcos bajo transformación con el aviso de que `rotate` gira
el plano, la regla de la ruta log, y los tres mapas de `lib/` con `limb`. O sea justo lo que
querría leer alguien que llega de fuera — que es para lo que existe esa traducción. Los seis
bloques quedaron portados, con sus fragmentos de código verificados igual que los del español.

📌 **Y la causa es la cuarta instancia del mismo patrón en dos días.** `docs/img`, la galería, el
conteo de ejemplos del README y los goldens del traductor se pudrieron todos por lo mismo: salida
derivada que nadie vigila. La referencia inglesa es eso, con un agravante — **no se puede
regenerar**: traducir es trabajo humano, así que no admite una compuerta al estilo `galfail`, que
compara contra lo que el generador produciría hoy.

**Séptima compuerta (`trfail`): se vigila la PROCEDENCIA, no el contenido.** `reference.md` lleva
grabado al final, en un comentario HTML, el `git hash-object` del `referencia.md` del que se
tradujo; la compuerta comprueba que siga siendo el vigente. No sabe si la traducción es buena
—nada automático puede saberlo—, sabe si es **vieja**, que era exactamente lo que nadie notaba.
Cuando falla imprime las tres cosas útiles: que está rancia, el `git diff <hash> -- docs/referencia.md`
que enseña lo que falta traducir, y la línea con la que sellar al terminar.

⚠️ **Se re-sella A MANO, y es deliberado.** Si sellara `capture` o `images`, sellar sería un
efecto colateral de otro comando y volveríamos al punto de partida. Sellar tiene que significar
«ya traduje». Es la misma razón por la que `capture` no toca `docs/img`.

Verificada como las anteriores: se le metió un cambio al español y la cazó (`trfail=1`) con las
otras seis en cero; revertido, vuelve a `trfail=0`. Corre en `check` y en `capture`, así que
también bloquea un release con la referencia inglesa atrasada.

### Cerrado en la sesión del 2026-07-28 (bis) — la condición 4 dio su primer dato, y no fue un bug

Alejandro le pasó a un modelo la **referencia del lenguaje** y una imagen (`geo/infra.png`, el
espectro electromagnético de su curso de Percepción Remota) y le pidió la figura, **sin manera
de ejecutar `mg`**. El resultado, `geo/espectro.mg`, compila a la primera y se parece bastante:
`display_size`/`world_window`, `rectangle` por dos esquinas, `fill` con hex, `text` con
`align`/`valign`/`color`, matemáticas `$10^2$`, subtrayectos con `;` y hasta `/n` para partir
renglón. Todo eso se lee de la referencia y se aplicó a ciegas.

Lo valioso no es el parecido: son **las dos cosas que su intento reveló, y que el autor no podía
ver**.

🔎 **1. Descubribilidad — cómo se le pone una flecha a una línea.** Escribió esto:

    polyline { 63 90  68 90 }
    marker(shape="arrow", marker_orient=0) { 68 90 }

Dos primitivas y la coordenada repetida a mano, cuando lo idiomático —y lo que hace **todo** el
corpus— es `polyline(marker_end="arrow") { 63 90  68 90 }`. **No lo encontró porque la referencia
no lo decía:** `marker_end` aparecía **una sola vez**, de pasada, dentro del párrafo de
`marker_at`, y nunca como *la* manera de ponerle una punta a una línea, que es lo más común que
alguien va a querer. Un lector competente con solo la referencia **no puede descubrirlo**.

Arreglado el mismo día en los dos idiomas: bloque propio junto a las formas de `marker`, con los
cuatro modos compilados antes de escribirlos, tabla de los ocho atributos, y el punto que lo
justifica —la flecha **se orienta sola** a la tangente, así que estampar un marcador suelto
obliga a repetir la coordenada y calcular el ángulo a mano—.

📌 **Esto es exactamente lo que la condición 4 existe para recoger.** No es un bug: ninguna
compuerta podía verlo, y el autor tampoco, porque ya sabe que `marker_end` existe. Hizo falta
alguien de fuera intentando una figura.

🎨 **2. Capacidad — MG no tiene gradientes.** La banda del espectro **es** un gradiente continuo,
y el modelo lo aproximó con franjas planas porque es lo único que el lenguaje permite. Ausencia
notoria (cualquier sistema 2-D los tiene) que nadie había pedido hasta ahora. Abierto
`plan_gradientes.md`, con la viabilidad ya verificada por backend — y con un hallazgo que
condiciona el diseño: **libharu solo implementa el sombreado tipo 4** (malla de triángulos), así
que el lineal sale por ahí pero el **radial no**, y se difiere por eso y no por gusto.

### Cerrado en la sesión del 2026-07-28 (ter) — el segundo dato de la condición 4, y el aviso de lienzo en blanco

Segundo experimento del mismo día y el más informativo de los dos, porque el dato salió de una
**equivocación**. Alejandro le pidió la misma clase de trabajo a otro modelo —una figura desde
una imagen, en chat, sin poder ejecutar `mg`— pero le pasó por error **`docs/bitacora.md` en
lugar de la referencia**. Se conservan los tres archivos en `Dropbox/metagrafica/`:
`fig2-3-flash.mg` (con la referencia), `fig-flash-bitacora.mg` (con la bitácora) y `test2-3.mg`
(la corrección a mano del segundo).

🔑 **El hallazgo: con la referencia eligió la herramienta EQUIVOCADA; con la bitácora, la
correcta.** El intento con la referencia levantó los ejes a mano sobre `world_window` —`grid()`,
dos `axis()`, dos `numbers()`, una struct de círculo por marcador—. El intento con la bitácora
fue directo a `plot(x=…, y=…) { xaxis … yaxis … }`, que es exactamente para lo que existe. Los
dos fallaron al compilar, pero **el segundo falló por sintaxis y el primero por diseño**: sus
errores de sintaxis se arreglan en cinco minutos y debajo queda una figura ilegible.

Por qué: `world_window 0 1 0 6` en un `display_size 12 8`. El motor es isométrico, la escala es
`min(12/1, 8/6) = 1.33 cm/unidad`, y el eje x entero ocupa **1.33 cm de los 12**. Los datos caen
DENTRO de la ventana —el tropiezo nº 1 de §14 no aplica— y aun así no se ve nada útil. Encima
`circle(0.007)` en unidades de mundo da un radio de 0.26 pt: los marcadores están en el SVG y son
invisibles.

La referencia describía las dos piezas por separado (la isometría en §2, `plot` en §11) y en
ningún sitio decía **cuándo usar cuál**. La bitácora no describe ninguna, pero está llena de
sesiones que cuentan por qué se construyó `plot`, y de ahí salió la elección correcta. La
lección no es que la bitácora sea mejor documento —no lo es, ni sirve para escribir figuras—:
es que **una referencia puede tener todos los hechos y ninguna regla de decisión**, y quien la
lee no puede inventarla. Añadida como tal en §2 de los dos idiomas (🔑 «de ahí sale la regla
para elegir herramienta»), más el caso hermano en §14, `smooth(&p)` en §10 (existía en el
motor, no en el documento) y el aviso de que el locus de `place` no admite `&path`.

⚠️ **Y un fallo silencioso que ninguna de las siete compuertas podía ver.** Al corregir el
intento de la bitácora apareció esto: `plot(x=(0,1), y=(0,6), box=(2, 1.5, 15, 9.5))` **sin
declarar `world_window`** compila con código 0, escribe un SVG válido y **sale en blanco**.
`box=` va en unidades de MUNDO y la ventana default es el cuadrado unitario, así que la caja
entera cae fuera. No hay nada a lo que agarrarse: ni error, ni aviso, ni diferencia observable
respecto de un bug del motor.

Cerrado con un **aviso no fatal** (`Aviso: la figura sale EN BLANCO…`, con el lienzo y el dibujo
en cm y la ventana vigente). Tres decisiones que vale la pena registrar:

1. **Se mide en coordenadas de DISPOSITIVO, no de mundo.** `Display` acumula una caja de
   cobertura alimentada por `inkPoint()` —`mt.transform` más el registro—, que los tres backends
   usan en lugar de `mt.transform` justo donde nace una coordenada que se va a pintar. Los
   backends también transforman **vectores de dirección** (la tangente de un marcador, el versor
   con que `text` deduce el giro): ésos siguen con `mt.transform` a secas, porque no son puntos
   de la página y arrastrarían la caja hacia el origen. Medir en dispositivo es lo que hace el
   diagnóstico exacto bajo `transform`, structs y el `box` de un `plot`, que en mundo no se ven.
2. **Se avisa solo cuando la caja NO TOCA la página.** Es la condición segura: sin intersección,
   nada visible se pintó, y no hay falsos positivos. Al revés no vale, y esos casos se callan a
   propósito — en un aviso, un falso negativo cuesta mucho menos que enseñar a ignorarlo.
   Salirse en parte por un borde es corriente y a menudo deliberado.
3. **No se avisa cuando no hubo tinta ninguna.** `examples/curvas3.mg` es biblioteca de datos,
   compila en blanco y está en el corpus. «No dibujé nada» es una decisión; «dibujé fuera del
   papel» es un accidente.

La compuerta 5 (pruebas negativas) no servía tal cual: exige `exit 1` y **cero** archivo de
salida, y un aviso es lo contrario de las dos cosas. Se le añadieron dos marcadores,
`% EXPECT_WARN:` y `% EXPECT_NO_WARN:`, con la misma convención auto-declarativa. Y hacía falta:
**un aviso es el diagnóstico MÁS expuesto a la regresión por silencio que esa compuerta
persigue**, no el menos — un error que deja de darse rompe algo visible tarde o temprano, pero
un aviso que deja de darse no rompe nada: la salida sigue byte-idéntica, las otras seis
compuertas siguen verdes, y lo único que se pierde es la única pista que tenía el usuario.
`EXPECT_NO_WARN` cubre el reverso, que es la otra forma de matar un aviso: llenarlo de falsos
positivos hasta que se ignore.

Las dos verificadas reintroduciendo a propósito lo que deben cazar: enmudecido el aviso →
`ERRFAIL lienzo_en_blanco (compiló en silencio)`; endurecida la condición a «algo se salió» →
`ERRFAIL lienzo_desborde_parcial (FALSO POSITIVO)`. Con ambos bugs dentro, las otras siete
compuertas siguieron en cero — que es justo la prueba de que ninguna podía ver esto.

Estado: `ok=72 … errfail=0 (err_ok=40)`, salida byte-idéntica (el aviso no añade un byte a
ningún archivo), traductor `ok=14`.

**Queda abierto, y es decisión de semántica, no de código:** `place(P, &datos)` no existe —el
locus se escribe literal—, y es la generalización que cualquiera hace de la regla de §3 («el
trayecto va como primer argumento, siempre»). Los dos modelos la intentaron. Antes de añadirla
hay que decidir cómo se lleva con las formas de 2 puntos (`count=`, `gap=`, `both_sides=`) y con
el locus de arco: hoy el número de puntos ES la sintaxis que elige el modo, y un `&path` de
longitud variable la vuelve dinámica.

### Cerrado en la sesión del 2026-07-28 (quater) — gradientes, Fases 1 y 2

`plan_gradientes.md`, abierto esa misma mañana por lo que destapó el experimento del espectro.
Cerradas las dos primeras fases: `gradient=[colores]` + `gradient_angle` en los tres backends,
`examples/espectro.mg` en el corpus (`ok=75`), y la cuarta invariante de la Capa 3 en el mismo
commit, como el plan exigía. La Fase 3 (paradas arbitrarias, radial, transparencia) sigue
diferida por la regla de demanda.

**La decisión de diseño, y va contra lo que decía el plan.** §2 proponía que el eje del degradado
viviera en la caja de la figura (el `objectBoundingBox` de SVG, «el gradiente acompaña a la
forma»). Se descartó al implementarlo, por dos razones que solo aparecen mirando el código:

1. **Ya había precedente y decía lo contrario.** `hatch_angle` barre su familia de líneas sobre el
   bbox de DISPOSITIVO. Se verificó antes de decidir: dos rectángulos con `hatch=45`, uno girado
   30° y otro no, comparten un solo `patternTransform="rotate(45)"` en el SVG — o sea el tramado
   sale a 45° **en el papel**, no respecto de la forma. Tramado y degradado son las dos maneras de
   rellenar un área con algo que no es un color plano; orientarlos en marcos distintos habría sido
   una incoherencia gratuita.
2. **`objectBoundingBox` sesga el ángulo.** Mapea la caja al cuadrado unidad, así que en una caja
   4:1 un gradiente «a 45°» sale a ~76° en la página. Es la familia de `plan_anisotropia.md`, y
   además habría obligado a EPS y PDF —que no tienen ese modo— a reproducir el sesgo a mano para
   no discrepar.

Con el eje en la página, los tres backends consumen **el mismo** (`Display::gradientAxis`) y
coinciden por construcción en vez de por vigilancia. Cada uno lo emite en su mecanismo nativo:
`<linearGradient gradientUnits="userSpaceOnUse">` en SVG (no objectBoundingBox, por lo anterior),
`shfill` con sombreado axial en EPS, malla de triángulos en PDF.

⚠️ **El EPS pasa a nivel 3** y lo declara (`%%LanguageLevel: 3`) **solo** cuando hay degradado, para
no mover la salida de todo lo demás. Se eligió `shfill` sobre la alternativa de franjas finas
porque es exacto, no crece con la resolución, y las franjas son justamente lo que ya se puede
escribir a mano en un `.mg`: emitirlas no añadiría nada al lenguaje. Ghostscript lo interpreta y
`psfail` lo verifica en cada corrida.

🔎 **La copia vendorizada de libharu estaba INCOMPLETA, y nadie lo sabía.** El plan daba por hecho
que el sombreado tipo 4 estaba disponible porque `hpdf.h` lo declara. No lo estaba:
`src/hpdf_shading.c` nunca se vendorizó, aunque sí la mitad consumidora —`HPDF_Page_SetShading`,
`HPDF_Page_GetShadingName`, el dict `/Shading` de `hpdf_pages.c`, todo compilado y enlazado—. El
árbol llevaba siendo incoherente con su propio header desde el día uno del backend PDF, y solo se
notó al necesitar la primera característica que lo usa. Se restauró el archivo de upstream v2.4.6
tal cual (misma licencia ZLIB, cero ediciones, el `wildcard` del Makefile lo toma solo). La
política de no parchear libharu sigue en pie: esto no es un parche, es completar la vendorización.
La limitación REAL —no expone los tipos 2 (axial) ni 3 (radial)— se confirma, y es la razón
técnica por la que el degradado radial queda diferido.

🐛 **Y un bug que ninguna compuerta habría cazado.** El tipo 4 codifica cada coordenada como entero
contra el rango de `/Decode`, y una coordenada FUERA de ese rango no se recorta: **envuelve**. Los
cuadriláteros que reproducen el `/Extend [true true]` del EPS —que por definición se salen de la
forma— reaparecían por el otro lado, encima de la figura, pintándola plana del color del extremo.
El degradado entero se veía de UN SOLO COLOR con el mesh perfectamente correcto: 24 vértices, los
colores buenos, las coordenadas buenas. Se corrige declarando el bbox del sombreado sobre los
vértices ya construidos y no sobre el del path.

📌 **Por qué no lo habría cazado nada, que es lo que vale la pena recordar:** la invariante (d)
cuenta operaciones de sombreado, y aquí había exactamente una por figura en los tres formatos —el
conteo era correcto—. El PDF era byte-estable, así que el golden lo habría bendecido. Se encontró
**mirando el render**. Las siete compuertas cazan clases de fallo, no «se ve mal»; para eso sigue
haciendo falta abrir la figura.

**La cuarta invariante de la Capa 3** cuenta rellenos degradados en los tres formatos
(`shfill` / `fill="url(#mggrad…)"` / `/ShN sh`) y exige que coincidan. Se cuentan **usos, no
definiciones**: dos formas con el mismo degradado y la misma caja comparten un `<linearGradient>`,
así que contar defs daría 1 donde EPS emite 2. Verificada como manda la casa —enmudeciendo el
degradado del SVG—: `c3fail=1`, y sobre todo `capture` **también** da `c3fail=1` mientras bendice
la salida rota sin protestar. Es, como (c), una invariante sin escapatoria por bendición.

`examples/espectro.mg` entra al corpus y a la galería, y es el ÚNICO sujeto de esa invariante: si
sale, la compuerta se queda sin nada que mirar. Está anotado en su encabezado y en `CLAUDE.md`.

### Cerrado en la sesión del 2026-07-28 (quinquies) — el diagnóstico del bloque huérfano

Salió escribiendo `examples/espectro.mg`: bajar el `{ }` de coordenadas a la línea siguiente
falla, y el mensaje hablaba de «se esperaba un comando… pero se encontró el número 0»,
señalando la primera coordenada. Cierto y sin embargo inútil.

**Por qué el mensaje era así**, que es lo que hacía falta entender antes de tocar nada: un `{ }`
suelto **es un constructo válido** —el bloque de ámbito de §7.1—. Así que el compilador no veía
una primitiva rota: veía una primitiva sin coordenadas y, detrás, un bloque de ámbito lleno de
números donde esperaba sentencias. El error era correcto; nombraba el síntoma.

La regla, acotada probándola antes de documentarla: por **dentro** tanto los `( )` como los
`{ }` pueden ocupar cuantas líneas quieras y llevar comentarios —una lista de paradas partida en
dos líneas compila, y un bloque de coordenadas partido también—; lo único que no puede haber es
un salto de línea **antes** del `{`. Vale igual para una primitiva sin paréntesis.

El diagnóstico nuevo vive en **`parseBlock`, no en la rama de `PrimStmt`**, y esa es la decisión:
ahí cubre de una sola vez todas las formas con bloque (`text`, `place`, `fit`, `sine`,
`compound`) en vez de repetirse en cada una. Da dos renglones: qué pasó, y **la línea que hay que
juntar** —se busca hacia atrás en el vector de tokens saltando los saltos de línea, que es barato
porque el archivo se tokeniza entero de antemano—.

⚠️ **Y da una segunda mitad distinta en el cuerpo de un `for`/`if`**, porque ahí la causa es otra:
`for i = 0 to 3 { 0 0  1 1 }` es querer generar coordenadas con un lazo, y juntar las líneas no
arreglaría nada. El primer renglón vale igual; el segundo manda a acumular en un `path` con `+=`.
Sin esa distinción el mensaje habría mentido en un caso para acertar en el otro.

Mismo defecto y mismo remedio que la rama de «`%s` no es un comando conocido (¿primitiva mal
escrita?)» de `parseStateStmt`, que se añadió en su día por la misma razón: el error señalaba al
`{` pidiendo «una expresión» y no nombraba nunca la palabra que había que corregir.

Dos fixtures nuevos (`bloque_coords_otra_linea`, `bloque_coords_en_for`), con `EXPECT_AT` para
que además se verifique **dónde** apunta; `err_ok` sube de 40 a 42. Verificados como manda la
casa: devuelto el diagnóstico genérico, los dos se ponen rojos («falló, pero sin decir…»), que es
justo lo que protegen — no que falle, sino que falle diciendo la causa.

📌 Y una lección de mantenimiento en el mismo movimiento: la entrada de §14 se había escrito unas
horas antes **citando el mensaje viejo**. Cambiar el mensaje la volvió rancia al instante. Se
actualizó en los dos idiomas, pero conviene tenerlo presente — **citar un mensaje del compilador
en la referencia crea un acoplamiento que ninguna compuerta vigila**, y aquí se salvó solo porque
las dos cosas se hicieron seguidas.

### Cerrado en la sesión del 2026-07-28 (sexies) — `elevacion_solar`: al corpus SIN cobertura nueva

Figura nueva, `examples/elevacion_solar.mg`: reconstrucción en español de la Fig. 7.4 de
Lillesand, Kiefer & Chipman (ángulos de elevación solar), pedida para el curso de Percepción
Remota de la ENCiT. Entra al golden — `ok=75` → **`ok=78`**, 26 ejemplos.

**La construcción es física, no medida.** Los dos números declarados arriba —`lat` y la
oblicuidad— gobiernan todo: la posición del observador sobre el limbo, la inclinación del plano
tangente, la de los tres rayos, el barrido de los tres arcos y el número que imprime cada
rótulo (`h = 90° − lat + δ`). Es la misma regla de `orbita_polar`: el valor que manda sobre la
geometría es el que se publica, así que el dibujo no puede desmentir al letrero.

🔎 **La lámina original no aguanta esa regla, y por eso no se copió.** Medidos sobre el escaneo,
sus tres rayos se separan ~10° entre sí, cuando entre un solsticio y el equinoccio va la
oblicuidad, 23.44°; y su punto de tangencia cae a ~24° de latitud, donde el Sol de verano pasa
al norte del cenit. Se rehízo, no se tradujo.

**El limbo es un meridiano.** `lib/fulldisk_map.mg` es una vista ortográfica desde el ecuador,
así que el borde del disco es el meridiano a 90° de la vista, visto de canto: un punto del limbo
a ángulo φ está **exactamente** a latitud φ. El observador se posa ahí sin corrección de ninguna
clase, y el ecuador del mapa cae solo sobre el plano ecuatorial dibujado. Nada de esto se ajustó
a ojo porque nada hacía falta ajustar.

El giro del satélite tampoco es a ojo: su antena apunta a +y en el marco del icono, así que para
mirar a la Tierra hay que llevarla a `lat + 180`, o sea `rotate = lat + 90`. Escrito con la
variable, sigue mirando a la Tierra si se cambia la latitud.

⚠️ **La latitud más significativa es la que no se puede dibujar.** En el trópico de Cáncer
`lat = obl`, luego `h = 90.0°` exacto: el Sol al cenit en el solsticio — que *es* la definición
del trópico. Pero entonces el rayo de verano y la línea del cenit son la MISMA línea, y la
composición de Lillesand supone al satélite sobre esa línea: el Sol le queda pegado detrás y los
rótulos se encabalgan (se compiló para verlo, no se supuso). El despeje del icono vale
`d·sin(lat − obl)`, que con `d = 8.8` y medio ancho ~1.5 pide **lat ≳ 33°**, y en el trópico vale
cero: no hay tamaño ni distancia que lo salve, solo sacar al satélite del cenit. Se eligió
latitud media (45°) y conservar al satélite en el cenit; el trópico queda como posible lámina
aparte, donde la coincidencia sería la lección y no el estorbo.

📌 **Por qué entró al corpus, que es lo nuevo aquí: NO ejercita ni una característica exclusiva.**
Se revisó una por una antes de proponerlo, y todas tienen ya quien las cubra — listas de cadenas
indexadas en `fill_styles`/`symbols`/`line_patterns`, el arco con flecha en los dos extremos y
`marker_start_orient="reverse"` en `fig2-5:96` (el mismo renglón), el texto girado en los rótulos
de eje de `fig1`/`quickstart` (en SVG sale el mismo `<g transform="rotate()">`; solo cambia el
ángulo), los dos `include` de `lib/` en `gravitacion_orbita`. Lo que aporta es **ser el único
usuario de `lib/fulldisk_map.mg`**: un asset generado, committeado y documentado en §12 de la
referencia que **ninguna de las ocho compuertas miraba**. Los otros dos mapas sí tenían usuario;
este no. Sienta un criterio de admisión que no existía: un ejemplo puede entrar por cubrir un
**asset** del repo, no solo una característica del lenguaje. Y es la primera figura del corpus
hecha para una **audiencia externa** —un curso— y no para probar el motor.

De paso, el conteo de tarjetas de `CLAUDE.md` estaba rancio: decía 23 desde el 2026-07-23 y eran
24 desde que entró `espectro`. Ahora 25, y el número sale de contar los `<article>` de la página
generada, no de la memoria.

### Cerrado en la sesión del 2026-07-29 — la cara tipográfica se fugaba entre renglones, y solo en PDF

`text("(1) Radiación incidente $E$/n(con factor de atenuación)")` sacaba el **segundo renglón en
itálico matemático**, y lo dejaba puesto para los textos siguientes. Solo en PDF: EPS y SVG
salían bien. Se vio mirando el render de una figura nueva, no por una compuerta.

**La causa es un guard que compara contra el estado equivocado.** `PDFDisplay::setFontFace`
abría con `if (face == dspstate.fontFace) return;`, y `dspstate` es el estado **lógico**, que
`push/popDrawState` restauran. Pero la cara del **dispositivo** vive en `current_font`, que es un
miembro nuestro y el `q`/`Q` de libharu no toca. Al salir de un bloque el estado lógico volvía a
la cara de fuera mientras el dispositivo seguía con la de dentro, y el guard —viendo `dspstate`
ya restaurado— **no re-seleccionaba**. Arreglado con un caché de dispositivo `dev_face`, fuera de
`dspstate`: exactamente la misma familia y el mismo remedio que el bug de `font_size` en EPS del
2026-07-09.

Vale la pena el detalle de por qué no bastó lo que ya había: `TextBlock::draw` **sí** acota cada
renglón con `push/popDrawState`, y su comentario describe este mismo caso —se puso el 2026-07-21
justo para eso—. El motor tenía la arquitectura correcta; lo que la derrotaba era el caché de un
backend. Un `push/pop` solo sirve si todos los guards de abajo miden contra el dispositivo.

⚠️ **El golden lo bendecía, y no de casualidad: `test/golden/texto.pdf` tenía los cuatro renglones
en negrita**, cuando `texto.mg:86` es un `text()` aparte y sin `/b`. O sea que el ejemplo que
existe para cubrir `TextBlock` —el único— llevaba el bug horneado en su propio golden desde que se
capturó. La invariante (a) de la Capa 3 cuenta operaciones de texto: son las mismas con la cara
bien o mal, así que es ciega a esto. Ninguna de las ocho compuertas podía verlo; el arreglo se
verificó comparando PDF contra EPS y contra la intención declarada en el `.mg`.

Repro mínimo, por si vuelve: cinco rótulos —`"uno $E$ dos"`, `"tres cuatro"`, `"cinco $E$/nseis"`,
`"siete ocho"`, `"nueve $\frac{a}{b}$ diez"`— rendeados en PDF y en EPS lado a lado. Antes
diferían en tres de los cinco; después son idénticos. Y da la forma exacta de la fuga: un run
math **en la misma línea** siempre restauraba bien (por eso «dos» nunca salió mal); lo que no
restauraba era el **corte de renglón**, y de ahí en adelante contaminaba a los vecinos hasta que
un `\frac` volvía a fijar la cara.

Movió **un solo golden** en todo el corpus, `texto.pdf`. `ok=78` y traductor `ok=14`.

### Cerrado en la sesión del 2026-07-29 (bis) — el Sol como icono de biblioteca

`lib/sun.mg` nueva: `struct Sun(rays=12, disc="gold", ray="darkorange", ray_in=1.35,
ray_out=1.9, lw=0.6)`. Disco de **radio 1**, así que `scale` ES el radio en unidades de mundo,
igual que en los mapas. Los rayos van en un anillo **aparte** —de `ray_in` a `ray_out`, en
radios—, separados del disco: ese hueco es lo que lo hace legible a tamaño chico, y es la única
decisión de dibujo del archivo. Estilo tomado de una lámina de pictogramas de sol (no va en el
repo): rayos de línea, no púas.

Adoptada en `elevacion_solar`, donde reemplaza tres `circle(fill="gold")` **y sus tres
`text("Sol")`**: el icono se lee solo, el rótulo era andamio. Lo dijo Alejandro antes de verlo
—«seguramente no necesitará la etiqueta, pero tal vez los rayos se encimen»—, y acertó en las
dos, aunque el encimamiento no fue donde se esperaba: no entre rayos de soles vecinos (los
separan 4.75 unidades y el anillo mide 1.52) sino entre el anillo y **las etiquetas de época**,
que colgaban de `sun_r + 0.5` y quedaban debajo de los rayos. En el primer intento
«Primavera/Otoño» se salía del lienzo.

**Lo que arregla el choque es medir desde el radio LIBRE, no desde el disco.** `sun_out =
2.15*sun_r` es el radio pasado el cual ya no hay rayos, y de ahí cuelga todo lo que se separa del
sol: el corte del rayo entrante (si llegara al borde del disco, como antes, atravesaría el anillo)
y el margen de los rótulos. Con eso el icono puede crecer o encogerse sin recalibrar nada.

Aun así hizo falta margen, y la columna de soles pasó de `x = 14.5` a `13.9`. 📌 **Mover la
columna no cambia ningún ángulo**, y eso es lo que hace que sea un ajuste de composición y no una
mentira: la altura de cada sol se calcula de su declinación (`sy = Py + (sun_x − Px)·tan δ`), así
que los tres resbalan por su propio rayo. Las elevaciones siguen imprimiendo 68.4/45.0/21.6, que
es la regla de la entrada del 2026-07-28: el número que gobierna la geometría es el que se
publica. Queda escrito en las NOTAS del `.mg`, porque un `13.9` sin explicación es justo lo que
alguien «corrige» de vuelta a `14.5`.

⚠️ **Un detalle cosmético que se decidió NO perseguir:** en el sol del equinoccio el rayo entrante
es horizontal y uno de los doce rayos del icono también, así que quedan colineales con el hueco
en medio y se leen como una raya cortada. En los solsticios no pasa: 23.44° no cae en la malla de
30°. Se quita con `rays=10` en ese sol si algún día molesta.

`lib/sun.mg` **nace con un usuario vigilado por las compuertas**, que es lo que a los tres mapas
les faltó hasta el 2026-07-28. Y de paso: §12 de la referencia enumera el contenido de `lib/`, así
que enumerarlo mal es la clase de podredumbre que este proyecto persigue — se añadió en los dos
idiomas y se re-selló `reference.md`.

### Cerrado en la sesión del 2026-07-29 (ter) — `marker_mid` no tenía una sola prueba

Sección 5 nueva en `markers-demo.mg`, el catálogo: `marker_start` / `marker_mid` / `marker_end`
sobre una polilínea de cuatro puntos, en contraste con la sección 2, donde `marker=` decora
**todos** los vértices. Sale un círculo al arranque, **dos** flechas en los vértices intermedios y
una al final, y eso es lo que enseña: `marker_mid` toma los intermedios, así que un vértice de más
es un marcador de más — es la manera de poner una punta a media línea sin partirla en dos.

⚠️ **Por qué hacía falta: `marker_mid` estaba documentado en la tabla de §4 de la referencia y NO
lo ejercitaba ningún `.mg` del árbol.** Sus dos hermanos sí tienen quien los use (`marker_start` y
`marker_end` salen en `fig2-5`, `fig4-1`, `elevacion_solar` y `gravitacion_orbita`); el de en medio,
nadie. Y `markers-demo`, que es el catálogo y el sitio donde un lector va a buscarlo, cubría las
dos clases de marcador —la primitiva y el atributo— pero ninguna de las tres formas posicionales.
Su regresión natural es la peor de todas: la punta desaparece, la salida sigue siendo
byte-estable, y las ocho compuertas siguen verdes. Queda anotado como cobertura exclusiva en el
encabezado del ejemplo.

Para hacerle sitio, el lienzo creció hacia abajo (`display_size 12 13.6`, `world_window 0 12 -1.6
12`). **Ventana y lienzo crecen lo mismo**, así que la escala isométrica no se mueve y nada de lo
que ya estaba cambia de tamaño ni de posición; va dicho en un comentario, para que el `13.6` no
parezca un número mágico. Verificado en los tres backends: círculo relleno y tres arpones, idénticos.

📌 **Y el motivo de la auditoría, que es la parte que sirve para después.** Todo esto salió de
preguntar si `examples/efectos_atmosfera.mg` —una figura del curso de Percepción Remota, la Fig.
1.11 de Lillesand— debía entrar al corpus. Se revisó característica por característica, como con
`elevacion_solar`: `angle_at`/`point_at`/`curve=true` los cubre `path_sample` (que además lo
declara como cobertura propia), `path X = sine(…)` lo cubren `franck_condon` y `turning_points`,
`atan2` `orbita_polar`, y `sqrt`, `hatch`+`hatch_gap`, el subíndice de grupo `_{…}`, el icono
girado por un ángulo derivado de la escena y los varios `include` en una figura tienen todos dueño.
**Lo único exclusivo era `marker_mid`.** Y el argumento de asset —entrar por ser el único usuario
de `lib/sun.mg`, que es exactamente por lo que entró `elevacion_solar`— se había evaporado una hora
antes, al adoptar `Sun` en `elevacion_solar`.

Así que la figura **no entra**, y la bandera se cubre donde le toca. La regla que queda, dicha por
Alejandro mientras prepara el curso —«va a ver más figuras y no todas entrarán al corpus»—: una
figura de curso **no entra por default**. Entra si aporta cobertura que nadie más da, o si es el
único usuario de un asset vigilado; y si lo que aporta es UNA bandera, la bandera se cubre en el
ejemplo que le corresponde, no se admiten 120 líneas por ella. Es el complemento del criterio del
2026-07-28, que dijo cuándo SÍ; este dice cuándo no basta.

La figura se queda fuera del repo por ahora: un `.mg` committeado en `examples/` que no está en la
lista `EXAMPLES` sería un archivo que **ninguna compuerta compila**, justo el patrón que `imgfail`
y `galfail` existen para evitar. Si las figuras de curso se acumulan, pedirán carpeta propia
declarada como no-corpus.

### Cerrado en la sesión del 2026-07-29 (quater) — la novena compuerta: los bloques de la documentación

Las ocho compuertas anteriores vigilan la **salida** del compilador. Ninguna mira lo que la
documentación **afirma**, y una afirmación falsa ahí es peor que un bug: es un bug que el lector
copia con confianza. `tools/docblocks.py` compila los bloques ```octave de `docs/referencia.md` y
`docs/reference.md`; `docfail` en `test/run.sh`.

**En su primera corrida encontró uno, y de los caros.** El ⚠️ de §10 declaraba que la forma de
partir de un trayecto que ya tienes es la de paréntesis —`smooth(&nodos)`—, «igual que en el
resto del álgebra», y añadía «vale lo mismo para `bezier`, `polyline` y las demás». Medido:
`polyline(&p)` y `bezier(&p)` sí; **`smooth(&p)` y `sine(&p, …)` no compilan**, los generadores
exigen su bloque literal. O sea que el aviso era cierto para las primitivas que CONSUMEN un
trayecto y falso justo para los generadores, que era el párrafo al que estaba pegado. Llevaba ahí
en los dos idiomas sin que nada lo viera. Corregido el texto a lo que el compilador hace hoy
—incluida la limitación, dicha en voz alta: suavizar un trayecto que ya está en una variable no se
puede— y anotada aparte la pregunta de gramática, que es de Alejandro: implementar `smooth(&p)` o
dejarlo así.

📌 **De dónde salió, porque explica el diseño.** No de auditar la referencia, sino de preguntar
qué contexto necesita un **agente externo** —un ChatGPT, sin repo y sin compilador— para escribir
una figura sencilla. La respuesta empieza antes del contenido: que todo lo que el contexto afirme
compile. ⚠️ **Un humano tropieza con un ejemplo malo y desconfía del documento; un modelo obedece.**
Para él la referencia es la única fuente de verdad, así que un error ahí no es una molestia, es
código roto con toda seguridad. Y es una compuerta que le sirve igual al lector humano: nadie
había comprobado nunca que los ejemplos de la referencia funcionen.

**Cómo declara cada bloque lo que espera: en el propio `.md`**, con un comentario HTML invisible al
renderizar, y no en una lista dentro del tool — misma política que `test/errors/*.mg`, porque dos
listas que mantener se desincronizan. `<!-- mg-noexec: razón -->` para lo que no es código (el
bloque de firmas de §10, con sus corchetes de argumento opcional); `<!-- mg-expect-error -->` para
un contraejemplo ❌ deliberado, que **debe** fallar — y que rompe la compuerta también si algún día
**compila**, porque un contraejemplo que dejó de serlo enseña lo contrario de lo que dice. Es el
mismo par que `EXPECT_WARN`/`EXPECT_NO_WARN` de las pruebas negativas.

**Los fragmentos no se marcan, y el argumento de por qué es lo interesante.** Nueve bloques usan una
`struct`, un `path` o una variable que el texto definió párrafos antes y no repiten; no pueden
compilar solos. No hacen falta marcadores porque `struct no definida`, `path no definido` y
`variable no definida` son errores de **EVALUACIÓN**: si el parseo hubiera fallado, el error sería
de sintaxis y se reportaría **en su lugar**. O sea que ver uno de esos tres prueba que el bloque
parseó — que es exactamente lo único que esta compuerta juzga. Sin ese argumento habría hecho falta
marcar nueve bloques a mano, y cada marca es una cosa más que se pudre.

Estado: 28 compilan, 9 fragmentos, 1 notación, 1 contraejemplo, 0 fallos, en los dos idiomas.
Verificada como manda la casa, reintroduciendo las tres clases: el bug real de vuelta (lo caza), el
marcador del contraejemplo quitado (lo caza, o sea que la marca es load-bearing) y el contraejemplo
vuelto válido (lo caza con su mensaje propio). `ok=78 … docfail=0`.

### Cerrado en la sesión del 2026-07-29 (quinquies) — el Modelfile del agente, con números

`tools/generar_modelfile.py` reescrito: ahora emite el Modelfile COMPLETO (`docs/modelfile_llm.txt`)
en vez de solo la galería, y `docs/galeria_llm.md` desaparece. Las tres decisiones salieron de un
experimento de tres brazos, no de opinión: mismo modelo (`qwen2.5-coder`, temp 0.1, seed 42), tres
SYSTEM —**A** referencia completa, **B** galería de 8 ejemplos sin NOTAS, **C** las dos—, tres
tareas cortas y `bin/mg` como juez. Artefactos en `../agente/exp3/`, fuera del repo.

**Contexto medido:** A 15.8k tokens · B 10.0k · C 25.2k · Modelfile anterior ~49k, o sea **1.5×
por encima de `num_ctx 32768`**: más de la mitad de la galería se descartaba en silencio. Con un
prompt largo, C llegó a 30.1k/32.8k = 92%, así que **C no es viable**.

**Lo que decidió cada cosa.** (1) Fuera la referencia completa: el brazo A compilaba igual que los
otros y **alucinaba mobiliario** —un `legend` vacío, un `table` con cuatro filas inventadas,
argumentos que no existen (`frame=`, `grid_dash=`)—; darle el catálogo entero a un modelo chico
hace que use todo lo que ve. Y era el único que **no lograba corregirse** con el mensaje del
compilador ni en dos vueltas, mientras B se corregía en una. (2) §15 va al frente: es el destilado,
y en la referencia vive al final —donde le toca para un lector humano—, que es lo primero que se
pierde si algo se trunca; reordenar es trabajo del Modelfile, no del documento. (3) Ejemplos sin
`% NOTAS` y lista explícita: las NOTAS eran el **28%** de lo que se mandaba y son procedencia
bibliográfica, mediciones y avisos de cobertura — ruido para quien escribe una figura. Es la misma
decisión que `tools/galeria.py` ya había tomado para el caso gemelo.

📌 **Los tres brazos eligieron `plot`**, o sea que la regla de decisión que se añadió a §2 el
2026-07-28 funciona — y B la acierta con `fig6-4` sola, sin referencia.

⚠️ **Y lo que NO se arregló, que es el resultado importante: las tres gráficas compilan y las tres
son ilegibles.** Causa raíz verificada arreglando solo eso: ponen **`world_window` en unidades de
DATOS**. Con la ventana en geometría de página y un `box=`, el resto de lo que escribió el modelo
estaba bien. Es UNA idea equivocada, es la regla más explícita del Modelfile, la ignoró en las tres
versiones, y **compila limpio**, así que el bucle con el compilador tampoco la ve. Apunta a un
aviso del motor —la clase del aviso de lienzo en blanco del 2026-07-28—, que es lo único que un
modelo no puede ignorar y que además le sirve a un humano.

💡 **Una trampa de prompt que conviene no repetir:** la primera versión de la regla del salto de
renglón decía «va `/n`… `\n` no existe en MG», y el modelo escribió **`\n`** — la regla *nombraba*
la forma incorrecta y eso fue lo que copió. Reescrita sin mencionarla, solo con la buena deletreada
y un ejemplo marcado «cópialo», salió bien a la primera. Las otras nueve reglas están expuestas a
lo mismo.

⚠️ **Dos trampas de instrumento, ninguna del modelo.** `ollama run` escribe **escapes ANSI a
stdout** aunque redirijas a archivo (`\x1b[4D\x1b[K` partiendo números por la mitad): tumbó una
corrida entera y es el mismo artefacto del 2026-07-28, cuyo diagnóstico —«redirigir a archivo»— no
bastaba; hay que usar `/api/generate`. Y la descripción de figura que se le daba (un volcado de
visión sobre un escaneo malo) **no es una tarea válida**: confunde tres habilidades, y los brazos
fallaban por la primera sin llegar a medir la tercera. Lo señaló Alejandro.

### Cerrado en la sesión del 2026-07-29 (sexies) — dónde vive una figura de curso: fuera del repo

`efectos_atmosfera.mg` —reconstrucción en español de la Fig. 1.11 de Lillesand, Kiefer & Chipman,
el balance radiativo que ve un sensor remoto, para el curso PercepcionRemota2026 de la ENCiT— se
terminó hoy y **no vive en el repo**: Alejandro la movió a los `assets` del curso y le cambió los
`include` de `"../lib/sun.mg"` a `"sun.mg"`, o sea a la **biblioteca instalada**. Compila así.

📌 **Y eso zanja la pregunta que quedaba abierta.** La entrada (ter) de hoy decía que la figura se
quedaba fuera del repo «por ahora» y anotaba que un `.mg` en `examples/` que ninguna compuerta
compila es el patrón que `imgfail` y `galfail` existen para evitar. La solución resultó no ser una
compuerta nueva ni una carpeta `examples/curso/`: **una figura de curso vive con el curso**, y la
forma de `include` de nombre a secas (§12) es exactamente el mecanismo que lo permite —`lib/` es
instalable justamente para esto—. Vale para las que vienen: al corpus solo entra lo que aporta
cobertura, y lo demás se apoya en la biblioteca instalada desde donde le toque vivir.

⚠️ **La dependencia que eso crea, dicha en voz alta:** la figura del curso depende de `lib/sun.mg`
y `lib/satellite.mg` **instalados**, no de una copia propia. Si `lib/` cambia de forma incompatible,
la figura del curso se rompe sin que ninguna compuerta lo note — la red de pruebas mira `examples/`.
Lo que la protege de hecho es que `lib/sun.mg` tiene ahora un usuario dentro del corpus
(`elevacion_solar`), así que su geometría está vigilada aunque su cliente externo no lo esté.

**Lo que la figura enseñó, ya recogido en el repo:** el sensor es `lib/satellite.mg` orientado por
el rayo que lo alimenta —`atan2` de sensor→terreno menos 90°, porque la antena del icono apunta a
+y, y el origen retirado `1.05·scale` para que la BOCA de la antena caiga donde convergen las
flechas— y el sol es `lib/sun.mg`, la biblioteca que nació de aquí (entrada *bis*). También destapó
el leak de cara tipográfica en PDF (entrada de la mañana) y motivó la auditoría de admisión que
cerró `marker_mid` (entrada *ter*). Verificada en los tres backends: paridad de texto 23/23/23,
`gs` limpio, `arcparity` OK, cero avisos.

### Cerrado en la sesión del 2026-07-30 — `make install` reparte también ejemplos y doc legible

Un `mg` instalado no tenía **de dónde aprender el lenguaje**: `install` dejaba binario, man y
`lib/*.mg`, pero la referencia y los ejemplos vivían solo en el árbol de fuentes. Ahora `install`
lleva la referencia (`referencia.md`/`reference.md`), la galería (`galeria.html`/`gallery.html` +
`docs/img/*.svg`) y el ensayo `calcular_en_vez_de_medir.md` a `share/doc/metagrafica/`, y los 26
ejemplos a `share/metagrafica/examples/`. La bitácora y `plans/` **NO se instalan**: son de
mantenedor.

📌 **Por qué los ejemplos van a `share/metagrafica/examples` y NO a `share/doc`.** Varios ejemplos
hacen `include "../lib/x.mg"` (ruta relativa) o `include "curvas3.mg"` (hermano). La búsqueda §15 es
`g_baseDir` (dir del archivo principal) → `MG_LIBDIR`. Si el ejemplo se instala **hermano de `lib/`**
bajo `share/metagrafica/`, ese `../lib/` resuelve igual instalado (`examples/../lib`) que en el árbol
—**sin editar un solo `.mg`**—. La alternativa que propuso Alejandro —dejar el `include` de nombre a
secas y que caiga a `MG_LIBDIR`— es más limpia para el binario instalado, **pero rompe el árbol sin
instalar**: en un checkout `MG_LIBDIR` apunta a `/usr/local/...` que no existe, y hoy los tres
ejemplos con mapa (`orbita_polar`, `gravitacion_orbita`, `elevacion_solar`) compilan en el golden
*precisamente por* la ruta relativa. Rescatarla exigía que el binario no instalado hallara el `lib/`
del repo (hornear `MG_LIBDIR=$(pwd)/lib`, no portable; o un candidato `exeDir/../lib`, y las dos
disposiciones difieren). Se descartó: la opción de hermanos es cero-edición, cero-motor, cero-riesgo
para las compuertas.

⚠️ **`install -d $(PREFIX)/bin ${MANPREFIX}/man1` como primera línea:** el `install` previo asumía
que esos dos dirs ya existían (normal para `/usr/local`), y reventaba en un prefijo virgen. Cazado
al probar la instalación en un stage temporal.

Verificado reconstruyendo con el `PREFIX` del stage (para que `MG_LIBDIR` apuntara dentro) e
instalando en un prefijo **virgen**: los cuatro casos con `include` rinden SVG correcto desde su
ubicación instalada —`orbita_polar`, `gravitacion_orbita` (dos `../lib/`), `fig4-1` (hermano
`curvas3.mg`) y `elevacion_solar` **desde otro cwd**, que confirma que `g_baseDir` sale de la ruta
del archivo y no del cwd—. `uninstall` refleja todo y deja `share/doc` (compartido) intacto. Solo se
tocó el `Makefile`; el motor y los ejemplos quedaron sin cambiar, así que no movió ningún golden.

---

## 2026-07-31 — `plan_pseudo3d.md`: nueve figuras en vez de tres clientes abstractos

Sesión de **documento, no de motor** (`ok=78 … docfail=0` sin moverse; único archivo tocado
además de este, `docs/plans/plan_pseudo3d.md`).

El plan de pseudo-3D derivaba todo su diseño de tres figuras del libro de Percepción Remota
(Fig. I.2, II.11, II.10) **que nadie había visto**, y de ahí sacaba una sola necesidad —malla de
suelo + rayos—, una sola adición al motor (`xyz()`) y un orden de fases que la seguía. Alejandro
juntó **nueve imágenes** en `local/simulate3d/meta/` con las figuras que se aspira a poder hacer,
y al mirarlas una por una **no piden todas lo mismo**. El plan no tenía dónde registrar eso, así
que se le añadió una §2 con un **vocabulario de seis estrategias (A–F) y una tabla
figura×estrategia**.

📌 **Lo que la tabla hace visible, y era el punto:** dentro de `fig2-7b` la **pantalla y el
cristal son estrategias distintas** (plano de la escena vs. sólido de caras); `lira_II-1` y
`waves`, de libros distintos, son **la misma**; `richards_1-6` y `1-7` también, así que una sola
figura de prueba cubre las dos; y `lira_II-7` con `fig18-5` comparten la única pieza que pide
geometría nueva, así que **se difieren juntas**. También aparece que en tres figuras **más de la
mitad del dibujo es anotación 2-D que ya se sabe hacer** (estrategia F) — reconocerlo es lo que
mantiene chico el alcance de todo lo demás.

**Hallazgo que cambió el diseño: una elipse del motor YA ES un círculo proyectado.**
`Matrix::ellipse_frame` entrega centro + semidiámetros conjugados `P(t) = C + u·cos t + v·sin t`,
que es literalmente la forma cerrada de la proyección ortográfica de un círculo del espacio (`u`,
`v` = proyecciones de la base del plano por el radio), y los tres backends **ya la consumen**
desde la reconstrucción de arcos del 2026-07-27. Consecuencia: una sentencia `plane3d` que empuje
la matriz del plano hace que `circle(r)` sobre un plano de la escena salga como **la elipse
exacta, con cero cambios en los backends** — se implementa reusando `OPMPUSH`/`Transform` como ya
hace la sentencia de transformación, más `using_ellipse` como hace `shear`. Es la pieza **más
barata del plan y la de más clientes (9 de 9 figuras)**: sin ella cada elipse se sigue midiendo a
ojo, como hoy en `fig2-7b-v3.mg` (`ellipse(0.6, 1.3)`, calibrada contra el `.png`). Por eso las
adiciones al motor pasaron de una a **dos** (`xyz()` **y** `plane3d`) y el orden de fases se
invirtió: primero el plano, después los rayos.

**Corolario gratis:** dentro de un `plane3d` el dibujo 2-D corriente funciona sin enterarse
(`sine`, `smooth`, `polygon` relleno, `place`), así que `lira_II-1` y `waves` **no necesitan
muestrear nada a mano ni usar `xyz()`**. ⚠️ Y de ahí el footgun a documentar: `xyz()` devuelve un
punto **ya proyectado**, o sea que dentro de un `plane3d` se transformaría **dos veces**.

**Decisión de ejes: `z` = PROFUNDIDAD** (x derecha, y arriba, z hacia el observador), no `z`
vertical. La razón es que hace de **`view3d(azimuth=0, elevation=0)` la identidad**: la vista
frontal de `fig10-2`/`fig2-7b` —donde la cara que importa conserva su forma real— queda como el
caso por default y no como un caso especial. El suelo de `richards`/`waves` es entonces el plano
**xz**. ⚠️ Ojo con `fig18-5`: sus rótulos `x`/`y`/`z` son de la física que ilustra, no de este
marco. Con eso quedan cerradas las cuatro decisiones que §7 dejaba abiertas (`xyz`, `view3d`,
una sola sentencia con `type=`, y los ejes).

⚠️ **Se corrigió también la fase «las figuras entran al corpus», que estaba mal escrita.** No
entran las siete: `local/` es confidencial **a propósito** (`.gitignore`: «figuras de artículos
sin publicar»), y el precedente del corpus (`franck_condon`, `turning_points`, `fig4-4`) es que
entra la **reproducción** con su procedencia en el encabezado, nunca el escaneo. Y una figura
entra por cobertura **de motor**, no de tema —la regla que dejó fuera a `efectos_atmosfera`—, así
que la recomendación es **tres**: `lira_II-4` (única usuaria del plano en bruto, ~14 círculos en
dos `for`), `richards_1-7` (única de los rayos) y `waves` (única del relleno en un plano y del
orden de pintado). Con nombre de la física (`angulo_solido`, `push_broom`, `onda_3d`): el número
de figura solo se usa cuando la edición es verificable por un lector, y estas no lo son.

🚧 **Y una frase de alcance que faltaba, dicha por Alejandro y ahora escrita en el encabezado del
plan:** todo esto es *simulación* de 3-D hecha en 2-D. Si algún día hace falta 3-D de verdad, eso
es Blender u otra herramienta, y **MG no la va a sustituir**. El valor de MG aquí es que la figura
sale de un `.mg` que se lee, se versiona y se recompila — no competir con un motor de render.

⚠️ **Los bloques ```octave de este plan son ILUSTRATIVOS** (sintaxis que aún no existe) y van
marcados como tales, porque a `docs/plans/` **no lo mira ninguna compuerta**: `test/run.sh` solo
le pasa `referencia.md` y `reference.md` a `docblocks.py`. Por eso la documentación de verdad es
una fase aparte del plan, en la referencia, donde `docfail` sí la compila.

---

## 2026-07-31 (bis) — Fases A+B y C del plan pseudo-3D: la cámara ya es un parámetro

Tres commits: `32f4089` (`tools/ver.sh` + hallazgos), `c7e3aca` (`view3d`/`plane3d`/`xyz()`),
`7af7866` (`lib/pseudo3d.mg` reescrita). `ok=78 … docfail=0 (err_ok=44)`, traductor `ok=14`.

### El método, que es lo que más conviene reusar

**La figura se escribió ANTES que la sintaxis, a propósito.** Se reconstruyó la fig. II-4 de
Lira —una esfera reticulada con el casquete del ángulo sólido, catorce círculos del espacio—
armando a mano la matriz de cada plano y descomponiéndola **QR en el propio `.mg`** para
emitirla como `rotate/scale/shear`. Cuatro líneas de álgebra por círculo, repetidas tres veces.
Eso compró dos cosas: la geometría quedó **probada en los tres backends antes** de añadir
gramática (y por tanto, si algo fallaba después, era la sintaxis y no las matemáticas), y
`plane3d` nació como **abreviatura de algo ya funcionando** en vez de como apuesta de diseño.
Al portar la figura, `tools/ver.sh --diff` dio **0 px en EPS/SVG/PDF y también con otra cámara**
(35°/42°) — lo segundo es lo que lo vuelve prueba y no coincidencia—, con los bytes de EPS y SVG
sí cambiando. Cuerpo del fuente: 113 → 87 líneas.

⚠️ **Pero 0 px solo es criterio válido si la figura YA era escena-derivada.** En la Fase C,
`fig2-7b-v3` no podía darlo **por construcción**: sus piezas no compartían cámara, y se puede
medir —la pantalla (`plano k=0.3`) recedía a **73.3°** y el cristal (`prisma a=35`) a **35.0°**,
treinta y ocho grados—. Cuando las piezas no comparten mundo, meterlas en uno **obliga** a
cambiar el dibujo: ese cambio *es* el arreglo. Confundir las dos situaciones llevaría a
perseguir un 0 px imposible o, peor, a "conseguirlo" conservando el defecto.

### `plane3d` cuesta cero cambios de backend, y por qué

`Matrix::ellipse_frame` representa una elipse por **centro + semidiámetros conjugados**
`P(t) = C + u·cos t + v·sin t`, que es literalmente la forma cerrada de la proyección ortográfica
de un círculo del espacio (`u`, `v` = proyecciones de la base del plano por el radio). Los tres
backends ya la consumen desde la reconstrucción de arcos del 2026-07-27. Así que `plane3d` es un
`Stmt` que empuja esa matriz con `OPMPUSH`/`Transform` —como ya hace la sentencia de
transformación— y enciende `using_ellipse`; un `circle(r)` dentro sale como la elipse **exacta**.
Y el `from`/`to` de un `arc` sigue siendo ángulo **del plano**, lo que hace que recortar por
visibilidad se escriba solo. `xyz()` devuelve un `Value::LIST` de dos, el mismo mecanismo de
`point_at`: **la gramática no se tocó**.

**Ejes: `z` = profundidad** (x derecha, y arriba, z hacia el observador). Se eligió porque hace
de `view3d(azimuth=0, elevation=0)` la **identidad** — la vista frontal es el default y no un
caso especial. Verificado a 0 px contra el dibujo plano de siempre.

📌 **El ocultamiento de la mitad trasera salió en forma cerrada**, como en `orbita_polar`: un
punto se ve si `P·w > 0`, y sobre un círculo eso es `A + B cos t + D sin t > 0`, o sea un solo
`acos`. Los cortes **caen sobre el limbo por construcción** (el borde de visibilidad es el plano
`P·w = 0`, y su intersección con la esfera ES el limbo). Para un meridiano `A = 0` ⇒ media
elipse exacta; para un paralelo `A/M = tan(lat)·tan(elev)`.

### Fase C: retirar una struct también es trabajo

`plano` **se fue**: era `plane3d` con menos generalidad y una cizalla propia horneada. `prisma`
pasó a tres planos de la escena con `pos=[x,y,z]` (`at=` sigue siendo palabra de colocación,
§8), y entró `lamina`. ⚠️ **El plan decía mal que `fig10-2v3` fuera oráculo**: no usa la
biblioteca, tiene su propio `shear`. El único cliente es `fig2-7b-v3`.

📌 **Y el puerto salió MÁS FIEL al original publicado, no menos.** Una pantalla perpendicular al
haz es un plano y-z, y en las dos proyecciones el eje y va vertical en la página: la pantalla
tiene lados **verticales** y arriba/abajo inclinados, que es lo que muestra `meta/fig2-7b.png` y
lo contrario de lo que producía `plano`. La pieza llevaba años **girada 90° en carácter**
respecto de su fuente, y no se había visto porque no había con qué compararla. La cámara se
**despejó del `.png`** (borde de la pantalla → `angle=44°`; razón del anillo, que vale
`f·cos(angle)` → `foreshorten=0.375`): dos medidas independientes que caen sobre la misma
cámara, lo que no estaba garantizado en un dibujo a mano.

### `tools/ver.sh`, y las dos cosas medidas que codifica

Las ocho compuertas cazan **clases** de fallo; ninguna contesta «¿se ve bien?». **(1)** Un SVG de
MetaGráfica hay que rasterizarlo con un **navegador**: `rsvg-convert` e Inkscape ignoran el
`@font-face` de LM Math y caen a una sans, o sea que mienten sobre la tipografía matemática (una
ρ correcta se ve como «ø»). **(2)** `--diff` es del **mismo formato**: entre backends el
antialiasing deja 6333 px de ruido contra los 524 de un rótulo desplazado 1.4 pt — la señal
queda debajo del ruido, y por eso la paridad entre backends se cuenta por operadores (Capa 3) y
no por píxeles. Dentro de un formato el piso es **0 px exactos**. No sustituye al golden, que es
más sensible: contesta la que el golden no puede, «los bytes cambiaron a propósito, ¿cambió el
dibujo?».

### Tres hallazgos, todos de escribir una figura de verdad

- 🐞 **Un `arc` de barrido CERO tumba el PDF entero** (libharu 0x1051); EPS y SVG lo toleran.
  Es exactamente `to == from`. Importa porque un barrido cero es la salida **natural** de
  recortar por visibilidad y significa «no se ve nada», que es legítimo. Ninguna compuerta lo
  caza: sin PDF escrito no hay golden que comparar.
- **`scale sx sy` a media línea** no toma `sy`; la salida —`scale sx (sy)`— vive solo en un
  comentario del código, y el error dice «variable no definida: shear», señalando una línea
  correcta.
- **La fuente math del SVG no lleva lista de respaldo.** Exposición baja (la ruta de publicación
  es un navegador y sí la carga); mejora de robustez, no bug de salida.

---

## 2026-08-01 — El arco de barrido cero, y la compuerta que no podía verlo

Cierra el primero de los tres hallazgos de ayer, que era el que bloqueaba a los demás: la
figura `angulo_solido.mg` no podía entrar a ningún lado mientras necesitara un `if` para
esquivar un bug del motor.

### El bug: la salida temprana estaba en el renglón equivocado

`arc_bezier` (`src/PDFDisplay.cpp`) abría con `if (sweep == 0.0) return;` **antes** de emitir
el `MoveTo` inicial. Con eso el path quedaba **vacío**, y el `Stroke` que hace quien llama
reventaba con `HPDF_PAGE_INVALID_GMODE` (0x1051) — el manejador de errores de libharu es fatal
a propósito (el primer error deja el documento inválido), así que un arco degenerado en
cualquier rincón de la figura costaba el PDF **entero**, con exit 1 y sin archivo.

El arreglo es mover la salida **debajo** del `MoveTo`/`LineTo`. Así el arco de barrido cero **no
traza nada pero sí deja la pluma en su punto de inicio**, que no es una invención: es lo que ya
hacían los otros dos, y por eso no fallaban. PostScript `arc` con `start == end` añade el punto;
SVG omite el comando `A` de extremos idénticos —lo dice su especificación— y conserva el `M`/`L`
que lo precede. El PDF era el único que se quedaba sin nada que trazar.

Se verificaron los **tres caminos** del constructor de paths, que son estados distintos de
libharu y no uno: arco suelto, arco como **primer** trazo de un `compound` (abre con `MoveTo`) y
arco **en medio** de uno (se une con `LineTo`). Los tres backends salen con la misma estructura
de operadores.

### Lo que costó más que el bug: la compuerta no alcanzaba a verlo

La prueba obvia era un fixture en `test/errors/`, y ahí apareció el problema de fondo: **el
harness de errores compila solo a SVG**. Un bug que vive en PDF le es invisible por
construcción. Y las otras compuertas, menos: sin archivo PDF escrito no hay golden que comparar
ni tres salidas que confrontar en la Capa 3. El bug no estaba en un hueco entre compuertas —
estaba en un punto ciego que **ninguna** cubría.

Se amplió `EXPECT_NO_WARN` para compilar a los **tres** backends. Y solo ése: los fixtures
fatales abortan en el parser, antes de que el backend importe, así que triplicar sus corridas
sería gasto sin cobertura. El razonamiento es que `EXPECT_NO_WARN` afirma *«esto es legítimo y
compila limpio»*, y esa afirmación no está completa en un solo backend. La clase de fallo que
abre —un backend que **aborta** donde los otros dos toleran— es justo la que se le escapa a todo
lo demás.

Verificado por el método de siempre, reintroduciendo el bug: `errfail=1` señalando `[pdf]`, con
los 78 goldens y las otras siete compuertas en verde. Esa asimetría **es** la demostración de
que la compuerta nueva mira algo que las demás no.

### La guarda se quitó, y ésa era la prueba de verdad

`angulo_solido.mg` esquivaba el bug con un `if tc > 0.001` alrededor de sus paralelos. Se quitó:
el SVG sale **byte-idéntico**, y EPS y PDF difieren únicamente en los cinco pares
`gsave`/`grestore` que abría el propio `if` — o sea cero cambio de dibujo, y la diferencia que
queda es la huella del alcance de estado que se fue. La figura ya no pierde su PDF al subir la
elevación (probada a 42°, donde un paralelo queda entero detrás del globo). `arcparity.py` pasa.

Que un paralelo entero oculto dé barrido cero **no es un caso a esquivar**: es la respuesta de la
cuenta. «No se ve nada» es un resultado legítimo de recortar por visibilidad, y el motor tenía
que aceptarlo sin que el autor lo envolviera en un condicional — sobre todo si la figura va a
entrar al corpus, donde el `if` habría quedado enseñando a esquivar un bug.

---

## 2026-08-01 (bis) — Fase D: la referencia aprende 3-D, y `docfail` cobra su primer ejemplo del plan

**§13 «Escenas pseudo-3D»**, nueva y en los dos idiomas. Contenido: la convención de ejes y por
qué es ésa, las dos proyecciones con sus fórmulas y los casos de comprobación que orientan
(θ=φ=0 identidad, φ=90° planta), `plane3d`, `xyz()`, el footgun del doble transform y las piezas
de `lib/pseudo3d.mg`.

### Dónde ponerla, y qué se renumeró

Entró **después de §12 Bibliotecas**, no después de §9 Transformaciones —que es donde encaja
conceptualmente, porque `plane3d` es una transformación—, por una razón concreta: ahí habría
renumerado seis encabezados y nueve referencias cruzadas **en dos idiomas**, y después de §12
renumera tres. La adyacencia con Bibliotecas tampoco es arbitraria: `lib/pseudo3d.mg` se
describe en §12 y ahora se apoya en §13.

Lo que sí hubo que barrer son las **anclas**: diez `#14-errores-comunes` que el renumerado dejó
apuntando a nada. Un enlace roto en Markdown no falla, solo no lleva a ningún sitio — y ninguna
compuerta los mira. (Anotado como candidato barato: `grep -o '(#[0-9]' vs los encabezados.)

Se tocaron además dos sitios fuera de la sección, y los dos importan más de lo que parecen: la
**referencia rápida** de §16, que es la tabla que alguien consulta sin leer nada más, y el «**No
hace 3D**» de §1 — que es de lo primero que lee alguien de fuera y a partir de hoy es medio
falso. Ahora dice qué es y qué no: una cámara que proyecta, sin superficies ocultas ni
iluminación.

### `docfail` cobró su primer ejemplo del plan, y el hallazgo no era de 3-D

El ejemplo `text("CIV") { xyz(3*d, 0, 5*d) }`, copiado tal cual de §4.5 de `plan_pseudo3d.md`,
**no compilaba**. `text` validaba la paridad de sus coordenadas en **parse-time contando
términos**, así que un punto `[x,y]` contaba como **uno** y el diagnóstico decía «número impar de
coordenadas (1)» señalando una línea perfectamente correcta.

Lo revelador es que **no era un hueco del 3-D**: `text("A") { point_at(&p, 0.5) }` fallaba
exactamente igual, y rotular un punto calculado sobre una curva es de lo más natural que se
puede querer. Las primitivas aceptaban puntos desde siempre (`PrimStmt::evalPath`); `text` era
la excepción, y nadie lo había notado porque quien tropieza parte el punto a mano en dos
expresiones y sigue. Se arregló difiriendo la paridad a eval-time, igual que las primitivas.

Eso obligó a **mudar una comprobación de fase**, que es justo cuando una comprobación se pierde
sin que nadie lo note. De ahí los dos fixtures y no uno: `text_punto_calculado.mg` fija que
compile, y `text_impar.mg` fija que el error de coordenada impar **siga existiendo** en su fase
nueva. `coords_impares.mg` vigila el de las primitivas y no habría dicho nada.

📌 El plan había escrito ese ejemplo marcándolo como ilustrativo, y ahí estuvo bien: a
`docs/plans/` no lo compila nadie, y por eso el plan dice explícitamente que la documentación va
en la **referencia**. El valor de la compuerta no es que la documentación esté bien escrita: es
que una afirmación falsa sobre el lenguaje no sobreviva a su primer commit. Un humano tropieza y
desconfía del documento; un modelo obedece.

### Un límite de `docblocks.py`, anotado

Un bloque ```octave **dentro de un blockquote `>`** no se extrae bien: el `> ` se cuela en el
fuente y el compilador se atraganta con él. Salió al documentar de paso la regla `scale sx (sy)`
—el menor que quedaba anotado en `PENDIENTES.md`—, y la salida fue poner esos ejemplos **en
línea**, que es como el resto del documento escribe sus avisos. No se tocó el extractor: hoy no
hay ningún otro bloque en esa posición, y el estilo del documento no la pide.

---

## 2026-08-01 (ter) — La primera figura pseudo-3D entra al corpus

`angulo_solido` es el ejemplo 27: **`ok=81`**, ocho compuertas en verde. Reproduce la fig. II-4
de Lira (proyección del ángulo sólido) y es la única del corpus que ejercita `view3d`, `plane3d`
y `xyz()` — si sale, las tres se quedan sin una sola prueba. Aporta además el caso duro de la
invariante (c) de la Capa 3: sus elipses tienen semidiámetros conjugados **genuinamente
oblicuos** (`u·v ≠ 0`), que es donde los tres backends toman caminos distintos —EPS traza con
matriz, SVG resuelve un SVD 2×2, PDF transforma puntos de control—.

### El detalle que la separaba del original, y por qué el camino evidente no sirve

En la figura publicada un meridiano parte el casquete por la mitad. Conseguirlo **no** es poner
el casquete sobre el plano de un meridiano (`gam = lam`): ese plano corta el disco por su centro
en el ESPACIO, cierto, pero la circunferencia dibujada va a radio 1 mientras el centro del disco
está a `rho = 0.62`, así que el arco pasa **por encima**, descentrado unos 0.29. Es la trampa
natural del problema: la condición correcta en 3-D no es la condición correcta en la página.

Lo que hace falta es que el arco cruce el centro **proyectado**, y eso no es un plano sino un
**punto de la esfera**: el rayo visual que pasa por el centro del casquete la corta en dos
puntos, y el **delantero** —el que no se oculta— fija la longitud del meridiano que hay que
dibujar,

    |C + s·w| = 1   ⟹   s = −(C·w) + √((C·w)² + 1 − ρ²)

📌 **Y lo que midiendo apareció:** el meridiano de 135° **ya pasaba** por el centro proyectado,
con 0.0067 de error, o sea prácticamente exacto — **pero en su mitad oculta**, que es justo la
que la figura recorta. El problema nunca fue la geometría; era de qué lado caía. Sin medirlo, lo
más probable habría sido mover el casquete a ojo hasta que se viera bien, que es exactamente lo
que esta figura existe para no hacer.

**Qué parámetro gastar fue la decisión de modelado:** se movió la **fase de la retícula**, no el
casquete. La fase de los meridianos es libre —nada en la física elige la longitud cero— mientras
que la posición del casquete es el asunto de la figura. Y como `lam0` sale de la cámara, la
propiedad **sobrevive a cambiarla**: verificado a 35°/38°, el meridiano sigue partiendo el
casquete. Un número puesto a ojo no lo haría, y esa es la tesis entera de la figura.

### Ni prefijo ni subcarpeta: la familia se distingue en la galería

Se consideraron las dos formas de marcar que ésta es «de las 3-D». Ninguna se sostuvo:

- **Prefijo `pseudo3d_`** — choca con la regla de nombres del proyecto: los ejemplos se llaman
  por su **asunto**, no por su técnica (`franck_condon`, `turning_points`, `elevacion_solar`), y
  por esa lógica `quickstart` sería `plot_quickstart` y `fig2-1` sería `struct_fig2-1`. Trae
  además un problema de frontera sin final: `orbita_polar` y `gravitacion_orbita` ya son escenas
  espaciales resueltas con trigonometría, y habría que decidir si entran.
- **Subcarpeta `examples/pseudo3d/`** — cuesta cuatro piezas de maquinaria **plana**, y una de
  ellas falla en silencio: `make install` usa `install -m 644 examples/*.mg`, así que no las
  instalaría. Las otras tres son la profundidad `examples/../lib` —que ese layout existe para
  garantizar, y que pasaría a tener dos convenciones conviviendo—, el `glob("*.mg")` de
  `galeria.py` y el `cd "$EXDIR"` de `run.sh`. Todo arreglable; ninguna razón para hacerlo.

La distinción se puso donde un lector la consume: un **grupo editorial** «Escenas pseudo-3D» en
la galería, que es el mecanismo que ya existía para esto y que costó una entrada en `GRUPOS`.
Queda además junto a §13 de la referencia, que es a donde salta quien vea la tarjeta.

---

## 2026-08-01 (quater) — `asin`/`acos`/`atan`, `deg`/`rad`, y la guarda que faltaba en `sqrt`

Salió de una pregunta de Alejandro —«¿valdría la pena implementar `acos` y simplificar el
código de algunas figuras?»— y la respuesta corta es sí, pero **no por lo que la pregunta
suponía**. El ahorro de código es de dos líneas en todo el corpus. La razón real es otra.

### El rodeo no era feo, era inseguro

La referencia **enseñaba** el rodeo en §14: «no hay `asin` ni `acos`, pero `atan2` los
expresa». Medido:

```
q = 1.3
t = atan2(sqrt(1 - q*q), q)     →  <circle cx="-nan" cy="-nan">, exit 0
```

`sqrt` no tenía guarda de dominio —`ln` sí—, así que `sqrt(negativo)` devolvía NaN callando,
`atan2` lo propagaba y las coordenadas salían `-nan` con **código 0 y archivo escrito**. Es
exactamente el modo de falla que `evalError` se volvió fatal para eliminar el 2026-07-15,
sobreviviendo en este rincón. En la prueba solo lo delató el aviso de lienzo en blanco, y
porque *todo* era NaN; con un punto malo entre cien no habría dicho nada.

O sea que la documentación estaba recomendando la forma insegura de escribir un ángulo. Eso es
lo que decidió el cambio, no la legibilidad.

Entran entonces **`asin`, `acos`** (con guarda de dominio al estilo de `ln`), **`atan`** de un
argumento —`atan2(y,1)` lo expresa, pero quien lo busca lo busca por su nombre— y la **guarda de
`sqrt`**, que es la que cierra la clase entera y no solo este rodeo. Se eligió error DURO y no
tolerar un −1e−16 de redondeo: la cultura del proyecto es que el autor acote a mano cuando la
geometría lo pide (el `clamp` de `angulo_solido` es justo eso, y **es** geometría, no un parche).
El corpus entero pasa con la guarda puesta, que es la evidencia de que no hay falsos positivos
hoy.

### `deg` y `rad`, con la constante del motor

Quince conversiones `* 180 / pi` escritas a mano en el corpus, **diez en una sola figura**.
Entran `deg` y `rad` usando `deg2rad` de `matrix.h` —la misma constante que usa el compilador,
no una copia—, que fue la condición que puso Alejandro y resultó ser más que higiene.

### Lo que destapó: el nº de segmentos de un arco lo decidía el ruido

Al portar `angulo_solido` a `deg`/`rad` la salida se movió, y no por donde se esperaba. El SVG
salió **byte-idéntico** en las dos figuras; el PDF de `angulo_solido` no: un meridiano pasó de
**3 segmentos de Bézier a 2**.

La causa es que `arc_bezier` usa `ceil(|barrido| / 90)`, de modo que un arco de **exactamente
180°** cae justo en la frontera. Con `v0 = psi − 90` y `v1 = psi + 90`, la resta
`(psi+90) − (psi−90)` **no** da 180 exacto en coma flotante para cualquier `psi`: daba
`180.00000000000003`, y ese ε de más pedía un tercer segmento. Con `deg()` el barrido sale
exacto y quedan dos, que es lo que la regla pretende.

Medido con `tools/ver.sh --diff` (mismo formato, piso de ruido 0 px): **471 px**, todos
antialiasing a lo largo de esa única curva, sub-píxel. El dibujo es el mismo. Anotado en
`PENDIENTES.md` junto al ítem de las constantes de Mortensen, porque es del mismo vecindario:
si algún día se toca `arc_bezier`, ahí está el argumento de que esa frontera merece un epsilon.

📌 Vale la pena quedarse con la forma del hallazgo: **el tercer segmento era un accidente**, y
por casualidad daba un poco más de precisión que la regla. Nadie lo habría encontrado buscándolo;
apareció porque un cambio que se creía cosmético movió bytes y hubo que explicar por qué.

### Las pruebas

Cuatro fixtures nuevos (`err_ok` 47 → 51): `dominio_acos`, `dominio_asin` y `dominio_sqrt` fijan
que aborten con su mensaje —van los tres porque son **tres guardas distintas** en el código y
una podría perderse sin que las otras lo noten—, y `deg_rad` fija que el par exista y compile en
los tres backends, cerrando el círculo dentro del propio fixture (`deg(rad(x))` devuelve `x`).

La referencia perdió el ⚠️ que enseñaba el rodeo y ganó, en los dos idiomas, la lista nueva de
funciones, el puente grados/radianes y un aviso de que las cuatro funciones con dominio
**abortan** — con el porqué, que es que un NaN en una coordenada no hace ruido.

---

## 2026-08-01 (quinquies) — `sine` deja de tragarse sus atributos, enrutándola por `PrimStmt`

Lo destapó `onda_3d.mg`, el ejercicio preliminar de la onda electromagnética:
`sine(half_cycles=1, amplitude=1, fill="#ffcdcd")` compilaba sin una queja y salía
`fill="none"`. Y no era solo `fill`: `parseSineArgs` aceptaba **cualquier** nombre sin
validarlo, y `SineStmt::exec` solo leía la geometría y empujaba una `Polyline` pelada, así que
`color=`, `line_width=`, `dash=` y `marker_*=` corrían la misma suerte.

Es **la misma clase de bug que el proyecto ya había cerrado dos veces** —el no-op mudo del
argumento nombrado, que hace que un typo parezca puesto y no haga nada—. `sine` se libró de
aquellas dos pasadas por una razón estructural y no por descuido: **tiene parser propio**
(`parseSine`), no pasa por `PrimStmt`, y por eso no heredó `isKnownPrimAttr`. Vale la pena
anotarlo como patrón: cuando un constructo se sale del camino común, hereda los bugs que el
camino común ya arregló.

### La decisión, y por qué (a) y no (b)

Había dos salidas: **(a)** que `sine` honre los atributos como todas las demás, o **(b)** que
rechace los que no entiende. Alejandro eligió (a), que era también lo que apuntaba la evidencia:
`bezier(&p, fill=)` **sí** los honra, y tener dos conductas para la misma curva —una que rellena
y otra que no— es lo confuso.

### Cómo, sin duplicar el aparato

En vez de copiar en `SineStmt` las diez líneas de estilo de `PrimStmt`, `parseSine` **arma un
`PrimStmt`** de nombre `"sine"` cuyo `pathArg` es la onda. La pieza que lo hace barato ya
existía: `PathSine`, el generador que servía al álgebra de trayectos (§9). Los nombrados se
reparten en el parser —`half_cycles`/`amplitude`/`phase`/`squared` al generador, el resto a
`PrimStmt`— y `SineStmt` desaparece.

Con eso `sine` hereda de una vez tres cosas y no una: el estilo por-primitiva **con su alcance**
(`gsave`/`grestore`), el **`closed=`** —que es justamente lo que vuelve rellenable una onda— y
la **validación** contra la lista blanca. Ese reparto es además lo que hace que un
`half_cicles=` mal escrito ya no se pierda: lo que no es geometría llega a la lista blanca y se
rechaza por su nombre.

**Cero churn en los 81 goldens**, que es la comprobación de que el enrutado no cambió el camino
sin atributos: cuando no hay ninguno, `attrs` queda vacío y se emite exactamente lo de antes.

### La cobertura, que son dos cosas distintas

Una compuerta de errores puede fijar que el typo se rechace, pero **no** que el atributo surta
efecto —para eso hace falta un golden—. Así que van las dos: `test/errors/
sine_atributo_desconocido.mg` para el rechazo, y **`sines.mg`** —la lámina de referencia de
`sine`, su casa natural— gana un grupo con una onda **rellena** (`fill=` + `closed=`), una
**morada gruesa** (`color=` + `line_width=`) y una **discontinua** (`dash=`, que hereda el verde
ambiente: el ejemplo enseña de paso que un atributo por-primitiva anula uno solo del estado, no
todos). `docs/img/sines.svg` se regeneró: es cara pública.

📌 **Queda abierto el otro hallazgo de `onda_3d`:** `polygon(&p)` sobre una curva generada toma
los puntos de control como vértices y rellena una cometa. La conducta es defendible; lo que
falta es una línea en §10 y otra en §13 diciendo que para rellenar una curva generada va
`bezier(&p, fill=)`.

---

## 2026-08-01 (sexies) — `onda_electromagnetica` al corpus, y el ejercicio se descarta

Segunda figura pseudo-3D del corpus: **`ok=84`**, 28 ejemplos. Reproduce la fig. II-1 de Lira —E
y B en planos perpendiculares— y cubre exactamente lo que `angulo_solido` no: un **generador**
(`sine`) y un **relleno** dentro de un `plane3d`, que es la promesa de §13 de la referencia —«ahí
dentro funciona el dibujo 2-D de siempre»— que hasta hoy no tenía una sola prueba, y
**marcadores bajo una matriz no conforme**.

Esto último merece anotarse porque era una pregunta abierta, no una casilla: los marcadores bajo
una matriz que no conserva ángulos son de la familia de `plan_anisotropia.md`, la más recurrente
del proyecto. **Sale limpia**: las puntas de los peines de E salen verticales —su dirección es
`proj(ŷ) = (0, cos φ)`— y las de B arriba-derecha, `proj(−ẑ)`. Ahora hay una figura del corpus
que lo vigila.

### Las dos amplitudes son distintas, y el factor se deriva

El plano de B es el horizontal, que la cámara escorza. Con la misma amplitud que E, B saldría
más corto y la figura **diría algo falso**: que un campo es menor que el otro. Se pide que las
dos se vean igual de largas y el factor se despeja de la cámara:

    ampb = amp · cos(el) / hypot(sin(az), cos(az)·sin(el))

Lo comparable en el dibujo es la **longitud**: E y B tienen unidades distintas y su razón física
no es dibujable, así que igualar lo que se ve es la lectura honesta. Y como sale de la cámara,
sigue valiendo si se cambia — la misma propiedad que `angulo_solido` con la fase de su retícula.

📌 **La cota de λ va DENTRO del plano de E, no en la página.** Así queda paralela al eje —que
desciende con esta cámara— y sus dos patas miden lo mismo sin una sola cuenta: bajo `plane3d` un
segmento vertical del plano se proyecta con la misma longitud sea cual sea su x. Verificado
porque *pareció* que no: a ojo la pata izquierda se veía más larga, y midiendo en el SVG salieron
26.458 y 26.457 pt. La lección es la de siempre al revés — mirar destapa defectos, pero también
inventa alguno; el que sobrevive es el que se mide.

### El ejercicio se descarta, habiendo servido

`onda_3d.mg` (de `waves.png`) se escribió como ejercicio preliminar y **se borró** al existir la
figura buena: era la misma escena sin peines, sin cota y sin rótulos. No se pierde nada porque lo
que valía ya está capitalizado en otra parte — la cámara medida quedó en la bitácora, y los **dos
hallazgos de motor que produjo** (que `sine` se tragaba sus atributos, y que `polygon(&p)` sobre
una curva generada rellena una cometa) están arreglado el primero y documentado el segundo.

Es el mismo criterio de la política V1: antes de borrar, lo que debe sobrevivir son las
**medidas**, no el archivo.

### De paso, el segundo hallazgo cerrado

§10 de la referencia gana, en los dos idiomas, el aviso de que para **rellenar una curva
generada** va `bezier(&p, fill=)` y no `polygon(&p, fill=)`. Lo que lo hace digno de un ⚠️ y no
de una nota al pie es que **nada avisa**: las dos son lecturas legítimas de la misma lista de
puntos —vértices contra puntos de control—, así que el error no falla, dibuja otra cosa.

---

## 2026-08-01 (septies) — `irradiancia`: la silueta no necesitaba motor, y el ángulo se dibuja donde vive

Tercera figura pseudo-3D del corpus (`ok=87`, 29 ejemplos), y con ella la **estrategia E** de
`plan_pseudo3d.md` queda resuelta. ⚠️ **El plan decía que E pedía «geometría nueva» en el motor.
Era falso.**

### La derivación, que cabe en veinte líneas de `.mg`

La proyección restringida al plano de la base es una **afinidad**, y la tangencia es invariante
afín. Así que no hay que resolver «tangente a una elipse desde un punto» en la página: se
**retro-proyecta el ápice al marco de la base** —un 2×2— y allí el borde vuelve a ser un
**círculo**, donde la tangencia es de secundaria: `θ = atan2(b,a) ± acos(r/D)`.

📌 Lo que hace la construcción limpia es el paso final: los puntos de tangencia se devuelven al
ESPACIO y las generatrices se dibujan con `xyz()` en sus dos extremos, **porque son rectas de
verdad del cono**. La retro-proyección solo sirvió para encontrar el ángulo, no para dibujar.

El cilindro que pedirá `fig18-5` es el hermano fácil: dos círculos iguales en planos paralelos
comparten marco, así que las tangentes comunes tocan en `atan2(s) ± 90°`, sin `acos` siquiera.

Verificado **dos veces**: numéricamente sobre seis configuraciones aleatorias —cámara, ápice,
centro y una base `u,v` deliberadamente no ortonormal— la elipse nunca cruza la generatriz
(≥ 3e−10) y la toca (≈ 2e−9); y **dentro de la propia figura**, porque `dΩ` es la sección del
cono a una fracción del recorrido y sale tangente por construcción. Si la derivación se rompe,
deja de estarlo y se ve.

⚠️ **Y la pieza que lo destrabó llegó por otro camino:** `acos` se añadió esa misma mañana
porque el rodeo `atan2(s, sqrt(1-s*s))` que la referencia enseñaba producía `-nan` en silencio.
Cuando se escribió la Fase F, `acos` no existía — que es probablemente por qué E parecía pedir
motor.

### El ángulo se dibuja en su plano, y eso es mejor que el original

El original marca φ con una **flecha doble plana**, en el papel. Eso miente un poco: el ángulo
entre dos direcciones proyectadas no es la proyección del ángulo del espacio. Aquí el arco vive
en el plano que contiene a n̂ y al eje, con marco `u = n̂` y `v =` la componente del eje
perpendicular a n̂ — y entonces se escribe `from=0 to=fid`, en grados y sin convertir nada.

📌 **La diferencia se midió sin buscarla.** Al colocar el rótulo de φ en la bisectriz **del
espacio**, cayó encima de la flecha de n̂: la proyección tampoco conserva la bisección de un
ángulo. Es la misma razón por la que el arco tiene que ser geométrico. El arco mide; el rótulo
solo señala, así que ése sí va en la bisectriz de la página — y bisecando contra la
**generatriz** más cercana, no contra el eje, porque el hueco libre lo bordea el cono (medido
desde P: n̂ a 90°, generatriz a 75.7°, eje a 63.5°).

### Dos veces la misma lección de método

Durante esta figura leí mal los píxeles **dos veces** y la medición me desmintió las dos: las
patas de la cota de λ (que parecían desiguales y median 26.458 y 26.457 pt) y el final del arco
de φ (que parecía terminar en la generatriz y termina en el eje, 63.540° los dos).

Es el contrapeso a la lección de `ver.sh`: **mirar destapa defectos que las compuertas no ven,
pero también inventa alguno**. El que sobrevive es el que se mide.

---

## 2026-08-01 (octies) — `\hat` y `\vec`, dibujados; y §13 gana su regla de decisión

Dos cosas, y van juntas por el archivo que comparten (la referencia), no por el tema.

### La tabla de §13: la referencia tenía los hechos y no la regla

Alejandro preguntó si las tres figuras pseudo-3D estaban bien explicadas para alguien de fuera.
Medido: **§13 no hacía ni una referencia a un ejemplo**, contra **22** en el resto del documento
— justo la sección donde un ejemplo trabajado más falta. Y la regla de decisión (qué alcanzar
para cada pieza de la figura) existía, pero vivía en `docs/plans/plan_pseudo3d.md`: material de
mantenedor, que ninguna compuerta compila y que `make install` no reparte.

Es **literalmente el hallazgo de la condición 4** del 2026-07-28: todos los hechos, ninguna
regla de decisión, y el modelo eligiendo la herramienta equivocada. §13 contestaba «cómo se
escribe `plane3d`», no «qué uso para esto». Entró una tabla de seis filas —cada una con el
ejemplo que la demuestra— y un «por dónde empezar». Ahora §13 cita a los tres (2/5/2).

### `\hat` y `\vec`: el acento se dibuja, y hay una razón

`irradiancia` entró al corpus con el sombrero de n̂ hecho a mano: una polilínea de tres puntos
con desplazamientos literales. Geometría puesta a ojo haciendo de tipografía, en la figura que
presume de no medir nada a ojo.

**Lo que decidió el diseño fue un dato, no un gusto.** Meter U+0302 al subset de LM Math no
habría bastado: un acento combinante tiene **avance cero** y se posiciona con las tablas
GPOS/MATH de la fuente. MG no tiene motor de shaping —coloca glifos por ancho de avance— así
que el código de posicionado hay que escribirlo **por los dos caminos**. Lo único que cambia es
de dónde sale la forma de la marca. Siendo así, la geometría cuesta estrictamente menos: no
toca el subset, no toca la ruta PUA del PDF, y escala con `font_size` por construcción.

Es además el criterio que el proyecto ya había aplicado: la raya de `\frac` es un trazo
(`Display::fracRule`), no un glifo — como en TeX, que también la dibuja.

**Cómo:** `Accent` es una `Fraction` con un hijo. Reusa toda la mecánica que `\frac` construyó
—composición inline con neto cero alrededor del hijo, medición de extent vertical, `TextLine`
como contenedor— y añade `Display::penSegment`, que es `fracRule` generalizado a un trazo
cualquiera. Un circunflejo son dos segmentos; una flecha, tres.

📌 **La marca se dimensiona por el ancho de la base**, así que sobre una letra ancha sale ancha
— lo que en TeX obliga a pedir `\widehat` aparte.

⚠️ **Y una heurística acotada, que conviene conocer antes de tocarla.** `childVExtent` usa
`kGlyphAscent`, que es altura de MAYÚSCULA, porque no hay métricas verticales por glifo. Sobre
una minúscula sin ascendente —`n̂`, `r̂`, `v̂`: justo el caso de la física— eso dejaba la marca
flotando un cuarto de em de más. La corrección baja a la altura de la equis **solo** cuando la
base es una letra suelta, sin script y de las que no suben; cualquier otra cosa conserva el
extent completo, que nunca queda corto.

**Cero churn** en los 87 goldens salvo `irradiancia`, que es donde se cobró el premio: el
sombrero dibujado a mano se fue y quedó `text("$\hat{n}$")`.

La familia se queda en dos a propósito. `\bar`, `\tilde` y `\dot` entran cuando una figura los
pida — la regla de demanda de siempre.

---

## 2026-08-01 (nonies) — `cono` y `cilindro` a la biblioteca, y una compuerta que probaba la máquina

Extraídas a `lib/pseudo3d.mg`, **después** de que `irradiancia` probara la receta en los tres
backends y no antes: es el método que funcionó con `plane3d`, donde la abstracción nació como
abreviatura de algo que ya andaba.

`cono` y `cilindro` toman `axis`, el vector que va de `pos` al otro extremo — lleva dirección y
longitud juntas, así que no hay parámetro de altura. El cilindro resultó el hermano fácil: dos
círculos iguales comparten marco, así que sus tangentes comunes tocan a `atan2(s) ± 90°`, sin
`acos`.

### La extracción se verificó portando la figura, y luego se revirtió

`irradiancia` se reescribió sobre `cono(...)` y `ver.sh --diff` dio **16 px**, que resultaron ser
**dos motas en los puntos de tangencia**: ahí se cruzan la elipse de la base y las generatrices, y
lo único que cambió fue el orden de pintado. Geometría idéntica ⇒ extracción fiel.

Y aun así se **revirtió**, por una razón que conviene registrar: `irradiancia` necesita los
puntos de tangencia para colocar el rótulo de φ, y **una struct de MG no puede devolver lo que
calculó**. Llamando a `cono` tendría que recalcularlos igual, así que la figura se quedaría con
la cuenta *y* la llamada. Se queda inline, que además es lo que §13 promete al mandar ahí «para
ver la derivación»; la biblioteca sirve a quien solo quiere la forma.

⚠️ **Consecuencia: `lib/pseudo3d.mg` sigue sin cliente en el corpus.** Cuatro piezas que ninguna
figura compila. Anotado en `PENDIENTES.md`; lo cerrará `fig2-7b` o `fig18-5` cuando alguna entre.

### Y el hallazgo de verdad: `docfail` estaba probando la máquina, no el repo

Al documentar las piezas nuevas, el bloque ```octave de §12 **falló** diciendo que `prisma` no
tiene `pos=`. Pero sí lo tiene — desde la Fase C.

La causa: `docblocks.py` compila el bloque en un directorio temporal y copia ahí las
bibliotecas, de modo que `include "pseudo3d.mg"` (nombre a secas) resuelve. Pero
`include "../lib/pseudo3d.mg"` —la forma que usa un ejemplo del corpus, y que la referencia
enseña— **no resolvía**, y caía a la búsqueda instalada (`-DMG_LIBDIR`). Estaba compilando
contra `/usr/local/share/metagrafica/lib/`, una copia de dos días antes con el `prisma`
**anterior** a la Fase C.

O sea que la compuerta habría **bendecido una API que el repo ya no tiene**, y en una máquina sin
`make install` habría fallado por no encontrar el archivo. Ninguna de las dos cosas tiene que ver
con lo que el bloque afirma.

Arreglado copiando las bibliotecas **dos veces** —al lado del bloque y en un `lib/` hermano— y
compilando el bloque en un subdirectorio, para que las dos formas de `include` resuelvan contra
el árbol. La prueba está en las dos corridas: antes fallaba contra el `prisma` instalado, después
pasa contra el del repo.

📌 Es la tercera vez en el día que una compuerta resulta estar mirando el sitio equivocado (las
otras dos: el harness de errores compilando solo a SVG, y §13 sin una sola referencia a un
ejemplo). El patrón que las une es que **ninguna fallaba**: había que preguntarles qué estaban
mirando.

---

## 2026-08-01 (decies) — `fig2-7b`: el original no es coherente consigo mismo, y se queda fuera

Alejandro señaló que las dos piezas pseudo-3D de la figura están simuladas de formas distintas, y
que la pantalla no es un paralelogramo en un espacio afín. **Las dos cosas son ciertas, y se
midieron.**

### La medición

Con las dos aristas verticales de la pantalla bien localizadas (mi primer intento las buscó con
extremos de `x±y` y capturó otra tinta — dio 0.845, y **estaba mal**):

| | |
|---|---|
| lado lejano | 415 px |
| lado cercano | 602 px |
| **razón** | **0.689** (un paralelogramo daría 1.000) |
| aristas superior / inferior | −28.54° / +28.80° — **convergen** |
| **punto de fuga** | **(353, 366)** |
| el cristal ocupa | x 300..439, y 320..449 |
| centro de la pantalla | y = 368 |

📌 **El punto de fuga cae DENTRO del cristal, a la altura exacta del centro de la pantalla.** La
pantalla está construida como la ve el punto de dispersión: la perspectiva no es adorno, es el
cono de difracción abriéndose. Y el cristal, en cambio, es un paralelepípedo afín.

O sea que **el original mezcla dos geometrías proyectivas en la misma figura**. Es el hallazgo de
la Fase C un nivel más abajo —allí eran dos *cámaras* distintas (73.3° contra 35.0°), aquí son
dos *geometrías*— y de él se sigue que **ninguna escena coherente puede reproducirlo**. Eso es
propiedad de la fuente, no límite de MG.

### Se intentó la versión afín, y quedó mejor

Midiendo razones contra el alto de la pantalla, el port tenía: la pantalla corta para la escena
(1.06 contra 0.885 del original), poco profunda (0.238 contra 0.284), las láminas **más bajas**
que la pantalla cuando el original las tiene **más altas** (0.80 contra 1.055), el lienzo
demasiado ancho (1.717 contra 1.243), el cristal chico, gris y sin girar, y dos rótulos
encabalgados. Todo eso se corrigió.

Para el giro del cristal, **`prisma` ganó orientación**: `ex`/`ey`/`ez`, las direcciones de sus
tres aristas, en el mismo estilo con que `plane3d` toma `u` y `v`. Se dan como vectores y no como
un ángulo **porque un ángulo obliga a elegir alrededor de qué**, y la respuesta depende de la
figura. El giro tampoco es adorno: la lámina es *poli*cristalina, así que el cristalito orientado
al azar es lo que la figura dice.

**Decisión de Alejandro: no entra al corpus.** Queda una sola diferencia y es la que un motor
afín no puede dar. Lo pulido se conserva en `local/`; si algún día entra, entrará por la figura y
no por cobertura. El hueco de `lib/pseudo3d.mg` sigue abierto.

### El bug que salió por el camino, y por qué no se arregló

Los rótulos del port salían en redonda contra la itálica del original. La causa: **`font
"italic"` como SENTENCIA es un no-op mudo**. `text(..., font="italic")` funciona; la sentencia
no, y **no avisa** (`font "noexiste"` sí avisa), así que dos maneras de escribir lo mismo dan
resultados distintos y una calla.

Causa exacta: `TextStmt::exec` hornea `FN_DEFAULT` cuando no hay `font=` por-primitiva, y eso
gana sobre la cara ambiente; solo se hereda con `FN_NOFACE`, que es como nacen los rótulos de
`axis`/`legend`. El comentario del código lo justificaba como «idéntico al comportamiento
previo» — inercia, no decisión.

⚠️ **El arreglo obvio se probó y NO es seguro:** cambiar el default a `FN_NOFACE` da `fail=30`,
`c3fail=2` e `imgfail=9`. Y no viene de los tres ejemplos que usan la sentencia —`fig4-4`,
`symbols` y `turning_points` piden `roman`/`Times-Roman`, o sea el default, así que ahí el no-op
es invisible—. Viene de otro sitio, y **que rompa la paridad entre backends** es lo que obliga a
averiguar de dónde antes de tocarlo. Revertido y anotado.

En la figura se resolvió por la vía correcta y de paso mejor: los rótulos son símbolos, así que
van en **modo matemático** (`$L$`, `$O$`, `$R$`, `$P$`) y salen de LM Math — que es exactamente
la itálica del original, no Times-Italic.

---

## 2026-08-03 — `multietapa` al corpus: una biblioteca sin ejemplo no la compila nadie

`examples/multietapa.mg` (reconstrucción de la fig. 1.25 de Lillesand, Kiefer & Chipman:
el mismo punto del terreno visto desde satélite, avión alto, avión bajo y suelo) entró al
golden. `ok=87 → **ok=90**`, con las ocho compuertas en cero.

**Por qué entra, si no ejercita nada nuevo.** Se revisó característica por característica y
no aporta ninguna en exclusiva: `include` de biblioteca ya está en `gravitacion_orbita`,
`polygon` en `irradiancia`/`path_sample`/`primitives`, `dashdot` en `line_patterns` y
`angulo_solido`, `gray()` en tres más, el texto multilínea (`/n`) en cuatro. Entra por la
razón de `elevacion_solar`: es el **único usuario de `lib/aircraft.mg` y `lib/people.mg`**,
los dos iconos que llegaron el 2026-08-02 (`c6495e2`).

📌 **Una biblioteca de `lib/` sin ejemplo que la incluya no la compila ninguna compuerta.** Se
pudre en silencio y el aviso llega el día que alguien la usa —o sea, en la peor sesión posible,
la de quien la estrena—. `lib/` es código publicado por `make install` y ninguna de las ocho
compuertas mira un `.mg` de ahí directamente: solo lo alcanzan **a través** del ejemplo que lo
incluye. Con esto, de los ocho `.mg` de `lib/` solo `pseudo3d.mg` sigue sin usuario en el
corpus — el hueco que dejó abierto la sesión del 2026-08-01 (decies).

**Lo que hubo que tocar, y lo que no.** Cero motor: el ejemplo compilaba ya a los tres
backends. Los `include` pasaron de `"satellite.mg"` a `"../lib/satellite.mg"`, que es la forma
que resuelve igual en el árbol e instalada (los ejemplos son hermanos de `lib/` a propósito).
Se añadió `multietapa` a `$EXAMPLES` en `test/run.sh` —lista explícita, no glob—, se generó
`docs/img/multietapa.svg` **a mano la primera vez** (la compuerta `imgfail` itera sobre
`docs/img/*.svg`: la presencia del archivo ES la declaración, así que `images` no puede crear
el primero) y se le escribió su tarjeta inglesa en la tabla `TRAD` de `tools/galeria.py`, en el
grupo editorial «Ilustraciones y diagramas».

**Dos afirmaciones falsas en el encabezado, cazadas al publicarlo.** Las NOTAS citaban
`barredor_mecanico.mg` como precedente de la rejilla en perspectiva, y ese archivo **no existe
en el repo** — es de otro árbol. Se reescribió la frase para que se sostenga sola. Y el título
y la descripción venían sin acentos (`Observacion multietapa`, `satelite`, `MAS detalle`):
inofensivo en un comentario, salvo que esas dos primeras líneas son justo lo que `galeria.py`
**publica**. Es la misma clase de la limpieza del 2026-07-23: un encabezado mal formado sale
publicado, y `galfail` vigila que la galería esté al día, no que el encabezado esté bien.

**Verificado mirando** (`tools/ver.sh`, rasterizado con Chrome): los tres formatos coinciden y
la figura se lee. Las dos aeronaves son de clase distinta —reactor arriba, avioneta abajo— y
esa es media figura: antes eran la misma silueta y solo los rótulos separaban «gran altitud»
de «baja altitud», o sea que la figura afirmaba dos plataformas y dibujaba una.
