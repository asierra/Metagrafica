# `at`, ventana local y anchor — notas de diseño

Notas para estudiar con calma la relación entre el modificador `at`, el truco de
`world_window` local y la idea de un parámetro `anchor`. Contexto: fig2-6 (el
detector se centraba en V1 vía `world_window`, y en V3 no, hasta el arreglo del
2026-07-10).

---

## 1. El punto de partida: el truco de `world_window`

La definición del detector (fig2-6):

```
struct Detector() {
    world_window 0.5 1.2 0.5 1.2      % rango 0.7 en x y en y
    line_width 1.0
    polyline { 1 0.3  1 0  0 0  0 1  1 1  1 0.7 }   % coords 0..1
}
```

La poligonal vive en `[0,1]²`, pero la ventana local es `[0.5,1.2]²`. Al colocar
la struct, el motor normaliza la ventana a la caja unitaria: `box = (local − 0.5) / 0.7`.

| local | box |
|------:|----:|
| 0.5   | 0.000 |
| 1.0   | +0.714 |
| 0.0   | −0.714 |

Efecto: la forma que ocuparía `[0,1]` pasa a ocupar `[−0.714, +0.714]`. Queda
**centrada en el origen de colocación** y **agrandada ~1.43×** (rango 1.428 vs 1.0).
Eso es un *anchor = centro* obtenido con la ventana, sin ninguna variable nueva.

**Clave:** ese recentrado es **invariante a la escala** porque ocurre *dentro* del
marco de colocación (la normalización `N` se aplica antes del `scale=`). Por eso se
comporta como un anchor de verdad: cambias `scale=` y sigue centrado.

---

## 2. El bug que había en V3 (arreglado)

La normalización ventana→unidad existía en tres caminos de colocación pero **no** en
la invocación directa:

| Camino de colocación | ¿Normalizaba `world_window`? |
|----------------------|:---:|
| `fit(S) { … }`       | Sí (`structBox` → matriz en `FitStmt`) |
| `place(S) { … }`     | Sí (`buildStructure` hornea `N`) |
| `repeat(S, …)`       | **No** (usa coords crudas; ver §5) |
| `S(at=…, scale=…)` (directa) | **No** ← el bug |

Por eso `fit(Pila, …)` respetaba su ventana pero `Detector(at=…, scale=…, rotate=…)`
no: dibujaba la poligonal en 0..1 sin centrar, y `scale=0.2` la encogía desde la
esquina en vez de desde el centro.

**Arreglo** (`InvokeStmt::exec`, parserv3.cpp): hornear la misma `N` que usa
`buildStructure`, *por dentro* del marco de colocación, de modo que
`device = … · frame · N · coord`:

```cpp
Box w = structBox(caller, def);
bool needN = (w.x != 0.0 || w.y != 0.0 || w.dx != 1.0 || w.dy != 1.0);
// … PUSHSTATE
if (hasFrame) OPMPUSH(frame);      // T·R·S de at/rotate/scale
if (needN)    OPMPUSH(N);          // N = scale(1/dx,1/dy)·translate(-wx,-wy)
//   ejecutar cuerpo …
if (needN)    OPMPOP;
if (hasFrame) OPMPOP;
// … POPSTATE
```

El orden importa: `N` va **después** del `frame` en la pila, para que las coords del
cuerpo se normalicen primero y luego el `at/rotate/scale` posicione la caja ya
centrada. Con esto la invocación directa queda consistente con `fit`/`place`.

Alcance del cambio en el corpus actual: **solo** afecta structs invocados de forma
directa **y** con `world_window` local ≠ unidad. Hoy el único caso es `Detector`
(fig2-6). El resto de structs con `world_window` (Marco, Flechas, FigAB/CD,
SenDeriv2…) se colocan con `fit`/`place`, y las demás invocaciones directas (Fg1,
SenDeriv2Sym, Cuadro) usan caja unitaria → intactas. Verificado con render + gs.

---

## 3. La pregunta de diseño: ¿hace falta un `anchor`?

Un `anchor` diría *qué punto del struct* aterriza en el punto de colocación
(esquina, centro, un punto nombrado). El truco de `world_window` ya es eso
(anchor = centro), gratis. Entonces:

- **`anchor` es redundante con la ventana local** para el caso de centrar.
- Tu propuesta: que la **ruta/locus diga *dónde*** (como `place`, orientado por la
  tangente) y que **`at` sea un *corrimiento*** que lleva el punto de referencia al
  centro. En ese esquema `at` **es** el anchor, expresado como offset en vez de como
  punto nombrado. Conclusión: **no necesitas una variable `anchor` aparte.**

### La condición (importante): ¿en qué marco vive `at`?

Depende de en qué marco se aplique el `at`, y el código actual **no** hace lo que esa
idea necesita:

- Hoy `InvokeStmt` construye `frame = T·R·S` (post-multiplicación: la traslación de
  `at` es la **más externa**, en coordenadas de mundo). O sea, `at` desplaza
  *después* de escalar.
- Si usas ese `at` para centrar, el corrimiento **no es invariante a escala**:
  cambias `scale=` y hay que recalcular el `at` (el "medio tamaño" cambió). Ese
  recálculo manual es justo el impuesto que un `anchor` nombrado evita.
