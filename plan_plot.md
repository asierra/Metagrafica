# Plan para graficado de datos (`plot`) en MG V3

Objetivo: convertir el patrón "meter datos científicos en una caja de ejes" en algo
declarativo y de baja fricción, **sin romper la filosofía 2D del lenguaje** (ejes como
`place` a lo largo de una línea, marcadores físicos, todo en el compilador). El caso
guía es `examples/fig6-4v3-clean.mg` (Geiger-Nuttall: x lineal, **y logarítmico**),
complementado por `fig2-3` (dos ejes lineales anisótropos) y `fig4-10` (paneles).

Este plan **no arranca de cero**: la spec ya diseñó §13.5 (`axis` declarativo) y §13.7
(marco de datos + azúcar `plot { }`). Aquí se prioriza, se aterriza en la API real
(`src/parserv3.cpp`) y se define un **prototipo** concreto. Léase junto con:
`especificacion_mg.md` §13.5 (tabla de args + "Propuesta para estudiar") y §13.7
(marco de datos, `plot { }`), y `plan_polybar.md` (barras, hermano de este).

## Punto de partida (qué YA existe)

- **`axis` declarativo (§13.5), funcionando.** `AxisStmt` en `parserv3.cpp:1503`: línea
  + marcas (`out`/`in`/`both`/`none`/`grid`) + etiquetas numéricas + título (con
  rotación §19 en ejes verticales). `from`/`to`/`step`/`start`/`decimals`/`tick_size`/
  `label_gap`/`title`/`title_font`/`title_size`. La posición de cada valor sale del
  lambda `posOf(v)` (interpolación lineal `p1..p2`), y los offsets físicos de `physOut`.
- **Marco de datos (§13.7), núcleo resuelto.** `world_window` en unidades de datos +
  `stretch=true`; marcadores **físicos** `dot`/`marker` (§4.6) inmunes al stretch;
  `fit(Struct, stretch=true)` como la misma idea sobre struct reutilizable. Realizado en
  `fig2-3.mg`.
- **El mecanismo "hornear el mapeo en los puntos"** ya existe en la rama path de `fit`
  (`parserv3.cpp:906`: `process_path(M, path)` aplica la matriz a los puntos y emite la
  polilínea ya transformada). Es la semilla arquitectónica de `plot` (ver Fase 4).
- **`compound` como precedente de parser** (`CompoundStmt`, `parserv3.cpp:1617`):
  constructo con args nombrados + bloque de **sentencias** (loop de `parseStatement`).
  `plot` se parsea igual.
