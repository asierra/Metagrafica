# Plan: traductor `mg1to2.py` (V1 → V3)

Plan detallado para implementar el traductor de fuentes MetaGráfica V1 (comandos
de dos letras) a la sintaxis V3. Pensado para que un modelo menor —o Alejandro—
lo implemente sin re-derivar el contexto. El cutover (§22.6) ya está hecho:
`bin/mg` es el compilador V3; este traductor es la última pieza para migrar el
material V1 histórico.

---

## 0. Idea en una frase

`mg1to2.py entrada.mg` lee un `.mg` **V1** (comandos de dos letras, ver
`examples/v1/*.mg`) y escribe un `.mg` **V3** equivalente (gramática nueva, ver
`especificacion_mg.md` y `examples/*.mg`). "Equivalente" = **su render debe
coincidir con el del original** (mismo dibujo).

---

## 1. El recurso más importante: ya existen 16 pares traducidos a mano

**No empieces de cero.** Para cada `examples/v1/X.mg` existe una traducción a
mano `examples/X.mg`, hecha durante el desarrollo del parser V3. Son la
**especificación por ejemplo** del traductor: el objetivo es que `mg1to2.py`
produzca algo **equivalente** (idealmente idéntico módulo espaciado/orden) a esas
traducciones a mano.

- `examples/v1/*.mg` — entradas (V1).
- `examples/*.mg` — salidas de referencia (V3, hechas a mano). Es el corpus de la
  red golden (`test/run.sh`, ok=32).
- `examples/v1/reference/*.svg` — **oráculo de render** (lo que produce el
  compilador V1). Es la verdad de fondo para verificar.

Los 16: `arrow curvas3 fig2-1 fig2-3 fig2-6 fig4-1 fig4-10 fig6-1 fig6-10
fill_styles line_patterns markers-demo primitives rpstest sines texto`.

**Actualización 2026-07-11:** `fig6-10` y `fig4-10` se re-portaron **fielmente**
al original V1 (puntos de ocupación, huecos de letrero, encuadre medido contra el
oráculo) — dejaron de ser "por calibrar" y son ahora **buenos objetivos de
fidelidad** para el traductor.

Empieza por los que ya son **píxel-idénticos** al oráculo cuando se compilan con
`bin/mg` (V3): **`fill_styles` y `line_patterns`**. Si tu traductor reproduce
esos dos, tienes el núcleo bien.

---

## 2. Verificación (el loop de trabajo)

Para cada ejemplo:

```bash
python3 mg1to2.py examples/v1/X.mg > /tmp/X_v3.mg      # traducir
./bin/mg /tmp/X_v3.mg /tmp/X.svg                        # render con el compilador V3
# comparar contra el oráculo V1, ignorando encabezado/dimensiones:
diff <(grep -vE '^<\?xml|width=|height=|viewBox' /tmp/X.svg) \
     <(grep -vE '^<\?xml|width=|height=|viewBox' examples/v1/reference/X.svg)
```

- **`fill_styles`, `line_patterns`** deben dar **diff = 0** (píxel-idénticos).
- El resto: comparar también contra la traducción a mano
  `diff /tmp/X_v3.mg examples/X.mg` para ver si el traductor coincide con el
  criterio humano. Varios ejemplos son "por calibrar / declarativo limpio" y su
  render no es píxel-exacto por diseño (fig2-3, fig4-1, fig4-10, …); ahí basta
  con equivalencia estructural.

**Regla de oro:** un traductor mecánico correcto reproduce el render del oráculo
para los ejemplos geométricos simples; los que la traducción a mano rediseñó
(paneles, marcos de datos) no son objetivo de fidelidad píxel.

---

## 3. Sintaxis V1 de entrada (lo que hay que parsear)

V1 es **orientado a líneas / tokens**, no estructurado por bloques. Reglas:

- **Comentarios**: líneas o restos que empiezan con `%`.
- **Directivas**: `$D w h` (dimensiones cm), `$P n` (tamaño de fuente base),
  `$S ...` (spline mode), `$O ...` (trigger de marcadores). Empiezan con `$`.
- **Comandos**: token en MAYÚSCULAS (`PL`, `BR`, `CR`, `LWIDTH`, `TSTYLE`, …)
  seguido de sus argumentos. Cada comando tiene **aridad conocida** (fija) o un
  **bloque de coordenadas** terminado en `}`.
- **Bloques de coordenadas**: muchos primitivos toman una lista de números
  (pares x y) hasta un `}` de cierre: `PL 0 0  5 3  10 0 }`. Algunos usan `:`
  para separar params de la lista: `CR 0.1 : ...puntos... }` (radio, luego
  centros). El `;` separa subtrayectos.
