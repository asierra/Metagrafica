# Plan para simulación pseudo-3D (proyección oblicua) en MG V3

Objetivo: dar soporte fiel a la ilustración científica 2.5D (planos que "receden",
prismas, pantallas inclinadas) manteniendo la filosofía 2D del lenguaje —espacio
isométrico por construcción (§3.1), ortogonalidad forma/posición, y la regla sagrada
`Subpath ::= (Coord Coord)+` intacta— sin introducir coordenadas `x y z` ni matemática
3D en el motor.

## Punto de partida (corregido respecto al draft anterior)

El draft anterior asumía que había que construir en C++ una máquina que "calcula la
matriz afín exacta". **Eso ya existe.** La cizalla es ciudadana de primera clase en V3:

- Sentencia de transformación local §11.1: `shear <sx> <sy>` (`parserv3.cpp` map en
  `transformOp`, exec en `TransformStmt`, con `using_ellipse=true` para que arcos →
  elipse bajo deformación). Los argumentos pasan por `parseTerm` → **aceptan
  expresiones y parámetros** (verificado); hay `sin`/`cos`/`tan`/`pi`/`round`.
- Constructor `transform=` §17: compone `shear`/`rotate`/`scale`/`translate`.
- `Matrix::shear` (`matrix.cpp`) = cizalla homogénea estándar.

**Verificado**: metiendo `shear 0.55 0` dentro del struct `Caja` de `fig10-2v3.mg`, los
tres planos salen como paralelogramos en EPS/SVG/PDF. El efecto pseudo-3D del `SHST 0
1.1` del V1 se reproduce **sin tocar el motor**. Lo que falta para clavar
`fig10-2.png` es fidelidad de relleno (gris + contorno, era `FGRAY -80`) y calibrar
eje/factor del shear, no capacidades nuevas.

## Taxonomía (decisión: priorizar OBLICUA)

Los ejemplos del corpus (`fig10-2`, `fig2-7`) son **proyección oblicua / caballera**:
la cara frontal conserva su forma real y recede por **un solo eje cizallado**. Es un
`shear` (más quizá `scale` de escorzo en profundidad). La **isométrica verdadera**
(tres ejes a 120°, todo escorzado) no tiene ejemplo en el corpus y queda como caso
posterior. Este plan diseña la oblicua primero; la isométrica se añade luego como
otro preajuste de la misma biblioteca.

Nota de nomenclatura: **no** llamar `isometric(...)` a la proyección de plano. El
motor ya es "isométrico por construcción" (§3.1, semilla *meet* del mundo); reusar el
término confunde. Términos libres: `oblicua`/`oblique`, `plano`/`plane`, `prisma`.

## Mecanismo (decisión: BIBLIOTECA .mg PURA)

Un plano oblicuo *es* una composición de `scale·rotate·shear` que `transform=` y las
sentencias §11.1 ya saben componer. Por tanto se ofrece como **biblioteca estándar
includible** (`lib/pseudo3d.mg`), construida sobre structs paramétricos (§8) y las
transformaciones existentes. **Cero C++.** Coherente con la filosofía del proyecto
(toda feature en el compilador, sin cajas negras; ver `project_philosophy`). Solo se
consideraría subir algo a builtin si un límite concreto del lenguaje lo impide, y se
anotaría aquí antes de hacerlo.

Un único mecanismo: **sentencia de estado con alcance** (como `translate`/`rotate`),
no atributo por-primitiva. No duplicar el concepto en `TransformCall` y en `AttrName`.

## Fases (ordenadas por lo que el corpus pide)

### Fase 0 — Portar fig10-2 (HECHA, sin motor)
`examples/simulate3d/fig10-2v3.mg` reproduce `fig10-2.png`: `shear 0.4 0` en un bloque
acotado (solo planos + esquina; cotas/enlaces sin cizallar); relleno `gray(0.8)` con
contorno (= `FGRAY -80`); base angosta vía `world_window 0 2.5 0 1` para el aspecto
alto-angosto del original. Compila EPS/SVG/PDF. **Aprendizaje:** el factor de shear
`k` domina el aspecto visual; el `world_window` del struct fija la base.

