# Plan: operación `transpose` para paths/datos (§9)

Propuesta de una operación **`transpose`** que intercambia los ejes (x↔y) de una serie de
datos, para que un usuario pueda cambiar la orientación de una curva de forma **declarativa**
en vez de reescribir cada par de coordenadas a mano.

## Motivación

En fig4-1 las curvas de difracción son datos `(t, intensidad)` con `t` horizontal, pero en la
figura se dibujan con la **intensidad horizontal** y `t` **vertical** (el perfil se extiende
en la dirección del haz). La V1 lo resolvía con un `rotate 90` + `at` anidado con los `fit`
—frágil, se descuadraba—. Al reescribirla, **transpusimos los datos a mano** (cada `t I` →
`I t`), que salió limpio. Una operación `transpose` haría ese paso declarativo:

```text
% datos naturales (t, intensidad), tal como salen de la fórmula/medición
path curva = { 0 9.87  0.05 9.79  0.10 9.55  … }
% ...dibujar con los ejes intercambiados, sin tocar los números:
draw transpose(&curva)
```

Caso de uso general: cualquiera con datos `(x, y)` que quiera plotearlos con los ejes
cambiados (barras horizontales vs verticales, perfiles laterales, etc.).

## Decisión de diseño clave: es una operación de DATOS, no un transform de marco

`transpose` = intercambiar x↔y de cada punto = **reflexión en la diagonal `y=x`**
(matriz `[[0,1],[1,0]]`, determinante −1, invierte orientación).

Se podría pensar en implementarlo como una **sentencia de transformación §11.1** (junto a
`rotate`/`scale`/`shear`), que sería barato (§11.1 ya existe). **PERO NO sirve para el caso
de uso**, porque un transform de marco transpone TODO el sistema de coordenadas — incluido el
rectángulo de `fit`. Es decir, `{ transpose  fit(Curva){0.35 0 0.9 1} }` colocaría la curva
en el rectángulo *transpuesto* `(0, 0.35)–(1, 0.9)`, no en el que se quiere. Es exactamente el
enredo "content + placement se transponen juntos" que hacía frágil el truco `rotate 90` del V1.

Por eso `transpose` debe operar sobre **los datos** (el path): intercambia los puntos y su
bounding box; luego `fit`/`draw` lo colocan **normalmente**, con el rectángulo en coordenadas
naturales. Es la versión declarativa de lo que hicimos a mano en fig4-1.

*(Corolario útil para documentar: `transpose` equivale a `scale(1,-1)` seguido de `rotate(90)`
— una reflexión, no una rotación pura. Empaquetarlo evita que el usuario acierte el orden y
los signos, que es literalmente lo que se equivocaba en V1.)*

## Sintaxis (encaja en el álgebra de paths §9)

Operación **unaria** sobre un path, como `reverse`/`flip_x`/`flip_y` (§9). Acepta **cualquier
path** —un literal `{ … }` o una referencia `&nombre`—, pero **solo un path** (no bloques ni
structs):

```text
PathExpr ::= … | "transpose" "(" PathExpr [ "," NamedArg* ] ")"

path perfil = transpose(&datos)                 % nombrado
polyline(transpose({ 0 0  1 2  2 3 }))          % literal, de una vez
```

Semántica: `transpose(p)` produce un path nuevo con `(x,y) → (y,x)` en cada punto (todos los
subtrayectos). No muta el original (funcional, como toda el álgebra §9).

### Operaciones de path decididas (conjunto que ejercita fig4-1)

| op | forma | qué hace |
|----|-------|----------|
| declarar | `path ID = PathExpr` | objeto path nombrado |
| referir | `&ID` | referencia a un path nombrado |
| `transpose` | `transpose(&p)` | intercambia ejes `(x,y)→(y,x)` (reflexión en `y=x`) |
| `flip_x` | `flip_x(&p)` | refleja en el eje x (`(x,y)→(x,−y)`) — el **espejo** real |
| `flip_y` | `flip_y(&p)` | refleja en el eje y (`(x,y)→(−x,y)`) |
| `concat` | `concat(&a, &b)` | suelda dos paths en uno continuo (ver abajo) |

Decisiones (2026-07-10, 2ª ronda):
- **Reflexión = `flip_x` / `flip_y` como ops sueltas** (no un parámetro `flip="x"|"y"` de
  `transpose`): dos nombres ahorran el parámetro y leen mejor. `flip_x` es el **espejo** que en
  la vieja `curvas3` hacía la esquina invertida del `fit` (`{0 0.5 1 0}`); pone el pico central
  al centro del perfil simétrico.
