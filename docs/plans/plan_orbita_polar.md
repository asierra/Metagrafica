# plan_orbita_polar — la figura y lo que destapó

> **Estado (2026-07-27):** tres de cuatro problemas CERRADOS. Queda la oclusión, con la
> receta ya **verificada** abajo. `examples/orbita_polar.mg` **no está en el corpus**
> (`test/run.sh`) ni tiene `docs/img`: hoy no lo vigila ninguna compuerta. Ver
> `docs/bitacora.md` 2026-07-27 y 2026-07-27 (bis), y `plan_anisotropia.md`.

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

## Pendiente 1 — ocultar la mitad trasera de la órbita

**No hace falta motor booleano.** El comentario original proponía usar intersección de paths,
pero la oclusión es una cuestión de **profundidad**, no de conjuntos en 2-D: una intersección
de contornos no sabe qué mitad está detrás. Y como órbita y globo son **concéntricos**, el
problema tiene forma cerrada.

**La geometría.** La órbita es la proyección de un círculo 3-D de radio `b` inclinado; su eje
**mayor** es la línea de nodos (z = 0), así que en el marco propio de la elipse
`P(t) = (a·cos t, b·sin t)` la mitad **lejana** es `t ∈ (90°, 270°)`. El cruce con el limbo
sale de `|P(t)|² = R²`:

    a² + (b² − a²)·sin²t = R²   →   sin t = ±√((R² − a²)/(b² − a²))

Oculto = lejano ∩ dentro del disco = `t ∈ (180−tc, 180+tc)` con `tc = asin(√(…))`.
Con `a=3.4, b=6.2, R=5` da `tc = 45°`, o sea oculto `t ∈ (135°, 225°)` y visible el resto.

**La receta, ya verificada** (emite `225 495 mgarc`, barrido de 270°, y el render es correcto —
los tramos lejanos que caen FUERA del disco siguen dibujados, que es lo que debe pasar):

```
a = 3.4
b = 6.2
R = earth_radius
s  = sqrt((R*R - a*a)/(b*b - a*a))
tc = atan2(s, sqrt(1 - s*s)) * 180/pi        % = asin(s) en grados; no hay asin, atan2 lo da
arc(a, b, from=(180+tc), to=(540-tc)) { 0 0 }   % en vez de ellipse(a, b)
```

⚠️ **Cuál de las dos mitades está detrás es una decisión de modelado**, no la dicta la
geometría 2-D: la otra convención es `from=tc, to=(360-tc)`. Hay **dos** órbitas (±15°) y hay
que elegir para cada una y **mirarlo**. Ese es el trabajo que queda: no es matemática, es
decidir cómo se lee la figura.

⚠️ Al pasar de `ellipse` a `arc` hay que reponer `marker_at` (los ángulos siguen siendo los
mismos; el marcador en 180° cae en la parte oculta y hay que moverlo o quitarlo).

**El lenguaje ya alcanza.** `sin`, `cos`, `tan`, `sqrt`, `abs`, `atan2`, `exp`, `ln`, `mod` y la
constante `pi` están en `include/ast.h:137,228-243`. **No hay `asin`/`acos`** — `atan2` los
expresa, como arriba. Si esto se repite en otra figura, valdría añadirlos (y quizá `deg`/`rad`),
pero por la política de demanda: hasta que una segunda figura lo pida.

## Pendiente 2 — decidir si la figura entra al corpus

Hoy no la vigila nada. Si entra a `EXAMPLES` de `test/run.sh` gana goldens en tres backends y
paridad geométrica; si además se le hace `docs/img/orbita_polar.svg`, entra a `imgfail` y a la
galería. Requisitos: encabezado con la convención de 2026-07-23 (1ª línea = título, párrafo =
descripción, `% NOTAS ———` para lo demás) y que `lib/mapa_p30_n55.mg` siga en el árbol.

⚠️ Es también el único cliente de `arc(rx,ry)`, `marker_at` y `place(rx/ry, at=)`: si no entra,
esas tres características quedan **sin cobertura de pruebas**.

## Suelto relacionado

- **`examples/test_sat.mg`** sigue sin trackear. Su cobertura (arco parcial girado y reflejado)
  ya vive en `rpstest`; según la política de efímeros toca borrarlo. Decisión de Alejandro.
- **Constantes de Mortensen para `arc_bezier`** (referencia en `plan_boolean_paths.md`):
  reducirían ~5× la deriva radial de la aproximación Bézier del PDF (hoy ≈2.7e-4·R, la cota de
  Van Aken, que es la que fija la tolerancia de `tools/arcparity.py`). ⚠️ Su método **no** pasa
  exactamente por los extremos del arco (`a = 1.00005507808`), y solo aplica limpio a segmentos
  de 90°. Mejora opcional, no defecto.
- **Precisión de impresión del SVG.** `SVGDisplay` emite 6 cifras significativas; cerca del caso
  degenerado de 180° la conversión extremos→centro amplifica ese redondeo ~150× (0.0005 pt →
  0.075 pt de corrimiento del centro). Invisible, y `arcparity` ya lo modela. Subirla lo quitaría
  de raíz pero movería **todos** los goldens SVG y todo `docs/img` a cambio de nada visible.
