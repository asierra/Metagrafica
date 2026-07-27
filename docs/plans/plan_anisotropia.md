# plan_anisotropia — la familia de bugs «fórmula isótropa en caso anisótropo»

> **Estado (2026-07-27): CERRADO.** Tres instancias arregladas y las **dos decisiones de
> semántica tomadas y escritas** (abajo). Documento de motor, de vida larga: sobrevive a la
> figura que lo destapó, y su valor duradero es «La firma» y «Cómo cazar más» — el resto es
> el registro de lo ya resuelto. Ver `docs/bitacora.md`, entradas del 2026-07-27.

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

## Decididas (2026-07-27) — eran DECISIONES, no defectos

Ninguna la alcanzaba el corpus y ninguna tenía respuesta obviamente correcta, así que se
**eligió la semántica y se escribió**, en vez de «arreglar» sin decidir. Ambas quedaron sin
tocar una línea de C++.

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

✅ **DECIDIDO: (b), enunciado como regla — «en la ruta log, `plot` mapea POSICIONES, no
formas».** Los tamaños quedan en coordenadas de la página. No es un accidente tolerado: es la
generalización de lo que esa ruta **ya hacía** con `line_width`, la tipografía, el radio de
`dot`/`marker` y `hatch_gap`; el radio de `circle`/`arc`/`ellipse` y el `width` de `polybar`
(§13.6) entran a la misma regla. Escrito en `especificacion_mg.md` §13.7 y en
`docs/referencia.md` §11, con el ejemplo compilado.

Se descartaron las otras dos:
- **(a) linealizar el radio en el centro** (elipse de la derivada local) — exacta solo en el
  límite infinitesimal: produce una figura que *parece* calculada sin estarlo. Bajo un
  logaritmo una circunferencia de datos no es ni círculo ni elipse, es un huevo asimétrico.
- **(c) rechazarlo como error** — bajo la regla (b) la figura resultante tiene un significado
  claro (un círculo trazado sobre la hoja, en la posición que le toca): poner una guarda
  pagaría el costo de prohibir sin el beneficio de resolver.

⚠️ **Costo aceptado y documentado, no oculto:** el mismo `circle(0.5)` mide distinto según el
eje sea lineal o log. **Avisa la documentación, no el compilador.** Para marcar datos, `dot`.

### B. La dirección «out» de las marcas de eje (`src/parserv3.cpp:2623`)

`px = uy, py = -ux` es la perpendicular al eje calculada en **mundo**, y `physOut` la usa como
dirección en **dispositivo**. Bajo `stretch` esas dos perpendiculares no coinciden.

**Latente por partida doble:** hacen falta a la vez `world_window … stretch=true` a nivel de
documento —que **ningún ejemplo del corpus usa**; los tres que dicen `stretch` lo hacen en
`fit`, que es otro mecanismo— y un eje **diagonal**, que hoy no existe (las dos líneas
siguientes fuerzan «out» a abajo o a la izquierda, y para ejes alineados ambas perpendiculares
coinciden).

✅ **DECIDIDO: perpendicular en el PAPEL**, como corolario de una regla que ya existía. Todo lo
demás de una marca de eje es físico —`tick_size` en puntos, grosor, tipografía—: una marca es
mobiliario, un trazo legible que sale del eje, no un vector con sentido en el espacio de datos.
Una longitud física en una dirección derivada del mundo es un híbrido sin dueño.

**Sin cambio de código, a propósito:** no hay figura que lo alcance. Cuando aparezca, el arreglo
es calcular la perpendicular **después** de pasar a dispositivo, no antes.

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