- **Cadenas de texto**: `XYDT x y El texto...` — el resto de la línea es la
  cadena (con markup TeX/griego que se PRESERVA tal cual: V3 usa el mismo
  `parse_text`).
- **Structs**: `OPST nombre ... CLST` delimitan una definición; `MKST nombre ...`
  la invoca.

> El lexer V1 (`src/mgpp.l`) y el parser (`src/Parser.cpp`) son la referencia
> autoritativa de la aridad y el formato exacto de cada comando. Cuando dudes de
> cómo se parsea un comando, míralo ahí. `src/lexmg.cpp` (generado) tiene el
> `keyword_map` con todos los tokens.

---

## 4. Arquitectura sugerida del traductor

Un traductor **stateful de una pasada** basta para el grueso:

```
tokens = tokenize(v1_source)        # respeta %comentarios, $directivas, }, :, strings
state  = TranslatorState()          # pila de fuente/gris/etc. para cerrar bloques
out    = []                         # líneas V3

for cada comando c con sus args:
    emitir la(s) línea(s) V3 según la tabla §5, actualizando `state`
```

**Estado que hay que llevar** (para traducir bien):
- Dentro/fuera de una `struct` (OPST/CLST) → indentar y abrir/cerrar `struct N() { }`.
- Transformaciones locales abiertas (TLLC/RTLC/…): en V1 se cierran con `IDLC`;
  en V3 con el fin del bloque `{ }`. El traductor abre un `{ }` al primer
  transform y lo cierra en `IDLC` (o al fin de struct).
- Ventana desproporcionada (ver §6): decidir `stretch=true` o renumerar.

**Alternativa más robusta** (recomendada si hay tiempo): reusar el **lexer V1**
en vez de re-tokenizar en Python. Pero como V1 está congelado y el objetivo es un
script independiente, tokenizar en Python es aceptable —los comandos son simples.

---

## 5. Tabla de mapeo comando → V3

Fuente: tablas de conversión de `especificacion_mg.md` (§21 y apéndices). Las
CONVERSIONES NUMÉRICAS (marcadas ⚠) son las que hay que respetar al pie.

### Documento / control
| V1 | V3 |
|---|---|
| `$D w h` | `display_size w h` |
| `$P n` | `font_size n` (tamaño base; en V3 es estado, ver THEIGHT) |
| `WW xmin xmax ymin ymax` | `world_window xmin xmax ymin ymax` (⚠ ver §6 stretch) |
| `INPUT "f"` | `include "f"` |
| `EXIT` | `exit` |
| `MAXDEEP n` | `max_depth n` |
| `INTXT`/`XYTXT`/`MKMR`/`MR` | — (obsoletos, se eliminan) |

### Primitivas
| V1 | V3 |
|---|---|
| `PL … }` | `polyline { … }` |
| `PG … }` | `polygon { … }` |
| `BR x1 y1 x2 y2 }` | `rectangle { x1 y1  x2 y2 }` |
| `CR r : centros }` | `circle(r) { centros }` (o `dot(r)` si es marcador) |
| `EL rx ry : … }` | `ellipse(rx, ry) { … }` |
| `DOT s pts… }` / `DOT s &path` | `dot(r) { pts… }`  ⚠ **s = tamaño físico (1er arg)**; V1 `DOT 3` ≈ V3 `dot(0.75)` (fig6-10, factor ~0.25) — calibrar contra el oráculo. `DOT s &dots` (dispersar sobre un path generado) → un `for` con `dot()` por punto (ver §6.4) |
| `BZ p0 c1 c2 p1 … }` | `bezier { … }` |
| `SP … }` | `spline { … }` |
| `DT texto` | `text("texto")` (en la pluma) |
| `XYDT x y texto` | `text("texto") { x y }` |