- **`normalize` NO urge y queda pospuesto.** `fit` encuadra el **bbox real** del path exactamente
  en el rectángulo pedido, así que mientras se coloque con `fit` (caso de fig4-1) `normalize` no
  aporta. Sigue documentado como parámetro opcional para el caso `draw`/`polyline` **sin** `fit`,
  pero fuera del alcance de este ejemplo.
- **`scale` de datos** (factores arbitrarios, no solo ±1) — **futuro**, no necesario ahora. Queda
  anotado aquí/en la spec; `flip_x`/`flip_y` cubren el caso −1 que sí hace falta.
- **`concat` como función** (`concat(&a,&b)`) por ahora. El **álgebra simbólica de paths** (p. ej.
  un operador `~`; `+` ya es concatenación de **cadenas** y `&` es referencia) queda como idea a
  futuro, **no urge**.

### `concat` reutiliza `concat_paths` de `src/splines.cpp` (ya implementado)

`concat_paths(Path &path1, Path path2, Matrix mt, bool use_translation)` (`splines.cpp:173`) ya
hace **más** que un append de puntos, y a nuestro favor:
1. **Reverse implícito** para orientar el segmento entrante (el usuario no lo hace a mano) —
   pero la decisión está cableada a **x**: `if (p2f.x > p2b.x) reverse`.
2. **Auto-traslación** que **suelda** el frente del segmento con el final del acumulado (el
   usuario tampoco posiciona la 2ª mitad; se pega continua).

Ojo: el reverse implícito invierte **orden de puntos**, NO refleja — la forma espejada la sigue
dando `flip_x`; `concat` solo suelda continuo. Dos verificaciones al implementar:
- La heurística de reverse decide **por x**, así que hay que concatenar en el espacio **natural
  `(t,I)`** (t=x monótona) y **transponer después** en el `fit` de fig4-1 (orden ya acordado).
  Concatenar *tras* transponer rompería la heurística (x=intensidad, no monótona).
- **Mejora opcional (no bloqueante):** generalizar la decisión de "orden en x" a "**extremo más
  cercano al final acumulado** (por distancia)" para que `concat` sea independiente de la
  orientación. Cambio local en `concat_paths`.

Plan: reusar `concat_paths` tal cual para fig4-1 (comodidad máxima) y **verificar contra el
oráculo** (`examples/v1/reference/fig4-1.svg`) que el perfil simétrico sale sin kink ni pico
doble; generalizar la heurística solo si aparece un caso no-x-monótono.

## Ejemplos

### E1 — fig4-1 con `transpose` (curvas = puro dato)

**`curvas3.mg` = puro dato** (sin structs, sin `world_window`, sin `fit`): las medias curvas
verbatim `(t, I)` y las simétricas **derivadas** con álgebra de paths. La reflexión + concat es
derivación de datos (no colocación), así que vive aquí; el `transpose` y el `fit` viven en
`fig4-1.mg` (placement). curvas3 deja de saber de orientación:

```text
% media curva medida, (t, I) verbatim del V1
path SenDeriv2 = { 0 9.87  0.05 9.7887  0.10 9.5492  …  2.00 0 }

% perfil simétrico: media curva + su espejo, soldadas en un solo path
path SenDeriv2Sym = concat(&SenDeriv2, flip_x(&SenDeriv2))
```

En `fig4-1.mg` se transpone en el sitio del `fit` (los ejes intercambiados) y se coloca normal:

```text
fit(transpose(&SenDeriv2Sym)) { 0.35 0  0.9 1 }   % patrón de una rendija
```

`fit` usa el **bbox real** del path transpuesto → sin `world_window`, sin `normalize`, sin rotar.

### E2 — plot genérico de datos con ejes cambiados

```text
path serie = { 0 0   1 2   2 3   3 5   4 4 }     % (x, y) medidos
polyline(&serie)                                  % barras/perfil vertical
polyline(transpose(&serie))                       % el mismo dato, horizontal
```

## Implementación (dos caminos, según ambición)

`transpose` es de las operaciones **más baratas** del motor (solo intercambia coordenadas),
pero necesita un punto de entrada en la gramática. §9 hoy **no está cableado** (ver
`plan_lmmath.md`/roadmap: `path`/`&`/`reverse`/… existen en la spec y parte en `splines.cpp`,
pero el parser no los tiene).

- **(A) Mínimo — `transpose` como primera pieza del álgebra §9.** Cablear: `path ID = PathExpr`
  (objeto path nombrado), `&ID` (referencia), y `transpose(&p)` como op unaria que mapea los
  puntos. Es el subconjunto más chico de §9 que habilita el caso. Reusa el `Path`
  (`std::vector<point>`) que ya existe. ~localizado en `parserv3.cpp` (parse de `path`/`&` +
  la op) + un helper en el motor.
