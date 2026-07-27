# plan_anisotropia — la familia de bugs «fórmula isótropa en caso anisótropo»

> **Estado (2026-07-27):** tres instancias CERRADAS, dos ABIERTAS por decisión de
> semántica (no por defecto). Documento de motor, de vida larga: sobrevive a la figura
> que lo destapó. Ver `docs/bitacora.md`, entradas del 2026-07-27 y 2026-07-27 (bis).

## La firma

Una fórmula geométrica que **solo vale si la transformación preserva ángulos o círculos**,
aplicada donde la transformación es anisótropa (escala por eje distinta, shear, o un marco
`stretch`). Se reconoce por cuatro tics:

1. Un **radio o longitud escalado por un solo factor** (o por la norma de una columna).
2. Un **`±90°`** para obtener una perpendicular o una tangente.
3. Un **`atan2` sobre una columna de la matriz** tomado como «la rotación».
4. Un **ángulo calculado en mundo** usado como si fuera un ángulo en **dispositivo**.

⚠️ Es una familia **silenciosa**: en el caso isótropo —que es casi todo el corpus— cada
una de esas fórmulas da el resultado correcto. Solo se rompen cuando una figura empieza a
girar o deformar cosas, y entonces salen todas juntas. Las tres cerradas llevaban meses en
el árbol y aparecieron el mismo día.

⚠️ **Y el golden las bendice.** Es autorreferencial: si el motor cambia y se re-bendice, el
error queda enshrinado. La única compuerta que las ve es la paridad geométrica entre
backends (invariante (c) de Capa 3, `tools/arcparity.py`), y solo para arcos.

## Cerradas

| # | Dónde | Qué asumía | Arreglo |
|---|---|---|---|
| 1 | `Matrix::transform_radii` (los 3 backends) | Los ejes de la elipse imagen = normas de columna. Son los semidiámetros **conjugados**, y solo coinciden con los ejes si u⊥v | `ellipse_frame` (centro + conjugados, cerrado bajo afinidad) + `ellipse_axes` (SVD 2×2 en forma cerrada) |
| 2 | Bloques de «corrección de signo» en `arc()` | `ea = sa + endAng`: el ángulo final absoluto tratado como **barrido**. Un arco de 190→350 (160°) salía de 350° | Desaparecieron: EPS recibe la matriz y traza el arco unitario con los ángulos intactos |
| 3 | `StructureArc::draw_side` | `angf ± 90` como tangente. Solo vale en el **círculo** | Tangente paramétrica real, `atan2(dir·ry·cosθ, −dir·rx·sinθ)` |

## Abiertas — son DECISIONES, no defectos

Ninguna la alcanza el corpus. Ninguna tiene respuesta obviamente correcta: hay que **elegir**
la semántica y documentarla. No «arreglar» sin decidir.

### A. El radio de un arco en la ruta LOG de `plot` (`src/parserv3.cpp:3455`)

`plot` tiene dos rutas. La **lineal** envuelve el contenido en una matriz, así que el radio
se transforma con todo lo demás (y un `circle` en un marco anisótropo sale elipse, que es lo
que la spec espera — §13.7 dice que por eso `dot` es físico). La **log** no puede usar matriz
(el log no es afín) y mapea los puntos **uno a uno**: transforma el **centro** del arco pero
deja el **radio**, que vive en un miembro aparte del `Arc`, no en el path.

Medido con el mismo `circle(0.5)` en dos marcos de idénticas proporciones:

```
plot lineal → mgarc [7.08661 0 0 0.644238 …]   elipse de 11:1  (radio en unidades de DATOS)
plot log    → 240.94 77.95 14.173 … arc        círculo de 14.17pt (radio en unidades de MARCO)
```

**La decisión:** en un plot log una circunferencia de datos **no puede** mapear ni a círculo ni
a elipse (el log no es afín), así que no hay respuesta exacta. Hay que elegir entre:
- (a) linealizar el radio en el centro → elipse aproximada, coherente con la ruta lineal;
- (b) declarar que en la ruta log el radio es cantidad de **marco** → círculo verdadero en el
  papel, y documentarlo como divergencia deliberada.

Hoy hace (b) por accidente, sin decirlo. Mínimo: documentarlo. `fig6-4`, el único plot log del
corpus, usa `dot` (físico), que es justo lo que la spec recomienda para marcos deformados.

### B. La dirección «out» de las marcas de eje (`src/parserv3.cpp:2622`)

`px = uy, py = -ux` es la perpendicular al eje calculada en **mundo**, y `physOut` la usa como
dirección en **dispositivo**. Bajo `stretch` esas dos perpendiculares no coinciden.

**Latente:** las dos líneas siguientes fuerzan el lado «out» a puramente abajo o puramente
izquierda, y para un eje **alineado a los ejes** —los únicos que hay— ambas perpendiculares
coinciden. Solo se rompe con un eje **diagonal bajo stretch**.

**La decisión:** si «out» debe ser perpendicular en el **mundo** (coherente con los datos) o en
el **papel** (coherente con la vista). Tampoco es obvio. Sin figura que lo pida, no tocar.

## Verificado LIMPIO — no volver a revisar

- **Orientación de marcadores.** `Dot::draw` pasa un **vector** de dirección en mundo y el
  backend transforma dos puntos (ancla y ancla+dir) para medir el ángulo ya en dispositivo.
  Es el patrón correcto, y explica por qué las flechas nunca fallaron pese a todo lo demás.
- **`StructureLine` / `StructurePath`.** Componen `rotate` DENTRO de la matriz, antes de la
  ambiente: rotar-y-luego-transformar ≡ transformar lo rotado. Correcto.
- **`physOut`, `ptX`, `ptY`** (`parserv3.cpp:2634/3014/3143`). Dividen por `scx` y `scy` por
  separado, con el colapso a `min()` cuando no hay stretch.
- **`getRatio` y las compensaciones de aspecto de V1:** eliminadas, sin rastro.
- **Bounding boxes.** El del arco se arregló el 2026-07-27 (`hypot(ux,vx)`, `hypot(uy,vy)`,
  exacto); los demás son punto a punto.
- **`line_width`, `dash`, tamaño de `dot`/`marker`, `hatch_gap`:** físicos por diseño, inmunes
  por construcción.

## Cómo cazar más

El barrido que encontró estas fue: `grep` de `atan2`, de `± 90`, de las conversiones pt↔mundo,
y de los `set_limits`/`adjust_limits`; luego **leer cada hit** preguntando «¿qué asume esto de
la matriz?». Es rápido (media hora) y conviene repetirlo cuando entre geometría nueva.

La prueba decisiva no es leer: es **construir el caso anisótropo y medirlo**. Un círculo bajo
`scale 2 1` seguido de `rotate 45` destapa la familia entera en un renglón — si sale círculo,
algo usó una norma de columna.