### Estilo (⚠ conversiones numéricas)
| V1 | V3 |
|---|---|
| `LWIDTH w` | `line_width (w×0.2)`  ⚠ **LWIDTH n → n·0.2 pt**; `LWIDTH 0 → 0.1` (algunos oráculos muestran 0.5 — VERIFICAR por contexto) |
| `LPATRN n` | `dash "…"`: 0/1→`"solid"`, 2→`"dashed"`, 3→`"dotted"`, 4→`"dashdot"`, 5→`"dashdotdot"` |
| `LCOLOR c` | `color "c"` |
| `FCOLOR c` | `fill "c"`  (c negativo o `-c` → además contornear: en V3 se añade `color=`) |
| `FILL` / `NOFILL` | activar/desactivar relleno (`fill=` presente/ausente) |
| `LGRAY g` / `FGRAY g` | `color gray(g/100)` / `fill gray(g/100)`  ⚠ **VERIFICAR el signo**: en `fill_styles`, `FGRAY 10` → `gray(0.1)` (oscuro); pero `LGRAY 50` del grid de fig2-3 salió NEGRO. Ajustar contra el oráculo. |
| `FPATRN n` | `hatch=ang, hatch_gap=gap`  ⚠ ver §6 (tabla índice→ángulo/gap); `n` negativo → contornear (`color=`) |
| `TSTYLE nombre` | `font "nombre"` (roman/italic/bold/…) |
| `THEIGHT h` / `TSIZE h` | `font_size h` |
| `TALIGN n` | `align "left"`(0)/`"center"`(1)/`"right"`(2) |

### Transformaciones locales
| V1 | V3 |
|---|---|
| `TLLC dx dy` | `translate dx dy` |
| `RTLC a` | `rotate a`  ✅ **§19 implementado (2026-07-11): el texto bajo `rotate` gira los glifos** en los 3 backends, así que `XYPP x y  RTLC 90  DT etiqueta` → `{ translate x y  rotate 90  text("etiqueta") { 0 0 } }` rinde la etiqueta vertical (p.ej. "energy" de fig6-10) |
| `SCLC sx sy` | `scale sx sy` |
| `SHLC hx hy` | `shear hx hy` |
| `IDLC` | (cierra el bloque `{ }` que agrupa los transforms) |

### Structs y placements
| V1 | V3 |
|---|---|
| `OPST n … CLST` | `struct n() { … }` |
| `MKST n args` | `n(args)` (invocación) |
| `SCST sx sy` | escala MTST del **siguiente** placement (no dibuja). En V3: `scale=` en la invocación, **o** doblarlo en el rect del `fit`/en el aspecto de la ventana. ⚠ Si `sx≠sy` era compensación anisótropa del V1 (`SCST .45 .8` en fig6-10) → en V3 **ensanchar `world_window`** para que su aspecto = el del display (ver §6.1), NO un `scale` anisótropo |
| `LNST sc [sh [cnt [gap]]] p1 p2` | `place(n, scale=sc, shift=sh, count=cnt, gap=gap) { p1 p2 }`  ⚠ **shift = 1 − arg2** (el motor V1 guarda `1-valor`); **gap** = hueco centrado para un letrero → `place(gap=)` **verificado** (fig6-10 `LNST … .5` → `gap=0.5`); `sc<0` → `both_sides=true` (flechas de doble punta) |
| `ARCST sc r da ai [sh [cnt]] c` | `place(n, scale=sc, r=r, from=ai, to=ai+da) { c }`  ⚠ **to = ai + da** (da es barrido) |
| `DPST n` / `&name` | `place(n, scale) { &path }` |
| `PWST p1 p2` | `fit(n, stretch=true) { p1 p2 }`  ⚠ **PWST SIEMPRE deformaba** → `stretch=true` |
| `RPST …` | `repeat(n, …)` |

### Pluma, generadores, álgebra de paths
| V1 | V3 |
|---|---|
| `XYPP x y` | `moveto(x, y)`  (o al=/advance= en la llamada del generador) |
| `TLPP dx dy` | `step(translate=(dx,dy))`  (o `advance=` en la llamada) |
| `TICKS n dx dy` | `ticks(n, mark=(dx,dy), at=…, advance=…)`  ⚠ V1 normalizaba dx/wdx, dy/wdy — en V3 NO; ver §6 |
| `GNNUM i0 inc n dec` | `numbers(from=i0, by=inc, count=n, decimals=dec, at=…, advance=…)` |
| `GNPATH n x y name` + `DOT s &name` | ⚠ **NO es `trail()`** (que no existe en el parser). `GNPATH` genera un path de `n` puntos desde `(x,y)` con el paso de pluma vigente (`TLPP dx dy`); `DOT s &name` dispersa dots físicos ahí = **columna/serie de puntos**. En V3 → un `for i = 0 to n-1 { y = y0 + i*paso  dot(r) { x y } }` (ver §6.4). **Trampa:** una traducción a mano lo confundió con `polyline(dash="dotted")` — NO repetir |
| `OPPT … CLPT` | `compound { … }` |
| `RPPT`/`CTPT…CLPT`/`INVPT`/`NORMPT`/`GNBZPATH` | `tile()`/`concat()`/`reverse()`/`normalize()`/`smooth()` (álgebra §9 — aún no toda implementada en el parser) |