- **(B) Completo — §9 con `transpose` incluido.** Cablear el álgebra entera (`reverse`,
  `concat`, `tile`, `normalize`, `fitrect`, `spline`/`smooth`) y `transpose` es una op más.
  Mayor alcance; solo si hay más de un ejemplo que empuje §9.

Recomendación: **ruta (A+)** = el mínimo (A) **más** `flip_x`/`flip_y` y `concat`, dirigido por
fig4-1 como ejemplo guía (ver "Estado" abajo). Habilita `transpose` + las Sym con la maquinaria
justa y deja el resto de §9 para cuando el corpus lo pida. `concat` reusa `concat_paths` de
`splines.cpp`, así que aporta poco código nuevo.

## Decisiones de la revisión + lo que queda

**1ª ronda (2026-07-10):**
1. **Nombre:** ✔ `transpose`. Alternativas descartadas: `swap_axes`, `flip_diagonal`.
2. **Operando:** ✔ **cualquier path** (literal `{ … }` o `&nombre`), pero **solo un path** (no
   bloques/structs).
3. **`fit`/`draw` de un path usan su bbox real:** ✔ sí (transpose "just works", sin redeclarar
   `world_window`).
4. **Restringido a paths:** ✔ sí. Un `transpose` de bloque/struct tendría el enredo del
   transform de marco (transpone también el placement); queda fuera a propósito.

**2ª ronda (2026-07-10) — CERRADO:**
5. **Reflexión = `flip_x` / `flip_y`** (ops sueltas, no parámetro de `transpose`). `scale` de
   datos → futuro. Ver "Operaciones de path decididas".
6. **`normalize`** pospuesto (fit encuadra el bbox exacto); documentado, fuera de fig4-1.
7. **`curvas3.mg` = puro dato**: medias curvas verbatim + simétricas vía `concat`+`flip_x`;
   `transpose` y `fit` viven en `fig4-1.mg`. Ver E1.
8. **`concat` reutiliza `concat_paths` (`splines.cpp`)** — suelda+orienta (reverse+traslación
   implícitos). Verificar contra oráculo; posible mejora "extremo más cercano" no bloqueante.
9. **`concat` como función**; álgebra simbólica de paths (`~` etc.) → futuro, no urge.
10. **Orientación / det −1:** con la reflexión expresada como `flip_x` explícito, la inversión
    de orientación es intencional y local; solo hay que **verificar visualmente** que el perfil
    simétrico casa con el oráculo (irrelevante en estas curvas abiertas; importaría en contornos
    cerrados con relleno par-impar — fuera de alcance aquí).

### Estado — IMPLEMENTADO (2026-07-11)

Ruta (A+) cableada en `src/parserv3.cpp`: `path ID=<PathExpr>`, `&ID`, `transpose`, `flip_x`,
`flip_y`, `concat`, y `polyline(<PathExpr>)` / `fit(<PathExpr>){rect}`. Jerarquía `PathExpr`
aparte de Expr/Value + registro `g_paths` (calcado de `g_structs`). Lexer intacto.

**Cambio respecto al plan:** `concat` **NO** reusó `concat_paths` — su heurística de reverse
POR-X soldaba por la cola y daba **doble pico**. Implementé la mejora **"extremo más cercano"**
(que el plan marcaba como opcional) dentro del wrapper `PathConcat` de V3: elige el par de
extremos más cercano, invierte `a`/`b` según convenga y suelda. Así `concat(&H, flip_y(&H))`
—media curva + su espejo en la posición— solda por el **pico compartido** y da **un pico
central**, limpio e intuitivo, sin depender de la orientación. (`concat_paths` queda intacto
para el front-end V1.) `reverse` sigue implícito dentro de `concat`.

`curvas3.mg` = puro dato (`SenDeriv2`/`SenCosDeriv2` `(t,I)` verbatim + `Sym = concat(&H,
flip_y(&H))`); `fig4-1.mg` transpone en el `fit`. **Verificado:** 15 ejemplos compilan y pasan
Ghostscript; fig4-1 SVG **pixel-idéntico** al struct viejo y al oráculo (57→55 paths: cada Sym
pasó de 2 trazos a 1 continuo); EPS y PDF OK. Pendiente: commitear; documentar §9 en la spec.

El resto de §9 (`tile`/`normalize`/`fitrect`/`spline`/`smooth`) sin cablear hasta que el corpus
lo pida.

## Fuera de alcance

- El resto del álgebra §9 (salvo lo mínimo para `transpose`).
- Transponer texto/dibujos completos (no es el caso de uso; reflejaría el texto).