- **La API para remapear puntos YA es pública** (auditado 2026-07-14, ver "Huella en el
  motor" en la Fase 4): `GraphicsItemWithPath` (`primitives.h:228`) expone
  `getPath()`/`setPath()` y de él heredan `Polyline`/`Dot`/`Polybar`/`Rectangle`/`Arc`; la
  discriminación sin RTTI se hace con `getType()`. **Consecuencia: `plot` no necesita
  elementos gráficos nuevos** — su huella en el motor es UN accesor
  (`GraphicsState::getPosition()`, hoy ausente).
- **Precedente que zanja el punto:** `axis` (log, ticks, alineación, títulos rotados) se
  implementó **sin añadir una sola primitiva al motor**. `plot` es del mismo tipo de
  constructo.
- **`grid` (§13.6)** y **`numbers`/`ticks` (§13.1-2)** como generadores de más bajo nivel.

**Corrección al plan anterior:** las ventanas anidadas (§16) **no están implementadas**
en V3 (`world_window` dentro de un bloque pisa la ventana global vía `mg.setWindow` y
`BlockStmt` no la restaura; §16 sigue en pendientes). `plot` por tanto **no** puede
desazucarar a "ventana anidada + stretch": su realización es la de `fit` (transformación
horneada/envolvente), que además es la única compatible con escala log (abajo).

## Aprendido y ARREGLADO en la sesión de fig6-4 (2026-07-14)

Al portar `fig6-4` (ver `fig6-4v3-clean.mg`, banco de pruebas de este plan) salieron
dos bugs del motor que bloqueaban el caso "plot en EPS" — **ambos corregidos**, red
golden intacta (`ok=32`):

1. **`/undefined in ellipse` (EPS).** `fit(stretch=true)` sobre una struct con
   `circle(...)` produce elipses al dibujar (`EPSDisplay::arc` compara normas de columna),
   pero el proc `/ellipse` solo se definía según la bandera de parse-time
   `flags.using_ellipse`, que no cubre ese caso. **Fix:** `EPSDisplay::arc` define
   `/ellipse` en su **primer uso** (miembro `ellipse_defined`). Desacople draw-time /
   parse-time resuelto de raíz, sin ampliar la heurística de parse (que habría metido un
   proc inútil en casi todo documento con `fit(stretch)`, que es ubicuo).
2. **Rótulos de `axis` en blanco (EPS).** `axis`/`numbers`/`grid` emiten etiquetas con
   `FN_NOFACE` ("hereda ambiente") y `Text::draw` **omite `setFontFace`** para `FN_NOFACE`
   (`text.cpp:416`); si el documento nunca fijó `font`, EPS hace `show` sin fuente actual
   → invisible (solo-EPS; SVG/PDF ok). **Fix:** guard en `EPSDisplay::text()` — si nunca
   se emitió fuente (`dev_size<0`), fija la cara vigente o Times-Roman. Cero churn.

**Lección para `plot`:** un plot debe (a) usar marcadores **físicos** por default (no
`circle`, que se deforma bajo stretch — el bug #1 nace de usar `circle`), y (b) no
depender de que el autor recuerde poner `font` antes de los ejes.

## Lecciones de ingeniería (durables — aplican a TODA feature de axis/plot)

Las Fases 1–1.5 y los ajustes de fidelidad a fig6-4/fig2-3 dejaron un patrón claro de
dónde se esconden los bugs. Se codifica aquí porque se repetirá en Fases 2–5 y en `plot`:

1. **Un fix de estado-de-dispositivo hay que reflejarlo en los TRES backends el mismo
   día.** Tres bugs de esta clase salieron por arreglar EPS y olvidar PDF (o al revés):
   `/ellipse` (parse-time), texto en blanco con `FN_NOFACE` (`dev_size<0` en EPS /
   `current_font` null en PDF — este último vivió meses hasta 2026-07-14), y el prólogo
   `/cshow`. **Regla:** al tocar fuente, color, alineación o prólogo de un backend, revisar
   los otros dos ANTES de cerrar.
2. **Trampa del prólogo EPS (bandera parse-time).** EPS escribe procedimientos (`/cshow`,
   `/rshow`, `/ellipse`) en `start()` **solo si** la bandera `g_flags.using_*` está en
   true. `exec()` corre ANTES de `start()`, así que cualquier `AxisStmt`/`PlotStmt` que
   emita `AT_TALIGN≠0` (o elipses) DEBE activar la bandera en exec. Olvidarlo = `undefined`
   en Ghostscript, no un error de compilación (no lo caza el build, sí `gs nullpage`).
3. **El golden cubre EPS+SVG, NO PDF.** El bug de etiquetas en blanco en PDF vivía justo
   en ese hueco. Toda feature de texto/eje necesita un **spot-check de PDF** (render +
   `pdftoppm` + vista), además de `gs -sDEVICE=nullpage` sobre el EPS y del golden.
4. **Marcadores/tamaños FÍSICOS en contexto de datos.** `circle` bajo `fit(stretch)` se
   deforma en elipse (origen del bug `/ellipse`); `dot` no. En `plot`, el mapper solo debe
   tocar *puntos*, nunca radios / `line_width` / `font_size` / `tick_size` / `label_gap`.
5. **La fidelidad a una figura-oráculo real destapa requisitos que la spec abstracta no
   ve.** `extend`, `label_gap`/`title_gap`, ticks hacia adentro y el centrado del título
   **no** estaban en el plan original: salieron de comparar pixel a pixel contra
   `fig6-4.png` y `fig2-3v1.eps`. **Cada fase se cierra portando una figura del libro
   extremo-a-extremo contra su oráculo, en los tres backends** — no con un caso sintético.

## TAREA — Automatizar las compuertas de la Lección 3 en `test/run.sh`

> **Alcance:** esto **trasciende el graficado** (es infraestructura de test del motor);
> vive aquí porque nace de la Lección 3. Migrar a `CLAUDE.md`/un plan de test si crece.

Hoy `test/run.sh` compara **solo EPS+SVG por bytes**; el PDF y la validez PostScript se
revisan **a mano**. Los tres bugs de esta línea vivieron justo en ese hueco. Tres capas,
**cada una caza una clase distinta** — ninguna sustituye a las otras:

**Capa 1 — PDF a la red golden. Casi gratis (hallazgo verificado 2026-07-14).**
Supuse que el PDF no era golden-eable por timestamps; **es falso**: la salida de libharu es
**byte-determinista y NO depende del path/nombre** (dos renders separados y a rutas
distintas son `cmp`-idénticos; no hay `CreationDate` ni `/ID` en el trailer). O sea:
`FORMATS="eps svg pdf"` + `normalize pdf` = identidad (`cat`) — ni siquiera necesita el
parche de `%%Title` que sí requiere el EPS. Costo: 16 renders más por corrida y 16 goldens
locales (no van a git, se regeneran con `capture`).
*Caza:* **regresiones** en PDF. *NO caza:* un bug **preexistente** se bendice como
"correcto" — el de etiquetas en blanco habría quedado congelado en el golden sin alarma.

**Capa 2 — `gs -dNOPAUSE -dBATCH -sDEVICE=nullpage` sobre cada EPS. La de mejor razón
valor/línea.** Un golden por bytes **no puede** cazar los bugs de prólogo: el
`/undefined in ellipse` producía un EPS **byte-estable** que reventaba en un intérprete
PostScript real. Solo un intérprete lo detecta. Falla si el exit ≠ 0.
*Requisito:* `gs` puede no estar → **saltar con aviso si falta** (que el harness siga
usable sin él), nunca fallar por ausencia de la herramienta.
*Caza:* `/ellipse`, `cshow`/`rshow` y todo `undefined` de prólogo (Lección 2).

**Capa 3 — Paridad de TEXTO entre backends.** La clase "texto silenciosamente omitido" es
invisible para las capas 1 y 2 (el golden la bendice; el EPS estaba sano). Lo que sí la
caza: extraer el texto del PDF (`pdftotext`, disponible) y del SVG (grep de `<text>`) y
comparar los **conjuntos**. "Texto presente en SVG pero ausente en PDF" **es exactamente**
el bug de 2026-07-14. Compara **contenido, no píxeles** → inmune a diferencias de
antialiasing/fuentes entre gs y poppler.
*Caza:* los dos bugs de "texto en blanco" (EPS `FN_NOFACE`, PDF `current_font`), que ya
picaron dos veces. *Costo:* moderado (un extractor por formato).

**Explícitamente FUERA:** comparar píxeles PDF vs EPS rasterizados. Frágil (AA y fuentes
difieren entre gs y poppler) y de mantenimiento alto; diferido salvo que algo lo exija.

**Orden sugerido:** Capa 2 primero (máximo valor, ~10 líneas), luego Capa 1 (casi gratis),
Capa 3 cuando toque tocar texto de nuevo. Las tres son independientes de las Fases 2–5:
pueden hacerse en cualquier momento y **conviene tenerlas antes de la Fase 4** (`plot`
mueve texto y ejes en los tres backends a la vez).

## Fricción concreta observada (motiva las features)

`fig6-4v3-clean.mg` quedó limpio en los ejes, pero estas cosas siguen a mano y son la
lista de trabajo:

| Fricción en fig6-4v3-clean | Feature que la elimina | Fase / estado |
|---|---|---|
| Eje y **logarítmico** rotulado a mano (`10^{-15}…10^5` en `text()` math) porque `axis` solo formatea decimales | `axis(scale="log")`: posición logarítmica + rótulos `10^n` con superíndice | 1 ✅ |
| Rótulos `0.30` en vez de `.30` (el `%.Nf` fuerza cero inicial) | `strip_zero=true` (estilo del libro); `format=` genérico después | 1 ✅ / 3 |
| Cada `10^n` manual lleva un **offset x distinto** según el ancho del rótulo | auto-alineación de etiquetas según el lado del eje (right/middle en eje izquierdo) | 1.5 ✅ |
| Línea del eje que no rebasa la 1ª/última marca; ticks al lado equivocado | `extend=` + `ticks="in"` | 1.5 ✅ |
| Etiquetas muy lejos / título corrido | `label_gap`=4 default, `title_gap`, título centrado | 1.5 ✅ |
| `from`/`to` del eje x y el rectángulo del `fit` afinados por separado y **deben** concordar | marco de datos: `axis(edge=...)` hereda rango, sin bloque | 2 (prereq de plot) |
| `step`/`decimals` puestos a dedo | `step` automático (familia 1/2/5·10ᵏ) + `decimals` derivado | 3 |
| Datos como coords **digitalizadas** en píxeles, fiteadas con stretch | datos en unidades reales + auto-mapeo (`plot { }`) | 4 |
| `Po^292`/`U^238` en posiciones adivinadas de la ventana exterior | dentro de `plot`, un `text()` se ancla **en datos** — cae gratis | 4 |
| Títulos matemáticos `λ^-1 (s)`, `E^-1/2 (MeV)^-1/2` puestos a mano con `text()` | `axis title=` ya acepta markup §14 + rotación → dentro de `plot`, `xaxis(title=…)`/`yaxis(title=…)` | 4 |
| Recta de ajuste = polilínea manual | (Fase 5) línea de tendencia | 5 |

## Decisiones de diseño

- **`scale="log"` y el formateo van en el motor (C++).** No pueden ser biblioteca `.mg`:
  `axis` es un generador builtin y el mapeo valor→posición y el formateo de etiquetas
  viven en `AxisStmt::exec`. (Contraste con pseudo-3D, que sí fue biblioteca pura porque
  todo era composición de transforms existentes.)
- **`plot { }` es un constructo tipo `compound`, y su mecanismo es el de `fit`, no una
  ventana anidada.** Bloque de sentencias (datos, líneas, marcadores, texto, ejes) que
  se ejecutan a una lista temporal; el mapeo datos→caja se aplica a los **puntos** de los
  items emitidos. Razón de fondo: la escala log **no es afín** — ninguna `Matrix` 3×3
  puede expresarla — así que el mapeo no puede vivir en el pipeline de matrices del
  motor; tiene que aplicarse a coordenadas. El caso 100% lineal puede optimizarse con
  una matriz envolvente (como `fit` de struct), pero el modelo mental es uno: *plot
  transforma las coordenadas de su contenido*.
- **`plot` vive en el PARSER; el motor no gana elementos gráficos** (auditoría 2026-07-14,
  detalle en "Huella en el motor", Fase 4). Emite solo items existentes, igual que `axis`,
  y remapea puntos con la API pública de `GraphicsItemWithPath`. Huella total en el motor:
  **un accesor** (`GraphicsState::getPosition()`). Esto mantiene la regla del proyecto —
  las features van en el compilador, el motor solo crece cuando hay geometría genuinamente
  nueva que dibujar (§"Adding a new primitive" de CLAUDE.md), y aquí no la hay.
- **Marcadores físicos por default en contexto de datos.** `dot` ya cumple; los símbolos
  de `plan_marcadores.md` deben heredar la propiedad física (posición transformada,
  tamaño en pt), no el escalado de mundo de `place`. Bajo escala log esto pasa de
  conveniencia a **necesidad**: un tamaño "de mundo" ni siquiera está bien definido.
- **No duplicar con `polybar`.** `plot` es el *marco* (mapeo de datos + ejes);
  `polybar` (§`plan_polybar.md`), `bezier`, `dot`, `sine` son *contenido* que se dibuja
  dentro. `plot` no debe re-implementar series.
- **Convención de nombres (bloquear):** los parámetros de `axis` siguen `sustantivo_atributo`
  — `label_gap`, `label_align`, `label_valign`, `tick_size`, `title_gap`, `title_font`,
  `title_size`. Cualquier knob nuevo (de axis o plot) se ciñe a esto. La superficie actual
  de `axis` ya es grande: `from to step start decimals scale minor strip_zero ticks
  tick_size labels label_gap label_align label_valign extend field title title_font
  title_size title_gap` (+ `format` pendiente). **Riesgo de sprawl**; lo mitiga la Fase 3
  (defaults inteligentes: el caso común no fija `step`/`decimals`/`from`/`to`). **Pendiente
  de doc:** la tabla de `especificacion_mg.md` §13.5 quedó **desactualizada** — sincronizarla
  con esta superficie real (y marcar `scale`/`minor`/`extend`/`strip_zero`/`label_align` que
  eran "Propuesta para estudiar" como implementados).

## Fases (ordenadas por costo/desbloqueo)

### Fase 1 — PROTOTIPO: `axis(scale="log")` + `strip_zero` (bajo costo, alto desbloqueo)

Es el prototipo natural: toca **solo `AxisStmt::exec`** (`parserv3.cpp:1503`), sin
sintaxis nueva, y desbloquea `fig6-4` casi por completo (elimina los cinco `text()` de
potencias de diez y el `0.30` con cero inicial).

**Diseño detallado del eje log:**

- **Args.** `from`/`to` siguen siendo **valores de datos** (p. ej. `1e-15`, `1e5` — la
  notación científica ya lexa, `lexer.l:45`), no exponentes. Razón: uniformidad con el
  eje lineal y con la herencia de rango de Fases 2/4 (`plot` declara rangos en datos;
  el eje los hereda tal cual). Validación: `from > 0 && to > 0` o error de compilación
  ("eje log requiere from/to positivos").
- **Mapeo.** `posOf(v)`: `t = (log10(v) − log10(from)) / (log10(to) − log10(from))`.
  Ejes decrecientes (`from > to`) funcionan solos, igual que en lineal.
- **Iteración de marcas SIN deriva flotante.** No iterar `v += step`: iterar el
  **exponente entero** `n` desde `ceil(log10(start))` en pasos de `step` décadas
  (entero, default 1) mientras `n ≤ log10(to)+eps`. Marca/etiqueta en `v = 10^n`.
  `step` no entero → error (las etiquetas son `10^n`). El modo `ticks="grid"` usa la
  **misma** iteración por exponentes (su loop actual también hace `v += step`;
  `posOf` ya da la posición correcta).
- **Rótulos.** Markup matemático del proyecto: `"$10{^%d}$"` → `parse_text` (que ya
  resuelve superíndices, §14), con `FN_NOFACE` como hoy. Caso especial `n = 0` → `"1"`
  (estilo del libro, como fig6-4). Esto es exactamente lo que la versión clean hace a
  mano; el generador lo hará solo.
- **`minor=`.** Marcas menores (sin rótulo, largo `tick_size/2`, misma dirección que
  las mayores según `ticks=`) en `2·10ⁿ … 9·10ⁿ`. **Default: apagadas** (el original
  de fig6-4 no las tiene; cero churn); `minor=true` las activa. Reusar el `GI_TICKS`
  existente con otro `physOut`; en modo `ticks="grid"` las menores NO barren el campo
  (siguen siendo marcas cortas sobre la línea).
- **Limitación documentada.** Rango que no contiene ninguna década completa (p. ej.
  `300..900`): sin marcas mayores. El fallback estilo "2, 3, 5·10ᵏ" de las libs de
  plotting queda fuera del v1; advertencia de compilación.
- **`strip_zero=true`** (ejes lineales): post-procesa la etiqueta ya formateada.
  Regla exacta: si empieza con `"0."` o `"-0."`, quitar ese `0` (`"0.30"` → `".30"`,
  `"-0.30"` → `"-.30"`); cualquier otra cosa (`"0"`, `"10.5"`) queda intacta. Booleano,
  sin riesgo de inyección; el `format=` genérico estilo printf se difiere a Fase 3
  (necesita validar el especificador — un `%s` colado a `snprintf` con argumento
  double es UB).
- Default = comportamiento actual en todo → **cero churn en goldens**.

**Entregable:** reescribir el eje y de `fig6-4v3-clean` como

```text
axis(scale="log", from=1e-15, to=1e5, step=5) { 0 1  0 5 }
```

(las marcas caen en `y = 1..5` ↔ `10^-15, 10^-10, 10^-5, 1, 10^5` — cinco décadas por
unidad de plot) y el eje x con `strip_zero=true`. Verificar EPS/SVG/PDF contra
`fig6-4.png`.

**`extend=` — HECHO (2026-07-14).** El "sobrante de línea estilo libro" (el eje que
rebasa sus marcas) que aquí quedaba como candidato ya está implementado en `AxisStmt`:
`extend=n` alarga **solo la línea** del eje más allá de sus extremos, **en unidades del
eje** (las mismas del bloque `{p1 p2}`, no pt físicos — la extensión de fig6-4 es
estructural, ~1 unidad, no un nib), sin mover marcas ni etiquetas. Escalar = ambos
extremos por igual; lista `[lo hi]` = `lo` antes de `p1`, `hi` después de `p2`. Default
`0` → cero churn. En fig6-4 la vertical usa `extend=(1, 0.3)` (baja 1 unidad bajo la 1ª
marca hasta el eje x, formando la esquina; rebasa 0.3 arriba) y el eje x `extend=0.4`.
Combinado con `ticks="in"` (marcas hacia adentro, como el original), fig6-4 iguala el
libro en los tres backends.

### Fase 1.5 — Auto-alineación de etiquetas (mata los offsets a mano) — HECHA (2026-07-14)

**Estado: implementada y verificada** (`AxisStmt::exec`, `parserv3.cpp`). Las etiquetas
de `axis` se alinean solas según el lado; fig2-3 y fig6-4 quedan con etiquetas bien
colocadas (el eje y de fig6-4 ya no se encima con la línea). Golden re-bendecido
(solo fig2-3 cambió, verificado contra el oráculo `examples/v1/reference/fig2-3.svg`:
la nueva alineación **coincide** con el oráculo V1, antes no). Override `label_align=`/
`label_valign=` funcionando. La trampa del prólogo EPS (`using_textalign`) se activó
como estaba previsto abajo. Notas de implementación:
- Etiqueta simple (`.30`) vuelve de `parse_text` como `Text` pelón; etiqueta math
  (`$10{^n}$`) como `TextLine`. Ambas rutas honran align/valign: `Text` por el backend
  (`cshow`/`rshow` en EPS, `text-anchor` en SVG, offset en PDF); `TextLine` por su propio
  desplazamiento de ancho. Cero código extra para cubrir las dos.
- **Bug preexistente destapado y ARREGLADO (2026-07-14):** en PDF las etiquetas de
  **texto simple** de `axis` (el eje x lineal `.30 .35…`) **no se dibujaban**; las math
  (`10^n`) sí. Raíz: `PDFDisplay::text()` (`PDFDisplay.cpp:533`) hacía `if (!current_font)
  return;` sin fallback a fuente default — el análogo exacto del guard `dev_size<0` que se
  arregló para EPS pero nunca para PDF. Un `axis` cuya primera etiqueta es texto simple y
  sin `font` previo salía en blanco solo en PDF. **Fix:** antes del `return`, si
  `!current_font` se fija la cara vigente (o Times-Roman). fig6-4 **y fig2-3** ganan sus
  etiquetas de eje x en PDF; cero efecto en documentos que ya fijan fuente (el guard solo
  dispara con `current_font` null). El golden (EPS/SVG) no lo cubre; verificado por vista.
- **`label_gap` afinado + `title_gap` nuevo (2026-07-14):** con la auto-alineación, el
  `label_gap` pasó a medir del eje al **borde cercano** de la etiqueta (antes, con anclaje
  por baseline, el texto se extendía hacia el eje y "comía" el hueco), así que el default
  de 8 pt quedó **muy lejos**. Bajado a **4 pt** (libra las marcas de 3 pt de `ticks="out"`
  con ~1 pt de margen). Efecto colateral: el título estaba en `label_gap·3`, que al encoger
  se encimaba con las etiquetas → **desacoplado** en `title_gap` (pt del eje al título,
  default `label_gap + 20` = reproduce la posición previa; override propio). Solo fig2-3
  (golden) se movió; re-bendecido tras verificar. fig6-4 iguala al libro en los 3 backends.
- **Título CENTRADO a lo largo del eje (2026-07-14):** el título se anclaba al punto medio
  pero se dibujaba alineado a la izquierda → salía corrido a la derecha (eje X) / arriba
  (eje Y). Ahora emite `align=center` acotado: en X centra horizontalmente, en Y (texto
  rotado) centra a lo largo de la línea porque `cshow`/`text-anchor`/offset operan sobre la
  base **rotada**. Verificado en EPS/SVG/PDF contra el render V1 de fig2-3 (`fig2-3v1.eps`):
  ambos títulos coinciden con el original. Reusa la trampa `using_textalign` de las
  etiquetas. Solo fig2-3 (golden) se movió; re-bendecido.

Hoy `AxisStmt` emite las etiquetas sin tocar `align`/`valign` → alineación ambiente
(izquierda/baseline). Por eso los `10^n` manuales de fig6-4 llevan un offset x
**distinto por etiqueta** según su ancho. Fix: `AxisStmt` fija alineación **acotada**
alrededor del loop de etiquetas, derivada del lado:

- eje ~horizontal (etiquetas abajo): `align="center"`, `valign="top"`;
- eje ~vertical (etiquetas a la izquierda): `align="right"`, `valign="middle"`.

**Mecanismo concreto** (el mismo patrón que el título ya usa para `title_size`):
envolver el loop de etiquetas en `GS_PUSHSTATE`/`GS_POPSTATE` y emitir dos `Attribute`
— `AT_TALIGN` (0=left, 1=center, 2=right) y `AT_TVALIGN` (0=baseline, 1=top, 2=middle,
3=bottom), los códigos de las sentencias `align`/`valign` en `parserv3.cpp:380-395`.

> ⚠️ **Trampa conocida (misma clase que el bug `/ellipse`):** el prólogo EPS
> `/cshow /rshow` solo se emite si `g_flags.using_textalign` está en true
> (`EPSDisplay.cpp:171`); la sentencia `align` lo activa en parse-time
> (`parserv3.cpp:383`). Si `AxisStmt` emite `AT_TALIGN≠0` sin activar la bandera, el
> EPS llama al operador `cshow` real de PostScript y truena en typecheck. Fix: poner
> `g_flags.using_textalign = true` en `AxisStmt::exec` cuando haya etiquetas — es
> seguro porque exec corre al construir el árbol, ANTES de que `EPSDisplay::start()`
> lea las banderas para escribir el prólogo.

Override con `label_align=`/`label_valign=` si el caso raro lo pide. **Riesgo de churn:**
`fig2-3.mg` (corpus golden) usa `axis`; sus etiquetas se moverán. Verificar visualmente
contra el original y re-bendecir con `capture`. Va en fase propia para no mezclar el
churn con el cero-churn de la Fase 1.

### Criterios de aceptación (Fases 1 y 1.5)

1. `make` limpio de warnings (`-Wall -Wpedantic -Wsuggest-override`).
2. **Fase 1:** `bash test/run.sh check` → `ok=32 fail=0 error=0` SIN re-bendecir
   (cero churn por diseño). **Fase 1.5:** se espera fallo solo en fig2-3; comparar
   visualmente contra el original y re-bendecir con `capture` únicamente ese cambio.
3. `./bin/mg examples/fig6-4v3-clean.mg` a `.eps`, `.svg` y `.pdf`: los **tres** backends
   consistentes entre sí y contra `fig6-4.png` (el bug de etiquetas de PDF ya se arregló,
   Fase 1.5). El EPS pasa `gs -dNOPAUSE -dBATCH -sDEVICE=nullpage fig6-4v3-clean.eps` sin
   error — esto caza los bugs de prólogo (`/ellipse`, `cshow`) — y el PDF se revisa por
   vista (el golden no lo cubre, Lección 3).
4. Casos de error con mensaje claro (no crash, no silencio): `scale="log"` con
   `from<=0`, `step` no entero en log, rango log sin década completa (advertencia).

### Fase 2 — Marco de datos: `edge=` + herencia de rango  ← PREREQUISITO DE PLOT, siguiente

`axis(edge="bottom"|"top"|"left"|"right")` **sin** bloque `{p1 p2}` y **sin** `from`/`to`:
toma los extremos físicos del borde y el rango de la `world_window` vigente (§13.7). Con
esto `fig2-3` pierde sus dos bloques de extremos y sus `from`/`to`. Requiere que
`AxisStmt` consulte `mg.getWindow()` cuando faltan bloque/rango.

**Es el camino crítico hacia `plot`:** dentro de `plot`, `xaxis`/`yaxis` son exactamente
`axis(edge=…)` con rango heredado. Sin `edge=`, `plot` tendría que pasar los cuatro
extremos y el rango a cada eje a mano. Por eso Fase 2 va antes que Fase 4. Bajo costo
(consultar `mg.getWindow()`), alto desbloqueo. **Sub-tarea nueva** que la Fase 1.5 dejó
lista: `edge` también fija el LADO de las etiquetas/marcas/título (hoy se deriva de la
tangente `horiz`; con `edge="top"` o `"right"` el "out" es el opuesto, y la auto-alineación
de la Fase 1.5 debe invertirse — center/**bottom** para top, **left**/middle para right).

*Nota log:* `edge` + `scale="log"` solo compone bien **dentro de `plot`** (Fase 4), donde
los rangos heredados son de datos reales. En un documento suelto la `world_window` es
lineal — un eje log standalone sigue necesitando `from`/`to` explícitos (como el
entregable de Fase 1).

### Fase 3 — Defaults inteligentes y `format=`

- `step` automático (localizador 1/2/5·10ᵏ, ~5–8 marcas) y `decimals` derivado del paso,
  cuando se omiten (§13.5 "Optativo"). Quita los dos parámetros más tediosos del caso común.
- `format=` genérico estilo printf para lo que `decimals`/`strip_zero` no cubran
  (notación científica, enteros con separador…). **Validado**: exactamente un
  especificador de conversión, solo `%f/%g/%e/%d`; cualquier otra cosa → error de
  compilación.

### Fase 4 — `plot { }` como constructo tipo compound

**Por qué `plot` vale ahora MUCHO más que cuando se esbozó.** El esbozo original asumía un
`axis` básico. Tras las Fases 1–1.5, `axis` es maduro: log, `minor`, `strip_zero`,
auto-alineación, `extend`, ticks in/out, `label_gap`/`title_gap` calibrados, **títulos
matemáticos centrados y rotados**. `plot` **hereda todo eso gratis** —su único trabajo es
el *mapeo datos→caja* del contenido y delegar los bordes a `xaxis`/`yaxis`. La salida de
`plot` sale con calidad de libro sin lógica de dibujo nueva. Además, como §16 (ventanas
anidadas) **no existe**, `plot` no es solo azúcar: es el **único** mecanismo limpio de
"marco de datos" para paneles (fig4-10) sin hackear la ventana global con stretch.

**Prerequisito: Fase 2 (`edge=`).** `xaxis`/`yaxis` dentro de `plot` = `axis(edge=…)` con
rango heredado. Hacer `plot` antes de Fase 2 obligaría a re-derivar los bordes a mano.

**Sintaxis:** UN solo bloque = contenido (sentencias, como `compound`); rangos de datos
como args; caja física como `box=` (tupla, como `at=` de `repeat`), default = ventana
vigente:

```text
plot(x=(0.30,0.50), y=(1e-15,1e5), yscale="log", box=(0.6,0.7, 5.79,5.4)) {
    polyline { 0.30 3e4  0.50 2e-15 }         % recta de ajuste, EN DATOS
    fill "black"
    dot(1.5) { 0.31 8e2  0.33 1.1e1  ... }    % puntos EN DATOS (físicos)
    text("$Po{^292}$") { 0.315 2e3 }          % anotación anclada a DATOS
    xaxis(step=0.05, strip_zero=true, title="$/iE^{/r-1/2} (MeV)^{/r-1/2}$")
    yaxis(minor=true, title="${/gl^-1} (/rs)$")
}
```

Un solo bloque, sin ambigüedad coords-vs-sentencias; `box=` omitido cubre "el plot es toda
la figura" (fig6-4), `box=` explícito cubre paneles (fig4-10). **Este es exactamente
fig6-4 sin una sola coordenada digitalizada ni un `text()` de potencia de diez o de
título** — el objetivo final del plan.

**El mecanismo, en UNA frase: `plot` transforma las coordenadas de su contenido, y se
bifurca según la escala.** Es la lección central del proyecto (log NO es afín):

- **Plot 100% lineal → matriz envolvente.** `FitStmt::fitMatrix(stretch=true)` sobre la
  caja, aplicada con `Transform` push/pop alrededor del contenido. **Ruta general y de cero
  mecanismo nuevo:** hasta structs invocadas dentro funcionan (igual que `fit` de struct).
- **Plot con algún eje log → mapper puntual** `(x,y) → (X(x), Y(y))`, con cada eje afín o
  `afín ∘ log10`, aplicado recorriendo la lista temporal de items (ver "Huella en el motor").
  Bézier: puntos de control mapeados puntualmente (aproximación, como al digitalizar).

#### Huella en el motor: CERO elementos gráficos nuevos, UN accesor (auditado 2026-07-14)

**Corrección al plan anterior:** este documento decía que la ruta log "necesita un hook
`GraphicsItem::mapPoints(fn)`". **Está sobre-diseñado** — mete un virtual a TODA la
jerarquía (N overrides) para lograr lo que un recorrido parser-side consigue reusando API
que ya existe. Auditoría del código:

- **`plot` es un `Stmt` puro, como `axis`/`compound`/`fit`.** Precedente decisivo: `axis`
  —con log, ticks, alineación y títulos rotados— **no añadió ni una primitiva al motor**;
  emite solo `Polyline`/`Text`/`GraphicsState`/`Attribute`/`Transform`. `plot` igual.
- **Los items con puntos ya exponen su path.** `Polyline`, `Dot`, `Polybar`, `Rectangle` y
  `Arc` heredan de **`GraphicsItemWithPath`, que tiene `getPath()`/`setPath()` PÚBLICOS**
  (`primitives.h:228-235`). El mapeo es `getPath()` → mapear → `setPath()`: **cero motor**.
- **Discriminación sin RTTI:** `switch (getType())` + `static_cast`, el idioma que el motor
  ya usa (`Polyline::draw` despacha así por `type`). Vive en el parser.
- **El ÚNICO cambio de motor:** `GraphicsState::position` es privado y **no tiene getter**
  (`primitives.h:408-414`), y ahí viven las anclas de `text()`. Hace falta añadir
  `point getPosition() const`. **Una línea, aditiva, sin tocar la jerarquía.**
- **Precedente de mapeo parser-side:** `FitStmt` (rama path) ya hace exactamente esto con
  `process_path(M, path)` (función libre de `splines.h`) antes de emitir el `Polyline`.
  Para log haría falta su gemela no-afín (un `process_path` que tome un mapper), **también
  como función libre del parser**.

**Invariante físico (Lección 4): cae GRATIS por construcción, no por disciplina.** `Dot`
guarda sus centros en el `path` pero el radio `r` es un miembro privado **aparte**
(`primitives.h:305-330`): mapear el path mueve los centros y **no hay forma** de que toque
el radio. Igual `line_width`/`font_size`, que son `Attribute`s, no puntos. El mapper
literalmente no puede alcanzar una cantidad física. Por eso el contenido de datos usa
`dot`, no `circle` (que además se deforma bajo stretch).

> ⚠️ **Gotcha que refuerza excluir los ejes del mapper:** en un `Polyline(GI_TICKS)`,
> `path[0]` **NO es un punto sino el vector de la marca** (`Polyline::draw` lo lee como `tk`
> y lo suma a los demás). Un `ticks()`/`axis` que cayera en el contenido de datos quedaría
> corrompido por el mapper. Los ejes ya están excluidos por diseño (se dibujan en coords
> exteriores); esto lo vuelve obligatorio, no una preferencia.

**Restricción de structs bajo log (error claro, no silencio) — la razón exacta:** una struct
invocada emite un `Transform` (matriz) + su contenido; el mapper del parser remapearía los
puntos **locales** y luego el motor aplicaría la matriz en **draw-time** → daría
`matriz(map(p))` en vez de `map(matriz(p))`, que **difieren cuando `map` no es afín**. Por
eso bajo log el contenido son primitivas directas (`polyline`/`bezier`/`dot`/`text`/
`polybar`). En un plot **lineal** no hay problema: la ruta es la matriz envolvente y las
structs funcionan igual que en `fit`.

**Parser** (calcado de `CompoundStmt`, `parserv3.cpp:1791`): `parsePlot` = args nombrados +
loop de `parseStatement` hasta `}`. Dentro se interceptan `xaxis`/`yaxis` (válidos **solo**
aquí) → `AxisStmt` con `edge` predefinido.

**Ejecución (`PlotStmt::exec`):**
1. **Separar** el cuerpo: sentencias de datos vs. `xaxis`/`yaxis`.
2. **Contenido de datos** → ejecutar a una `GraphicsItemList` temporal y aplicarle el mapeo:
   matriz envolvente si es lineal; si algún eje es log, **recorrer la lista** y remapear los
   puntos vía `getType()` + `getPath()`/`setPath()` (y `getPosition()` para las anclas de
   texto) — ver "Huella en el motor".
3. **Ejes** → NO pasan por el mapper: `xaxis`/`yaxis` corren en coords exteriores con
   `p1 p2` = borde de `box` y `from/to/scale` de los args de `plot`. `axis` ya sabe log,
   alinear, centrar título, `extend` → **reuso directo, cero lógica de dibujo nueva.**
4. **Orden de pintado = orden de escritura:** `grid` primero (fondo), ejes al final
   (encima). `plot` no reordena.

**Opción a considerar: `frame=true`** — dibujar los cuatro lados de `box` (marco cerrado
estilo libro), no solo los dos ejes. fig6-4 usa ejes en L (2 lados); muchas figuras de MQ
tienen marco completo. Sería `xaxis`/`yaxis` en los cuatro bordes (o un rect); bajo costo
una vez que `edge=` existe.

**Qué NO hace `plot`:** clipping a la caja (no hay clip en el motor; se documenta — un
`clip=true` esperaría a que el motor lo soporte), leyendas, series con estilo automático.
Todo eso es contenido explícito, estilo MG.

**Criterios de aceptación (Fase 4):** portar fig6-4 y fig2-3 a `plot { }` (uno log, uno
lineal-anisótropo) y que igualen sus versiones actuales en los **tres** backends (golden
EPS+SVG + spot-check PDF + `gs nullpage`), con **cero** coordenada digitalizada de eje y
cero `text()` de potencia/título. Caso de error: struct con transform dentro de un `plot`
log → mensaje claro.

### Fase 5 — (opcional, cuando un ejemplo lo pida) contenido de alto nivel

- **Línea de tendencia** sobre una serie (la recta manual de fig6-4).
- Integración con **`polybar`** (histogramas dentro de `plot`).
- La *anotación anclada a datos* ya NO está aquí: cae gratis con `plot` (Fase 4).
Diferidas: sin ejemplo del corpus que las exija hoy.

## Decisiones propuestas (antes "preguntas abiertas" — veto de Alejandro bienvenido)

1. **Forma del bloque de `plot`:** un solo bloque de contenido + rangos como args +
   `box=` nombrado con default "ventana vigente". Elimina el doble bloque del borrador
   §13.7 (frágil y sin precedente en el parser) y reusa el patrón `at=(x,y)` de `repeat`.
2. **Estilo de rótulo del libro:** `strip_zero=true` booleano YA (Fase 1, cubre `.30`);
   `format=` printf genérico DESPUÉS (Fase 3, con validación estricta del especificador).
3. **`minor` en log:** apagadas por default (fidelidad a fig6-4 y cero churn);
   `minor=true` para los semilog que las quieran.
4. **Corpus golden:** promover `fig6-4v3-clean.mg` al corpus (EPS+SVG) **YA** (tras la
   Fase 1.5, no solo la 1) — ejercita eje log + `fit(stretch)` + texto matemático con
   superíndices + `extend` + ticks-in + título/label spacing, combinación que ningún golden
   cubre y donde se escondieron TODOS los bugs de esta línea. **Caveat:** el golden es
   EPS+SVG; el bug de PDF vivió justo en ese hueco, así que su promoción NO sustituye el
   spot-check de PDF (Lección 3). Los crudos `fig6-4.mg`/`fig6-4v3.mg`/`fig6-4.png` siguen
   sin commitear.

## Estado y orden recomendado (act. 2026-07-14)

- **Fase 1 ✅** (log + strip_zero), **Fase 1.5 ✅** (auto-alineación), y adelantados por
  fidelidad a la figura: `extend` ✅, ticks-in ✅, `label_gap`/`title_gap` ✅, título
  centrado ✅, fix de PDF `current_font` ✅. Todo verificado en 3 backends contra oráculo.
- **Siguiente: Fase 2 (`edge=`)** — camino crítico a `plot`, bajo costo. Incluye invertir
  la auto-alineación de la 1.5 para `edge="top"/"right"`.
- **Luego Fase 4 (`plot`)** — el headline; su valor subió mucho porque hereda todo el
  pulido de `axis`. Portar fig6-4 y fig2-3 a `plot{}` es el criterio de cierre.
  **Costo de motor auditado: UN accesor** (`GraphicsState::getPosition()`), cero elementos
  gráficos nuevos → es trabajo de parser, no de motor.
- **Fase 3 (auto-step/decimals + `format=`)** — ortogonal, alto valor (se afinó `step` a
  mano en cada ejemplo); puede intercalarse antes o después de la 4.
- **Tarea de test (independiente, ver sección "TAREA — Automatizar las compuertas de la
  Lección 3"):** `gs nullpage` + PDF al golden + paridad de texto en `test/run.sh`.
  Conviene **antes de la Fase 4**, que mueve texto y ejes en los tres backends a la vez.
  Barata: el PDF resultó byte-determinista, así que la Capa 1 es casi gratis.
- **Tarea de doc pendiente:** sincronizar la tabla de `especificacion_mg.md` §13.5 con la
  superficie real de `axis` (ver "Convención de nombres" en Decisiones de diseño).

## Resumen

El graficado de datos ya tiene el núcleo (`axis` §13.5 + marco de datos §13.7 + `fit`) y,
tras las Fases 1–1.5, un `axis` de calidad de libro (log, alineación, `extend`, spacing,
títulos matemáticos centrados) verificado en los tres backends. Quedan dos frentes: **el
`edge=` de la Fase 2** (camino crítico) y **empaquetar en `plot { }`** (Fase 4), un
constructo tipo `compound` cuyo mecanismo es *transformar las coordenadas de su contenido*
—matriz envolvente si lineal, mapper puntual si log (porque log no es afín y §16 no
existe)— delegando los bordes a `xaxis`/`yaxis` = `axis(edge=…)`, que **hereda gratis**
todo el pulido. `plot` es **trabajo de parser**: su huella auditada en el motor es UN
accesor, cero elementos gráficos nuevos (igual que `axis`, que no añadió ninguno). El norte es fig6-4 y fig2-3 escritos como `plot { }` sin una sola
coordenada digitalizada. La lección durable que enmarca todo: **cada fase se cierra
portando una figura real del libro contra su oráculo, en EPS/SVG/PDF** — ahí es donde
aparecen los requisitos y los bugs que la spec abstracta no anticipa.