### Fase 1 — `lib/pseudo3d.mg` (HECHA, sin motor)
Structs paramétricos con **vértices COMPUTADOS** de los params (no gimnasia de ventana
ni shear ambiental): la proyección oblicua se hornea en la geometría del paralelogramo,
así cada pieza se coloca con `at=`/`scale=`/`rotate=` y compone predeciblemente.
- `plano(w, h, k=0.4, relleno, contorno, lw)` — cara rectangular oblicua (polígono con
  el borde superior desplazado `k·h`).
- `prisma(w, h, d, a=35, f=0.6, frente/techo/lado, …)` — caja de 3 caras sombreadas;
  profundidad proyectada a `a` grados con factor `f` (1=caballera, 0.5=gabinete),
  dibujada de atrás hacia adelante.
- Trig en **radianes** (`cos(a*pi/180)`); `gray(g)` sirve de default de color.
- **⚠ Footgun del lenguaje:** un identificador desnudo seguido de `(` se parsea como
  llamada a función (`dx (h+dy)` → `dx(h+dy)`). En coords, parentizar la variable:
  `(dx) (h+dy)`. Anotado en la propia lib.
- **Sin z-buffer:** orden de pintado = orden de escritura (dibujar lo lejano primero).

### Fase 2 — Portar fig2-7 sobre la biblioteca (panel b HECHO)
`examples/simulate3d/fig2-7b-v3.mg` (panel b, montaje pseudo-3D): pantalla = `plano`
blanco; láminas policristalinas = polígonos oblicuos con `hatch` inline; cristal =
`prisma`; anillo = `ellipse` directa (el motor la da; no hizo falta cizallar un
círculo); rayos = líneas; ángulos = `arc`; haz = `polyline(marker_end="Arrowr")`.
Compila EPS/SVG/PDF; θ/2θ como texto matemático. (El panel a —Bragg 2D— ya estaba en
`fig2-7v3.mg`, no usa pseudo-3D.) Pendiente opcional: `hatch` como parámetro de `plano`
para no dibujar las láminas inline.

### Fase 3 — (opcional) puntos 3D arbitrarios como GENERADOR
Solo si algún ejemplo pide trazar líneas entre vértices 3D arbitrarios que no se puedan
precomputar cómodamente. Hacerlo como **generador §13** (estilo `axis`/`grid`) que
recibe los vértices 3D **por argumento** (listas nombradas) y *emite* pares 2D → la
gramática `Subpath ::= (Coord Coord)+` queda intacta. **No** leer tripletas con `;`
dentro del bloque `{ }` (eso rompería la regla sagrada que este plan protege).

### Fase 4 — (especulativo, diferir) jaula 3D con ejes/ticks
`box_axis`/caja delimitadora con cuadrículas y marcas. Ni fig10-2 ni fig2-7 lo piden;
va al final o se descarta.

## Para retomar la próxima semana (act. 2026-07-12)

Fases 0-2 **hechas y commiteadas** (`lib/pseudo3d.mg`, `examples/simulate3d/fig10-2v3.mg`
+ `fig2-7b-v3.mg`, ambos calibrados a su `.png`). Puntos de arranque, en orden sugerido:

1. **`hatch` como parámetro de `plano`** (bajo costo): añadir `trama=0, trama_gap=3` y un
   `if trama > 0 { polygon(hatch=trama,…) } else { polygon(fill=…) }` en el cuerpo. Así
   las láminas de `fig2-7b-v3` dejan de ir inline. (Ojo: duplica la lista de vértices;
   ver si compensa.)
2. **Refactor de `fig10-2v3` para usar `plano`** (validación de la extracción sobre la
   figura real; hoy usa `shear` en bloque, que también es válido). El demo ya probó que
   `plano` reproduce esos planos; el riesgo es re-calibrar la esquina/cotas.
3. **Afinado fino de `fig2-7b`** (rendimiento decreciente): elipse levemente ladeada
   siguiendo la pantalla, cristal más laja, pantalla un pelín más vertical.
4. **Fase 3** solo si aparece un ejemplo con vértices 3D arbitrarios no precomputables.

Recordar el **footgun**: identificador desnudo + `(` = llamada a función; parentizar la
variable en coords (`(dx) (h+dy)`).

## Resumen
La intervención más económica y coherente es **no** añadir motor: la cizalla ya está,
y una biblioteca `.mg` de `plano`/`prisma` sobre las transformaciones existentes
reproduce el corpus oblicuo con impacto nulo en los backends. La isométrica verdadera,
los generadores 3D y la jaula quedan como extensiones posteriores de la misma línea.
