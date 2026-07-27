# plan_orbita_polar — la figura y lo que destapó

> **Estado (2026-07-27): CERRADO.** Los cuatro problemas resueltos y la figura **en el
> corpus** (`ok=72`, 24 ejemplos), con `docs/img/orbita_polar.svg` y tarjeta en la galería:
> hoy la vigilan las seis compuertas. Ver `docs/bitacora.md` 2026-07-27 y
> `plan_anisotropia.md`, que es el hilo que sigue vivo.

Órbitas polares alrededor de la Tierra en proyección ortográfica, con satélite. Es la figura
que pidió los arcos elípticos y, de paso, destapó tres bugs de motor.

## Cerrado

1. **Arcos elípticos.** `arc` no aceptaba `rx`/`ry` pese a estar en §4.5 desde siempre: no
   había cliente. La flecha «Rotación de la Tierra» es el primero.
2. **Rotación de arcos y elipses**, en los tres backends. Tres bugs, una causa raíz
   (`plan_anisotropia.md`). El arco de la antena del satélite ya gira bien.
3. **Órbitas concéntricas.** `rotate` gira el plano entero, así que con el centro en
   `{0 earth_y}` el propio centro se desplazaba y las dos elipses quedaban descentradas
   ~2.6 mm hacia lados opuestos. Se lleva el origen al centro con `translate`.
4. **Flechas de sentido** (`marker_at`) y **satélite sobre la órbita** (`place(..., rx/ry, at=)`).

## Cerrado — ocultar la mitad trasera de la órbita

**No hizo falta motor booleano.** El comentario original proponía intersección de paths,
pero la oclusión es una cuestión de **profundidad**, no de conjuntos en 2-D: una intersección
de contornos no sabe qué mitad está detrás. Y como órbita y globo son **concéntricos**, el
problema tiene forma cerrada, evaluable en el propio `.mg` con la trigonometría que el
lenguaje ya tenía.

**La geometría.** La órbita es la proyección de un círculo 3-D de radio `b` inclinado; su eje
**mayor** es la línea de nodos (z = 0), así que en el marco propio de la elipse
`P(t) = (a·cos t, b·sin t)` la mitad **lejana** es `t ∈ (90°, 270°)`. El cruce con el limbo
sale de `|P(t)|² = R²`:

    a² + (b² − a²)·sin²t = R²   →   sin t = ±√((R² − a²)/(b² − a²))

Oculto = lejano ∩ dentro del disco = `t ∈ (180−tc, 180+tc)` con `tc = asin(√(…))`.
En la figura terminada `tc ≈ 56.8°`, o sea 113.6° ocultos.

```
s  = sqrt((R*R - a*a)/(b*b - a*a))
tc = atan2(s, sqrt(1 - s*s)) * 180/pi        % = asin(s) en grados; no hay asin, atan2 lo da
arc(a, b, from=(180+tc), to=(540-tc)) { 0 0 }   % en vez de ellipse(a, b)
```

**Los cortes caen sobre el limbo por construcción**, no por ajuste: el disco del mapa es un
`circle(1)` escalado por `scale=earth_radius`, o sea el mismo `R` y el mismo centro que la
ecuación. Verificado a 1600 px: el trazo termina dentro del ancho de línea del limbo.

**Cuál de las dos mitades está detrás resultó ser una decisión de modelado**, no la dicta la
geometría 2-D. Se eligieron **opuestas** —la de +15° esconde su izquierda (`from=180+tc`),
la de −15° su derecha (`from=tc`)— para que se lean como dos planos que se cruzan y no como
copias de uno solo. Con media órbita oculta queda **una** flecha por órbita, en lados
opuestos: `marker_at=[360]` en la primera (360 = 0, pero dentro del barrido 225→495) y
`[180]` en la segunda.

**El lenguaje alcanzó.** `sin`, `cos`, `tan`, `sqrt`, `abs`, `atan2`, `exp`, `ln`, `mod` y la
constante `pi` están en `include/ast.h:137,228-243`. **No hay `asin`/`acos`** — `atan2` los
expresa, como arriba. Si esto se repite en otra figura, valdría añadirlos (y quizá `deg`/`rad`),
pero por la política de demanda: hasta que una segunda figura lo pida.

## Cerrado — la figura entró al corpus

`orbita_polar` está en `EXAMPLES` de `test/run.sh` (`ok=72`, 24 ejemplos × 3 backends), tiene
`docs/img/orbita_polar.svg` —así que entra a `imgfail`— y tarjeta en la galería. Es el
**único cliente** de `arc(rx, ry)`, `marker_at` y `place(..., rx/ry, at=)`, y el primer
ejemplo del corpus con arcos elípticos **girados**, que es lo que vigila la invariante (c)
de la Capa 3. Encabezado a la convención de 2026-07-23.

**Escala real (2026-07-27).** El semieje mayor se dejó de escribir a mano: sale de
`orbit_km`/`earth_km` sobre `earth_radius`, y el menor de un `plane_angle` declarado. El
rótulo se arma con `str(orbit_km)`, así que la geometría y el letrero no pueden divergir.
Se descubrió comparando: la lámina de origen (y nuestra primera versión, con `b=6.2`)
dibujaba la órbita a ~1500 km mientras el letrero decía 800. Ajustarlo fue cambiar **dos
declaraciones**; `tc`, los satélites y las flechas se recolocaron solos — que es la prueba
de que la figura está calculada y no ilustrada. A 800 km la órbita se ve rasante (12.6%
sobre la superficie): es honesto y se lee distinto, y se aceptó ese cambio de lectura.

**Segundo satélite (2026-07-27).** En la órbita de atrás, a `at=[270]`: el extremo inferior
del eje mayor, que cae a 2° de la dirección en que se proyecta Bolivia (lat −17, lon −65) en
la vista lat 30, lon −55 del mapa —`(−0.83, −3.62)` desde el centro, o sea −102.9°—, así que
sobrevuela Sudamérica. Por ser extremo del eje mayor su tangente es horizontal y queda
acostado, en contrapunto al de arriba. Se compararon 215/240/250/270 sobre el render; 215
—delante del globo, a la altura de Bolivia— quedó como la alternativa descartada.

## Suelto relacionado

- **`examples/test_sat.mg`** ya no está en el árbol; su cobertura (arco parcial girado y
  reflejado) vive en `rpstest`.
- **Constantes de Mortensen para `arc_bezier`** (referencia en `plan_boolean_paths.md`):
  reducirían ~5× la deriva radial de la aproximación Bézier del PDF (hoy ≈2.7e-4·R, la cota de
  Van Aken, que es la que fija la tolerancia de `tools/arcparity.py`). ⚠️ Su método **no** pasa
  exactamente por los extremos del arco (`a = 1.00005507808`), y solo aplica limpio a segmentos
  de 90°. Mejora opcional, no defecto.
- **Precisión de impresión del SVG.** `SVGDisplay` emite 6 cifras significativas; cerca del caso
  degenerado de 180° la conversión extremos→centro amplifica ese redondeo ~150× (0.0005 pt →
  0.075 pt de corrimiento del centro). Invisible, y `arcparity` ya lo modela. Subirla lo quitaría
  de raíz pero movería **todos** los goldens SVG y todo `docs/img` a cambio de nada visible.
