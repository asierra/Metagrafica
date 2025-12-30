# MetaGráfica

![C++ Standard](https://img.shields.io/badge/C%2B%2B-14-blue.svg)
![License](https://img.shields.io/badge/License-GPL%203.0-green.svg)

[English](#english) | [Español](#español)

## Español

MetaGráfica es un lenguaje gráfico vectorial 2D de construcción
descriptiva. La versión original de finales de los 1980s controlaba
directamente una impresora láser HP en una época en que no había
buenas aplicaciones gráficas con salidas de la calidad requerida por
un documento científico para publicación. En la versión de 2002 se
migró al lenguaje `C++`, se aprovechó la biblioteca estándar de
estructuras de datos STL y la orientación a objetos. Con esa versión
la salida era exclusivamente Encapsulated PostScript y el texto usaba
el código Latin1 o ISO 8859-1 que incluía todos los caracteres
especiales usados en español y los símbolos matemáticos y griegos se
conseguían con la fuente estándar `symbol`.  Esa versión fue usada
para los libros de texto de Mecánica Cuántica de Luis de la
Peña. PostScript se estancó en esos años, no alcanzó la revolución de
Unicode, pero sigue siendo soportado por la mayoría de las
aplicaciones gráficas y es trivial convertirlo al moderno PDF. Esta
versión genera únicamente EPS y de ser necesario, integra una fuente
de TeX para los símbolos griegos, matemáticos y físicos.

El tipo de datos elemental es el punto, definido por una pareja de
coordenadas (x, y) en un espacio cartesiano bidimensional. La
estructura de datos básica es el camino ("path"), formado por una
serie de puntos, con lo que se puede definir cualquier objeto gráfico
como curvas, polígonos, etc. Las características específicas como
grosor de línea o color se definen mediante atributos. Maneja una
versión limitada de texto matemático estilo LaTeX, que permite
escribir símbolos griegos, super y subscripts y otros símbolos
físicos. Su fortaleza es la definición de *estructuras*, que permiten
combinar distintos primitivos gráficos con transformaciones lineales
en coordenadas homogéneas, para generar gráficos más complejos, sin
necesidad de usar otro lenguaje de programación. Es importante aclarar
que Metagrafica no pretende ser un lenguaje de programación, es muy
específico y limitado a la generación de figuras técnicas de alta
calidad.

Esta versión deberá ser compilada con el estándar 2014 de C++,
con las recomendaciones del [C++ Ortodoxo]
(https://gist.github.com/bkaradzic/2e39896bc7d8c34e042b).

### Uso

```bash
make
./mg examples/primitives.mg
```

Ver ejemplo abajo.
---

## English

MetaGráfica is a 2D descriptive vector graphics language. The original version from late 1980s directly controlled an HP laser printer at a time when there were no good graphics applications with output of the quality required for a scientific publication. In the 2002 version, it was migrated to the C++ language, taking advantage of object oriented programming and the STL. With that version the output was exclusively Encapsulated PostScript and for text we used the Latin1 or ISO 8859-1 coding that included all the special characters used in Spanish and Western Europe languages; the mathematical and Greek symbols were achieved using the standard symbol font. That version was used for Luis de la Peña's Quantum Mechanics textbooks. PostScript stagnated in those years, falling short of the Unicode revolution, but it is still supported by most graphical applications and is trivial to convert to modern PDF. This version still generates only EPS and, if necessary, embedes a TeX font for Greek, mathematical and physical symbols.

The elementary data type is the point, defined by a pair of coordinates (x, y) in a two-dimensional Cartesian space. The basic data structure is the path, formed by a series of points, with which any graphic object can be defined such as curves, polygons, etc. Specific characteristics such as line width or color are defined by attributes. It handles a limited version of LaTeX-style mathematical text, which allows you to write Greek symbols, super and subscripts and other physical symbols. Its strength is the definition of structures, which allow combining different graphic primitives with linear transformations in homogeneous coordinates, to generate more complex graphics, without the need to use another programming language. It is important to clarify that Metagrafica is not intended to be a programming language, it is very specific and limited to the generation of high quality technical figures.

This version must be compiled with the C++ 2014 standard, following the recommendations of [Orthodox C++] (https://gist.github.com/bkaradzic/2e39896bc7d8c34e042b).

### Usage

```bash
make
./mg examples/primitives.mg
```

+### Quick Reference
+
+**Graphics Primitives**
+
+*   `PL x1 y1 ... }`: Polyline. Join all points of the path with straight lines.
+*   `CR r [dq [q0]] : x1 y1 ... }`: Circles or arcs of radius `r`.
+*   `BR x1 y1 x2 y2 }`: Rectangle defined by opposite corners.
+*   `PG x1 y1 ... }`: Filled polygon.
+*   `DOT r x1 y1 ... }`: Filled circles of radius `r`.
+*   `BZ x1 y1 ... }`: Bezier curve.
+*   `DT text`: Draw text at current position.
+*   `XYDT x y text`: Draw text at position `x y`.
+
+### Examples
+
+A simple MG file with a corner, a circle and a message:
+
+```text
+$D 12 8
+WW 0 24 0 16
+
+PL 12 15  12 8  20 8 }
+CR 6 : 12 8 }
+
+XYDT 8 10 Hello World!
+```
+
## License

GPL 3.0

## License

GPL 3.0
Copyright (c) 2024 Alejandro Aguilar Sierra (algsierra@gmail.com)