- En cambio, el truco de `world_window` **sí** es invariante a escala (la `N` se
  aplica antes del `S`).

Para que `at` sustituya limpiamente a `anchor` habría que decidir que `at` es un
**offset local** (aplicado antes de la escala, orden `S·R·T` en vez de `T·R·S`), no
una traslación de mundo.

### El dilema

```
                 hoy                          alternativa "at local"
        frame = T · R · S                     frame = S · R · T
        (at en coords de MUNDO)               (at en coords LOCALES del struct)

  at posiciona el struct en el lienzo   at es un corrimiento que escala con el struct
  centrar = recalcular at por escala    centrar = at fijo, invariante a escala
  = posición absoluta (lo intuitivo)    = anchor implícito (lo que pides)
```

Son dos rutas coherentes; **mezclarlas es lo que confunde**. Opciones:

1. **`at` = posición de mundo (hoy)** + dejar el centrado al `world_window`.
   No cambia nada; ya funciona. `anchor` queda cubierto por la ventana.
2. **`at` = offset local** (cambiar a `S·R·T`). `at` pasa a ser el anchor
   invariante a escala; pero cambia la semántica actual de posición absoluta y
   habría que revisar todos los `at` del corpus (§4).
3. **Separar dos modificadores**: p.ej. `at=` posición de mundo y `anchor=`/`shift=`
   offset local. Más explícito, pero es "la variable extra" que querías evitar.

Recomendación tibia: mantener (1) mientras el `world_window` cubra el centrado;
considerar (2) solo si aparece un patrón donde reescalar+recentrar a mano se vuelve
molesto de forma recurrente.

---

## 4. Cómo se usa `at` hoy en los ejemplos que ya funcionan

`at` está **sobrecargado** entre constructos —no siempre significa lo mismo—:

### a) Invocación directa `S(at=…)` — traslación del marco (mundo), `T·R·S`

```
fig2-1:  Fg1(at=(0.53, 0.5), scale=0.46)            % Fg1 = caja unitaria
fig2-6:  Detector(at=(7, -0.2), scale=0.2, rotate=37)  % Detector = ventana 0.5..1.2
rpstest: Cuadro(scale=(0.2, 0.125), at=(0.1, 0.75)) % Cuadro = caja unitaria
fig4-1:  struct Fg1() { rotate 90  SenDeriv2Sym(at=(1, 0)) }   % Sym = caja unitaria
```

`at` = dónde cae el origin del struct en coords del padre. En `Detector` es el único
donde además hay `world_window`, así que es el único donde el centrado (§1) entra en
juego. En los demás, la caja es unitaria y `at` es simplemente la esquina.

Nota `SenDeriv2Sym(at=(1, 0))`: aquí `at` desplaza en x por 1 *después* de
`rotate 90` del cuerpo del padre — un ejemplo de por qué el marco de `at` (mundo vs
local) no es cosmético.

### b) `repeat(S, at=…, advance=…)` — origen de la rejilla (mundo), `T·R·S·A^k`

```
rpstest: repeat(Cuadro, count=3, scale=0.3, at=(0.05, 0.05), advance=(0.3, 0))
fig4-1:  repeat(Arrowr, scale=0.02, count=9, at=(0.24, 0.1), advance=(0, 0.1))
```

`at` = posición de la instancia `k=0`; cada paso suma `advance`. Misma familia
`T·R·S` que la invocación directa, con `A^k` (transform acumulado) por dentro.

### c) Generadores `ticks`/`numbers(at=…, advance=…)` — posición de la pluma (mundo)

```
fill_styles: numbers(from=0.1, by=0.1, count=10, at=(0.025, 0.25), advance=(0.1, 0))
fig6-10:     ticks(23, mark=(2, 0), at=(2, 1.95), advance=(0, 0.2))
fig2-6:      % ticks(10, mark=(0, 0.1), at=(0.76, 0.8), advance=(0.24, 0))  (comentado)
```

Aquí `at` = punto de arranque de la pluma en coords de mundo; `advance` mueve entre
elementos. No hay struct ni caja; es puramente la posición inicial.

### Resumen de la sobrecarga de `at`

| Constructo | Qué significa `at` | Marco |
|------------|--------------------|-------|
| `S(at=…)` directa | origin del struct en el padre | mundo (`T·R·S`) |
| `repeat(…, at=…)` | posición de la instancia `k=0` | mundo (`T·R·S·A^k`) |
| `ticks`/`numbers(at=…)` | posición inicial de la pluma | mundo |

En los tres, `at` es **coordenada de mundo**. La discusión de §3 (¿`at` local?) solo
tocaría la fila 1 (y quizá la 2). Cambiarlo obligaría a revisar estos usos.

---

## 5. Pendiente relacionado: `repeat` tampoco normaliza `world_window`

`RepeatStmt::exec` usa las coords crudas del cuerpo (`T·R·S·A^k`) sin pasar por
`structBox`, igual que hacía la invocación directa antes del arreglo. Hoy no muerde
porque los structs que se repiten (Cuadro, Arrowr) tienen caja unitaria. Si algún día
se repite un struct con `world_window` local ≠ unidad, saldría el mismo síntoma que
tenía el detector. Arreglo análogo al de §2 si llega a hacer falta.
