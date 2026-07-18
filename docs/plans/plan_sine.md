# Plan: primitiva `sine` — fase 2 (phase≠0 y half_cycles fraccionario)

Estado y hoja de ruta de la primitiva `sine` (§4.13). El **núcleo ya está
implementado** (commit del parser V3); este documento cubre lo **diferido**,
para que Alejandro lo estudie o un subagente lo implemente sin re-derivar el
contexto.

## Qué ya está hecho (núcleo)

`SineStmt` en `src/parserv3.cpp` expande `sine(...)` a un `Polyline(GI_BEZIER)`
en tiempo de evaluación (sin clase de motor; reutiliza `curveto`). Cubre:

- **`phase=0`** (seno): `half_cycles=n` entero → `n` jorobas de la plantilla
  `&sinpi` con **signo alterno** `(−1)ᵏ`. Puntos de control = los de V1
  (`examples/v1/bzsinepaths.mg`): `C1=(0.41, 1.335·A)`, `C2=(0.59, 1.335·A)`,
  `P=(1, 0)` por joroba (pico = amplitud `A`).
- **`squared=true`** (|ψ|²): `n` bumps positivos de la plantilla `&cos2pi`
  invertida (sin²(πt)): dos cúbicas por bump, todas positivas.
- **Base arbitraria**: `û` = dirección de la base, `v̂` = perpendicular (+90°),
  ancho por joroba `w = L/n`; cada control local `(x, y)` → mundo
  `p1 + x·û + y·v̂`.

**Hallazgo que garantiza el match con el oráculo**: `&sin2pi` de V1 (que usaba
para pares de jorobas) es *algebraicamente idéntico* a dos `&sinpi` con signo
alterno alrededor de la línea base (controles normalizados `(0.41, ±1.335)`,
`(0.59, ±1.335)`). Por eso el ensamblado limpio "n jorobas alternas" reproduce
la aproximación de V1 sin replicar su mezcla manual sinpi/sin2pi/RPPT.

Verificado: analítico (amplitud, alternancia, bumps positivos) + estructura de
fig4-10 (phi n=1..4 → 1,2,3,4 cúbicas; rho → 2,4,6,8). fig4-10 NO es byte-exacto
al oráculo por diseño (es un rediseño limpio de encuadre; §22.6).

## Diferido 1 — `phase` — **HECHO** (commit `2e1ba32`, enfoque de cuartos; rama conservadora)

Implementado tal como se recomienda abajo (rama de cuartos SOLO para phase≠0;
el núcleo phase=0 quedó intacto). Además (2026-07-18) `sine` es también
**expresión de path** del álgebra §9 (`path p = sine(...) { base }`): con
phase=90/270 cada llamada es un medio ciclo de coseno entre extremos con
pendiente cero — la pieza con que fig16-9 arma funciones de onda con
envolvente por tramo vía `concat`. Sigue pendiente solo `half_cycles`
fraccionario y `phase`+`squared` (aviso y dibuja phase=0).

### Enfoque recomendado: generalizar a **cuartos de ciclo** (piezas de 90°)

En vez de jorobas de medio ciclo (`&sinpi`), descomponer la onda en **cuartos de
ciclo** (90° cada uno) — así cualquier `phase` múltiplo de 90° cae en frontera de
pieza y se arma con piezas enteras (sin recortar). Cada cuarto es un bezier
monótono derivado de `&sinpi2` (`0 0  0.417 0.6667  0.693 1  1 1`, cuarto de seno
0→π/2, sube 0→1) y sus reflexiones:

| Cuarto | Forma | y inicial→final | Construcción desde `&sinpi2` |
|---|---|---|---|
| Q0 | sube | 0 → +1 | `&sinpi2` directo |
| Q1 | baja | +1 → 0 | `&sinpi2` reflejado en x (espejo horizontal) |
| Q2 | baja | 0 → −1 | Q0 con y negada |
| Q3 | sube | −1 → 0 | Q1 con y negada |

Un seno = `[Q0, Q1, Q2, Q3]` repetido; `half_cycles=n` = `2n` cuartos. La `phase`
(múltiplo de 90°) **rota el punto de arranque** en la secuencia de 4:

- `phase=0` → arranca en Q0 (sube desde 0). Es el seno actual.
- `phase=90` → arranca en Q1 (baja desde +1). Es el **coseno** (empieza en el
  máximo). fig6-1.
- `phase=180` → arranca en Q2; `phase=270` → Q3.

`amplitude` escala la `y`; el mapeo local→mundo (`û`/`v̂`) es el mismo del núcleo.

**Compatibilidad**: reimplementar el núcleo phase=0 sobre cuartos debe dar los
MISMOS puntos que las jorobas `&sinpi` actuales (Q0+Q1 = una joroba `&sinpi`).
Verificar que `&sinpi2`+reflejo reconstruye `&sinpi` (0 0 / 0.41 1.335 / 0.59
1.335 / 1 0) antes de cambiar, para no romper fig4-10. Alternativa conservadora:
dejar el núcleo phase=0 como está y añadir SOLO una rama de cuartos para
phase≠0 (menos elegante, cero riesgo de regresión).

**`squared` + `phase`**: son ejes independientes (spec §4.13). `phase` desplaza el
muestreo ANTES de elevar al cuadrado. `sin²` con fase desplazada = `|onda|²` de la
onda ya desfasada; con phase múltiplo de 90° las piezas siguen siendo enteras.
`&cos2pi` (raised, todo positivo) es el caso `phase=0, squared`; un `phase=90,
squared` levanta el coseno² (mismo template, arranque en el máximo).

## Diferido 2 — `half_cycles` fraccionario

`half_cycles` puede ser fraccionario (onda parcial, spec §4.13). Si el resto no
cae en frontera de 90°, hay que **recortar la última pieza con de Casteljau**
(subdividir la cúbica en el parámetro `t` que corresponde a la fracción sobrante y
quedarse con el trozo). No hay caso en el corpus todavía → diferido hasta que uno
lo exija. Con de Casteljau, cualquier `half_cycles` y cualquier `phase` (no solo
múltiplos de 90°) quedan soportados con la misma maquinaria de cuartos.

Subdivisión de Casteljau de una cúbica `P0 P1 P2 P3` en `t`:
```
A = lerp(P0,P1,t);  B = lerp(P1,P2,t);  C = lerp(P2,P3,t)
D = lerp(A,B,t);    E = lerp(B,C,t)
F = lerp(D,E,t)      # punto sobre la curva en t
# sub-cúbica [0,t] = P0 A D F ; sub-cúbica [t,1] = F E C P3
```

## Verificación sugerida (fase 2)

- **phase=90**: comparar contra fig6-1 (WKB). ⚠ fig6-1 puede necesitar calibración
  de encuadre; verificar la FORMA (arranca en el máximo, 2 medios ciclos) y, si el
  encuadre coincide, byte a byte.
- **Muestreo analítico**: `examples/v1/bzsinepaths-examples.mg` tiene puntos de
  seno computados (`CRSINEPI2`) para medir el error de la aproximación bezier.
- Nuevo showcase: `examples/v3/sines.mg` (solo usa `sine`; añadir filas phase=90
  cuando esté).
