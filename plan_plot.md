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

- **`axis` declarativo (§13.5), funcionando.** `AxisStmt` en `parserv3.cpp:1512`: línea
  + marcas (`out`/`in`/`both`/`none`/`grid`) + etiquetas numéricas + título (con
  rotación §19 en ejes verticales). `from`/`to`/`step`/`start`/`decimals`/`tick_size`/
  `label_gap`/`title`/`title_font`/`title_size`. La posición de cada valor sale del
  lambda `posOf(v)` (interpolación lineal `p1..p2`), y los offsets físicos de `physOut`.
- **Marco de datos (§13.7), núcleo resuelto.** `world_window` en unidades de datos +
  `stretch=true`; marcadores **físicos** `dot`/`marker` (§4.6) inmunes al stretch;
  `fit(Struct, stretch=true)` como la misma idea sobre struct reutilizable. Realizado en
  `fig2-3.mg`.
- **`grid` (§13.6)** y **`numbers`/`ticks` (§13.1-2)** como generadores de más bajo nivel.

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

## Fricción concreta observada (motiva las features)

`fig6-4v3-clean.mg` quedó limpio en los ejes, pero seis cosas siguen a mano y son la
lista de trabajo:

| Fricción en fig6-4v3-clean | Feature que la elimina | §spec |
|---|---|---|
| Eje y **logarítmico** rotulado a mano (`10^{-15}…10^5` en `text()` math) porque `axis` solo formatea decimales | `axis(scale="log")`: posición logarítmica + rótulos `10^n` con superíndice | §13.5 (propuesto) |
| `from`/`to` del eje x y el rectángulo del `fit` afinados por separado y **deben** concordar | marco de datos: `axis(edge=...)` hereda rango de la ventana, sin bloque | §13.7 |
| Rótulos `0.30` en vez de `.30` (el `%.Nf` fuerza cero inicial) | `format=` / `strip_zero` | §13.5 (propuesto) |
| `step`/`decimals` puestos a dedo | `step` automático (familia 1/2/5·10ᵏ) + `decimals` derivado | §13.5 (propuesto) |
| Datos como coords **digitalizadas** en pixeles, fiteadas con stretch | datos en unidades reales + auto-mapeo (marco de datos / `plot`) | §13.7 |
| Recta de ajuste = polilínea manual; `Po^292`/`U^238` en posiciones adivinadas | (Fase 5) línea de tendencia; anotación anclada a dato | — |

## Decisiones de diseño

- **`scale="log"` y `format=` van en el motor (C++).** No pueden ser biblioteca `.mg`:
  `axis` es un generador builtin y el mapeo valor→posición y el formateo de etiquetas
  viven en `AxisStmt::exec`. (Contraste con pseudo-3D, que sí fue biblioteca pura porque
  todo era composición de transforms existentes.)
- **`plot { }` es azúcar, no un mecanismo nuevo.** Desazucara a: ventana anidada (§16)
  con `stretch` + contenido en datos + `axis(edge=…)` por borde. La spec (§13.7) deja
  **abierta** la forma del bloque (¿doble bloque rango+caja, o caja de la ventana
  vigente?). Se decide con datos de uso (Fase 4), no antes.
- **Marcadores físicos por default en contexto de datos.** `dot` ya cumple; los símbolos
  de `plan_marcadores.md` deben heredar la propiedad física (posición transformada,
  tamaño en pt), no el escalado de mundo de `place`.
- **No duplicar con `polybar`.** `plot` es el *marco* (ventana de datos + ejes);
  `polybar` (§`plan_polybar.md`), `bezier`, `dot`, `sine` son *contenido* que se dibuja
  dentro. `plot` no debe re-implementar series.

## Fases (ordenadas por costo/desbloqueo)

### Fase 1 — PROTOTIPO: `axis(scale="log")` + `format=` (bajo costo, alto desbloqueo)

Es el prototipo natural: toca **solo `AxisStmt::exec`** (`parserv3.cpp:1512`), sin
sintaxis nueva, y desbloquea `fig6-4` por completo (elimina los cinco `text()` de
potencias de diez).

Cambios acotados en `AxisStmt::exec`:
- **Mapeo log.** Hoy `posOf(v)` interpola lineal en `[from,to]`. Con `scale="log"`:
  `t = (log10(v) - log10(from)) / (log10(to) - log10(from))`. `from`/`to` son valores
  reales (p. ej. `1e-15`/`1e5`); las marcas mayores caen en las décadas.
- **Marcas mayores/menores.** Mayores en cada potencia de 10 (o cada `step` décadas);
  menores (`minor`, sin rótulo) en 2·10ᵏ…9·10ᵏ. Reusar el `GI_TICKS` existente.