---

## 6. Los tres nudos difíciles (leer con cuidado)

### 6.1 Coordenadas desproporcionadas / `docwmin` / `stretch`
El motor V1 **normalizaba cada eje por separado** al cuadrado unitario y luego
estiraba con `scale(dvx,dvy)`; además aplicaba un factor `docwmin = min(wdx,wdy)`
a las escalas de los placements. El motor V3 **no** hace nada de eso (es
isométrico por construcción, §3.1). Consecuencias para el traductor:

- Si la ventana V1 (`WW`) es **desproporcionada** respecto a `$D` (aspect ratio
  distinto), el V1 la deformaba. Para reproducirlo en V3 hay dos caminos (§21 de
  la spec): (a) emitir `world_window … stretch=true`, o (b) **renumerar** las
  coordenadas a una ventana proporcional. La traducción a mano de `fig2-3` usó
  (a) con la ventana en unidades de datos; `fig2-1` quedó como "archivo
  desproporcionado" pendiente de calibrar. **Empieza sin `stretch`; si el diff
  contra el oráculo muestra deformación por eje, añade `stretch=true`.**
- **Técnica medida (fig6-10, 2026-07-11) para el caso `$D` ancho + `SCST` anisótropo:**
  el síntoma es una figura **apretada en un eje** con márgenes vacíos en el otro —
  porque el meet isométrico se limita por el eje más restrictivo. La causa exacta es
  **aspecto de `world_window` ≠ aspecto de `$D`**. Solución idiomática V3: **darle a la
  ventana el aspecto del display** (extender el eje corto: en fig6-10 la ventana pasó de
  11.8×9.4 a 11.8×**23.6** en X, con los paneles colocados a lo ancho). Esto REEMPLAZA el
  estirado anisótropo `SCST .45 .8` del V1 sin deformar nada. Diagnóstico útil: rasterizar
  oráculo y salida a PNG y medir posiciones (perfil de columnas oscuras) — dio pozos 88px
  vs 44px = factor 2 exacto → confirmó el letterbox.
- Las escalas de placement (`LNST sc`, `MKST scale`) llevaban el factor
  `docwmin` en V1. Si un placement sale con tamaño incorrecto, el factor a
  aplicar es `sc × min(wdx,wdy)` o su inverso — calibrar contra el oráculo (esto
  ya se vio en `fig2-1`: `Fg1(scale=0.46)` "real" ≈ 0.386 = 0.46×0.84).

### 6.2 `FPATRN` → `hatch`/`hatch_gap`
V1 `FPATRN idx` (idx 1..10) es una tabla de 10 patrones = 4 ángulos × 3
densidades. El motor V3 la conserva (`Display::patternFor`). El mapeo idx →
(ángulo, gap):
```
ángulo = ((idx-1) % 4) · 45        → 0, 45, 90, 135
densidad d = (idx-1) / 4           (división ENTERA)  → 0,1,2
gap    = 4 / (1 + d)               (entera)          → 4, 2, 1
```
Entonces `FPATRN 8` → ángulo 135, gap 2 → `hatch=135, hatch_gap=2`. `idx`
negativo (`FPATRN -8`) → además contornear → añadir `color=` en V3.
(El parser V3 hace el camino inverso en `hatchIndex()`; aquí es directo.)

### 6.3 Estado que "se filtra" y hay que resetear
V1 reseteaba estilos explícitamente (`LPATRN 0`, `LWIDTH 1`, …). En V3 el estado
también persiste, así que el traductor debe **conservar esos resets** — no
omitirlos por parecer redundantes. Ejemplo real: `fig2-3` ponía `LPATRN 2`
(curva) y luego `LPATRN 0`; sin el `dash "solid"` de vuelta, la trama contaminaba
grid y ejes. **Traduce cada comando de estado, incluidos los que "vuelven al
default".** (Alternativa: envolver en bloques `{ }` donde V1 usaba estado
acotado, pero es más invasivo; el reset explícito es más fiel.)

