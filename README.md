# metagrafica

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

License: GPL 3.0

Copyright (c) 2024 Alejandro Aguilar Sierra (algsierra@gmail.com)