- **Rótulos `10^n`.** En vez de `snprintf("%.Nf")`, generar la cadena de markup
  matemático `"$10{^%d}$"` y pasarla por `parse_text` (que ya hace superíndices, §14).
  Esto es lo que `fig6-4v3-clean` hace a mano; el generador lo hará solo.
- **`format=`** (ejes lineales): cadena estilo `printf` o `strip_zero` para el `.30` del
  libro; default = comportamiento `decimals` actual (sin cambio → cero churn en goldens).

Entregable: reescribir el eje y de `fig6-4v3-clean` como
`axis(from=1e-15, to=1e5, scale="log", minor=9) { 0 0  0 5.5 }` y verificar EPS/SVG/PDF
contra `fig6.4.png`. Añadir un golden nuevo (o promover fig6-4 al corpus) tras validar.

### Fase 2 — Marco de datos completo: `edge=` + herencia de rango

`axis(edge="bottom"|"top"|"left"|"right")` **sin** bloque `{p1 p2}` y **sin** `from`/`to`:
toma los extremos físicos del borde y el rango de la `world_window` vigente (§13.7). Con
esto `fig2-3` pierde sus dos bloques de extremos y sus `from`/`to`. Requiere que
`AxisStmt` consulte `mg.getWindow()` cuando faltan bloque/rango. (El eje log de Fase 1
compone: `edge="left", scale="log"`.)

### Fase 3 — Defaults inteligentes

`step` automático (localizador 1/2/5·10ᵏ, ~5–8 marcas) y `decimals` derivado del paso,
cuando se omiten (§13.5 "Optativo"). Quita los dos parámetros más tediosos del caso común.

### Fase 4 — Azúcar `plot { }`

Constructo parser (como `axis`/`compound`: `parsePlot` + `PlotStmt`) que empaqueta
ventana de datos + `stretch` default + ejes. Decidir la forma del bloque (§13.7 lo deja
abierto); recomendación de la spec: probar primero el marco de datos con ventana anidada
(cero costo, reusa §16) y adoptar `plot` al ver la repetición real en fig2-3/fig4-10.
Borrador de sintaxis a validar:

```text
plot(x=(0.30,0.50), y=(1e-15,1e5), yscale="log") {   % rangos de datos
    dot(1.5) { ... }                 % contenido EN DATOS (marcadores físicos)
    xaxis(step=0.05, format=".2f")   % ejes por borde, rango heredado
    yaxis(minor=9, title="$\lambda^{-1}$ (s)")
}
```

`xaxis`/`yaxis` = alias de `axis(edge="bottom"/"left")` válidos solo dentro de `plot`.

### Fase 5 — (opcional, cuando un ejemplo lo pida) contenido de alto nivel

- **Línea de tendencia** sobre una serie (la recta manual de fig6-4).
- **Anotación anclada a un punto de datos** (`Po^292`/`U^238`).
- Integración con **`polybar`** (histogramas dentro de `plot`).
Diferidas: sin ejemplo del corpus que las exija hoy.

## Preguntas abiertas (para Alejandro)

1. **Forma del bloque de `plot`** (Fase 4): ¿rangos como args + contenido en un bloque
   (borrador de arriba), o `plot` deriva la caja de la ventana vigente y solo toma
   contenido? La spec §13.7 lo deja explícitamente a decidir con uso real.
2. **Estilo de rótulo del libro:** ¿`format=".2f"` genérico estilo printf, o un flag
   `strip_zero`/estilo nombrado para el `.30` sin cero inicial? (Los libros de MQ usan
   `.30`, `10^5`.)
3. **`minor` en log:** ¿marcas menores en las 8 subdivisiones por default, o solo con
   `minor=` explícito? (El original de fig6-4 no las tiene; muchos semilog sí.)
4. **¿Promover `fig6-4` al corpus golden** tras la Fase 1, o mantenerlo como banco de
   pruebas del plan hasta que `plot` esté?

## Resumen

El graficado de datos ya tiene el núcleo (`axis` §13.5 + marco de datos §13.7 + `fit`).
El trabajo es **pulir el eje** (log, formato, defaults, `edge=`) y **empaquetar** el
patrón en azúcar `plot { }`. El **prototipo** (Fase 1) es acotado y de alto valor:
`scale="log"` + `format=` en `AxisStmt::exec`, que cierra `fig6-4` sin sintaxis nueva.
Las dos correcciones de EPS de esta sesión (ellipse, fuente FN_NOFACE) ya dejaron el
camino de "plot en EPS" libre.
