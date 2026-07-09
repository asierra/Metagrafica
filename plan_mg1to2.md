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
`especificacion_mg.md` y `examples/v3/*.mg`). "Equivalente" = **su render debe
coincidir con el del original** (mismo dibujo).

---

## 1. El recurso más importante: ya existen 14 pares traducidos a mano

**No empieces de cero.** Para cada `examples/v1/X.mg` existe una traducción a
mano `examples/v3/X.mg`, hecha durante el desarrollo del parser V3. Son la
**especificación por ejemplo** del traductor: el objetivo es que `mg1to2.py`
produzca algo **equivalente** (idealmente idéntico módulo espaciado/orden) a esas
traducciones a mano.

- `examples/v1/*.mg` — entradas (V1).
- `examples/v3/*.mg` — salidas de referencia (V3, hechas a mano).
- `examples/v1/reference/*.svg` — **oráculo de render** (lo que produce el
  compilador V1). Es la verdad de fondo para verificar.

Los 14: `arrow curvas3 fig2-1 fig2-3 fig2-6 fig4-1 fig4-10 fig6-1 fig6-10
fill_styles line_patterns primitives rpstest sines`.

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
  `diff /tmp/X_v3.mg examples/v3/X.mg` para ver si el traductor coincide con el
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
| `DOT … }` | `dot(r) { … }` |
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
| `RTLC a` | `rotate a` |
| `SCLC sx sy` | `scale sx sy` |
| `SHLC hx hy` | `shear hx hy` |
| `IDLC` | (cierra el bloque `{ }` que agrupa los transforms) |

### Structs y placements
| V1 | V3 |
|---|---|
| `OPST n … CLST` | `struct n() { … }` |
| `MKST n args` | `n(args)` (invocación) |
| `LNST sc [sh [cnt [gap]]] p1 p2` | `place(n, scale=sc, shift=sh, count=cnt\|gap=gap) { p1 p2 }` |
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
| `GNPATH n x y name` | `trail(start=(x,y), count=n, …)` |
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

- Los ejemplos "declarativo limpio / por calibrar" (fig2-3, fig4-1, fig4-10,
  fig6-1) **no** son objetivo de fidelidad píxel: sus traducciones a mano
  rediseñaron el encuadre (marcos de datos, paneles). El traductor mecánico puede
  producir una versión más literal (con `stretch`, sin el rediseño); está bien.
- Features del parser V3 aún no implementadas (álgebra de paths completa §9,
  `valign`, ventanas anidadas §16): si un V1 las usa, el traductor puede emitir
  la sintaxis V3 correspondiente aunque el parser todavía no la compile —o
  marcarla con un comentario `% TODO` — pero eso solo aparece en material V1
  avanzado, no en los 14 ejemplos base.

---

## 9. Entregable

`mg1to2.py` en la raíz del repo. CLI mínimo:
```
python3 mg1to2.py entrada.mg [salida.mg]     # stdout si no se da salida
```
Sin dependencias externas (solo stdlib). Un test `test/run_translator.sh` que
corra el loop §2 sobre los 14 ejemplos y reporte diff por archivo sería el
complemento natural (empezando por exigir diff=0 en fill_styles/line_patterns).
