# Plan para soporte de simulación 3D por medio de transformadas afines en MG

La arquitectura de MetaGráfica V3 que has diseñado, basada en la ortogonalidad entre la forma (`primitive(args)`) y la posición (el bloque `{ x y }`), junto con la fuerte pila de transformaciones afines, ofrece una vía muy limpia para integrar esto sin tener que modificar la regla sagrada `Subpath ::= (Coord Coord)+`. Introducir coordenadas `x y z` rompería la simplicidad del parser y del motor subyacente.

Para mantener la filosofía 2D del lenguaje ("espacio isométrico por construcción") pero dar soporte nativo a la ilustración científica pseudo-3D, estas son las extensiones más naturales para la gramática V3:

### 1. Modificadores de Plano como Transformación (2.5D)

En lugar de que el usuario calcule a mano los valores de `shear` y `rotate`, puedes añadir proyecciones de plano directamente a `TransformCall` y `AttrName` en la EBNF.

El motor de MG interpretará estas instrucciones, calculará la matriz afín exacta (rotación + cizallamiento + escala) y la empujará a la pila de transformaciones. El usuario sigue dibujando formas 2D puras, pero el bloque de estado las "aplasta" hacia la perspectiva correcta.

**Propuesta para la EBNF (§2):**
Añadir a `TransformCall` y `AttrName`:

* `isometric(plane)` donde `plane` es `"top"`, `"left"`, `"right"`.
* `oblique(plane, angle, factor)` donde `plane` es `"front"`, `"top"`, `"side"`.

**Ejemplo de uso en V3:**

```text
% Dibuja un volumen proyectado armando sus tres caras planas
compound("cristal_difractor") {
    isometric("top") {
        % Se dibuja en 2D normal, la matriz lo proyecta al techo
        rectangle(fill="#E0E0E0", color="black") { 0 0  5 5 }
        circle(0.2, fill="black") { 2.5 2.5 } % El círculo se vuelve elipse automáticamente
    }
    isometric("left") {
        rectangle(fill="#A0A0A0", color="black") { 0 0  5 3 }
    }
    isometric("right") {
        rectangle(fill="#606060", color="black") { 0 0  5 3 }
    }
}

```

**Por qué funciona en tu motor:** Como indicas en §4.4, el `rectangle` y otras primitivas ya son formas transformables que operan transformando sus vértices. Al aplicar una matriz isométrica u oblicua, el backend (SVG, EPS o PDF) hará el renderizado perfecto sin que el motor central necesite saber nada de matemáticas 3D.

### 2. Generador de Coordenadas 3D a 2D (`project`)

Para casos donde realmente se necesita trazar líneas entre puntos arbitrarios en el espacio (como en diagramas de rayos X o visualización de órbitas), puedes introducir una función de nivel `Expression` o una primitiva generadora que acepte tríos pero emita la secuencia de pares que el bloque `{ }` ya espera.

**Propuesta:** Una primitiva de trayectoria (similar a `bezier` o `spline` en §2) llamada `path3d`.

```text
% Usando projection oblicua por defecto
path3d(proj="oblique", angle=45, factor=0.5, color="red", line_width=1.5) {
    % x y z
    0 0 0 ; 
    5 5 5 ;
    10 0 0
}

```

Internamente, en la fase del compilador, `path3d` lee grupos de tres, aplica la fórmula de proyección y los convierte a una `polyline` estándar de pares de coordenadas 2D. Así, proteges el backend de cualquier cambio estructural.

### 3. Generadores Espaciales (Expansión de §2 `Generator`)

En gráficos científicos y de datos espaciales, dibujar la "jaula" delimitadora (bounding box) es fundamental y tedioso. Puedes extender los generadores de V3 (`axis`, `grid`) para incluir contenedores de volumen.

**Propuesta:** Añadir `box_axis`.

```text
box_axis(proj="isometric", x_len=10, y_len=10, z_len=5, ticks=true) { 0 0 }

```

Esto invocaría una subrutina en el motor que automáticamente genere los tres planos isométricos con sus cuadrículas y marcas (`ticks`), dejando todo listo para que el usuario emita sus marcadores de tamaño físico (`marker`, §4.6) u objetos dentro del volumen.

### 4. Primitivas de Volumen Simulado vía `struct`

La gran ventaja de haber implementado `struct` (§8) con parámetros nativos es que ni siquiera tienes que programar formas tridimensionales básicas en C/Python.

Puedes crear una librería estándar (por ejemplo, `include "3d_utils.mg"`) que envuelva las transformaciones propuestas en el paso 1 para ofrecer prismas listos para usar:

```text
struct Prisma(w, h, d, color_top="white", color_left="gray", color_right="black") {
    isometric("top")   { rectangle(fill=color_top) { ... } }
    isometric("left")  { rectangle(fill=color_left) { ... } }
    isometric("right") { rectangle(fill=color_right) { ... } }
}

% En el script del usuario:
Prisma(5, 3, 2) { 10 10 }

```

El enfoque de añadir las matrices de proyección estandarizadas (`isometric` y `oblique`) a la pila de estado actual es la intervención más económica, elegante y coherente con el manifiesto de la Especificación V3. Te daría resultados matemáticamente perfectos para ilustraciones de artículos académicos con un impacto nulo en la complejidad de renderizado del backend.