> **Nota (bug #1 arreglado 2026-07-11):** el parser V3 ya acepta **varias sentencias
> por línea** (aridad acotada en sentencias de estado, como `translate`/`rotate`), así
> que el traductor puede emitir `line_width 0.4  color "red"  n()` en un renglón si
> conviene. Ya NO es obligatorio "una sentencia por línea" (antes, una sentencia de
> estado se tragaba la siguiente del mismo renglón).

### 6.4 `GNPATH … dots` + `DOT s &dots` → columnas de puntos con `for`
Patrón V1 frecuente (fig6-10: ocupación electrónica): `TLPP 0 dy` fija el paso de
pluma, `GNPATH n x0 y0 dots` genera un path de `n` puntos, y `DOT s &dots` dispersa
un dot físico en cada uno. En V3 no hay `trail`; se emite un bucle:
```
for i = 0 to n-1 {
    y = y0 + i*dy        % ⚠ precomputar en variable (ver abajo)
    dot(r) { x0 y }
}
```
Un `TLPT dx 0` que traslada el path y repite `DOT s &dots` → otro bucle desplazado
(o `dot(r) { x0 y  x1 y }` sembrando ambos). **Trampa histórica:** la traducción a
mano confundió esto con `polyline(dash="dotted")` (líneas punteadas) — el traductor
NO debe hacerlo; son **puntos**.

> **⚠ Trampa de coords calculadas:** los bloques de coordenadas `{ }` se parsean con
> `parseTerm` (solo `* /`, potencia `^`), **sin `+`/`-` binario**. Una coord como
> `y0 + i*dy` NO se puede escribir inline en el bloque → hay que **precomputar en una
> variable** (`y = y0 + i*dy`, que sí usa la gramática de expresión completa) y usar
> `dot(r) { x0 y }`. Aplica a cualquier coordenada aritmética generada por el traductor.

---

## 7. Orden de implementación (incremental, verificable)

1. **Tokenizador V1** + esqueleto (comentarios, `$D`/`$P`/`WW`, un primitivo
   `PL`). Verifica que traduce un `PL` suelto.
2. **Primitivas geométricas** (`PL PG BR CR EL DOT BZ`) + **estilo básico**
   (`LWIDTH LCOLOR FCOLOR LGRAY FGRAY FILL/NOFILL`). Con esto apunta a
   **`primitives.mg`** (diff pequeño) y a la fila de colores/grises de
   **`fill_styles`**.
3. **`FPATRN` → hatch** (§6.2), **`THEIGHT/TSTYLE/TALIGN`**, **`GNNUM`**,
   **`XYDT/DT`**. Objetivo: **`fill_styles` y `line_patterns` diff = 0**. Este es
   el hito que valida el núcleo.
4. **Structs** (`OPST/CLST/MKST`) + **placements** (`LNST/ARCST/PWST/RPST`) +
   **transforms** (`TLLC/…/IDLC`). Objetivo: `arrow`, `rpstest`.
5. **Generadores/ejes** (`TICKS`), **`LPATRN`→dash**, **`OPPT/CLPT`→compound**,
   **pluma** (`XYPP/TLPP`). Objetivo: fig2-6, fig6-10.
6. **Casos desproporcionados** (§6.1: stretch/renumerar) y **álgebra de paths**
   (`RPPT/CTPT/…`, si el parser V3 ya la soporta). Objetivo: fig2-1, y los
   multi-panel (fig4-1/fig4-10, que igual no son píxel-exactos por diseño).

En cada paso, corre el loop de verificación (§2) sobre los ejemplos que ese paso
debería cubrir.

---

## 8. Qué NO tiene que resolver el traductor

- Los ejemplos "declarativo limpio / por calibrar" (fig2-3, fig4-1, fig6-1) **no**
  son objetivo de fidelidad píxel: sus traducciones a mano rediseñaron el encuadre
  (marcos de datos, paneles). El traductor mecánico puede producir una versión más
  literal (con `stretch`, sin el rediseño); está bien. **Nota:** `fig4-10` y
  `fig6-10` ya NO están en esta categoría — se re-portaron fielmente (§1) y sí sirven
  como objetivo de fidelidad.
- Features del parser V3 aún no implementadas: **álgebra de paths completa §9**
  (spline/tile/concat/reverse — el motor `splines.cpp` existe pero no todo está
  cableado) y **ventanas anidadas §16**. Si un V1 las usa, emitir la sintaxis V3 y
  marcar `% TODO` — pero solo aparecen en material V1 avanzado, no en los 16 base.
  **Ya HECHAS (no marcar TODO):** `valign` (§7.3), `dash`, marcadores/flechas §4.11,
  hatch como estado, rotación de texto §19, `outlinefill`, concatenación de cadenas.

---

## 9. Entregable

`mg1to2.py` en la raíz del repo. CLI mínimo:
```
python3 mg1to2.py entrada.mg [salida.mg]     # stdout si no se da salida
```
Sin dependencias externas (solo stdlib). Un test `test/run_translator.sh` que
corra el loop §2 sobre los 16 ejemplos y reporte diff por archivo sería el
complemento natural (empezando por exigir diff=0 en fill_styles/line_patterns).
