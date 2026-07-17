# MetaGráfica

![C++ Standard](https://img.shields.io/badge/C%2B%2B-14-blue.svg)
![License](https://img.shields.io/badge/License-GPL%203.0-green.svg)
![Version](https://img.shields.io/badge/version-3.0.0--beta-orange.svg)

**Español** · [English](README.md)

**Un lenguaje descriptivo para figuras técnicas y científicas.**

Describes *qué es la figura* —puntos, paths, structs, transformaciones— y `mg` la compila
a **EPS**, **SVG** o **PDF**. Sin interfaz gráfica ni ratón: una figura es código fuente,
así que se versiona, se compara, se parametriza y se regenera.

Con él se compusieron las figuras de Ana María Cetto y Luis de la Peña, *[Quantum
Mechanics: A Physical Approach](https://doi.org/10.1017/9781009679633)* (Cambridge
University Press, 2025) y, en versiones anteriores, las de *[Introducción a la mecánica
cuántica](https://www.fondodeculturaeconomica.com/Ficha/9786071601766/F)* de Luis de la
Peña (FCE/UNAM, 3ª ed.).

## Inicio rápido

```text
display_size 9 5.5
font_size 9
world_window -2 11 -1.5 5.5

plot(x=(0,10), y=(0,100), box=(0,0, 9,4.5), grid=true) {
    line_width 0.8
    polyline { 0 0  1 1  2 4  3 9  4 16  5 25  6 36  7 49  8 64  9 81  10 100 }

    xaxis(step=2, label="x")
    yaxis(step=25, label="$y = x^2$")
}
```

```bash
bin/mg quickstart.mg quickstart.svg
```

![y = x² graficada con MetaGráfica](docs/img/quickstart.svg)

Ese es el archivo completo ([`examples/quickstart.mg`](examples/quickstart.mg)). `plot`
mapea **unidades de datos** a una caja física en centímetros; los ejes heredan los rangos
`x=`/`y=` y se rotulan solos. El `$…$` del rótulo es notación matemática: MetaGráfica
incrusta una fuente de TeX para letras griegas, símbolos, superíndices y subíndices.

## No solo gráficas

Las gráficas de datos son un uso. El lenguaje se hizo para **ilustraciones** —diagramas
de aparatos, esquemas, lo que un artículo necesite— y ahí es donde los structs, la
colocación sobre arcos, las flechas y el texto matemático se ganan su lugar:

![Aparato de difracción de electrones](docs/img/fig2-5.svg)

> Figura 2.5 de Ana María Cetto y Luis de la Peña, *Quantum Mechanics: A Physical
> Approach*, Cambridge University Press, 2025.
> [doi:10.1017/9781009679633](https://doi.org/10.1017/9781009679633) — reproducida aquí
> como [`examples/fig2-5.mg`](examples/fig2-5.mg), el código que la compuso.

Menos de 60 líneas de MetaGráfica: el detector es un struct colocado a 37°, las flechas
del haz son otro struct barrido sobre un arco, y la `φ` va en Latin Modern Math.

## Compilación

```bash
make                 # compila bin/mg y la página de manual
sudo make install    # opcional: deja mg en el PATH
```

Necesita un compilador de C++14 (`clang++` o `g++`), `flex`, y `pandoc` para la página de
manual. [libharu](http://libharu.org/) viene incluida en `third_party/` para la salida
PDF; no hay nada más que instalar.

## Uso

El formato de salida lo elige la **extensión** del archivo de salida:

```bash
bin/mg figura.mg              # → figura.eps
bin/mg figura.mg sal.svg      # → SVG
bin/mg figura.mg sal.pdf      # → PDF
```

| opción | |
|---|---|
| `-h` | ayuda |
| `-v` | versión |

## El lenguaje en un minuto

Un **punto** es una pareja de coordenadas; un **path** es una lista de puntos. Cada
primitiva lleva su path entre `{ }` y su estilo entre `( )`:

```text
polyline { 0 0  1 2  3 1 }              % polilínea abierta
polyline(closed=true) { 0 0  1 0  1 1 } % contorno cerrado
polygon { 0 0  1 0  1 1 }               % relleno
circle(2) { 5 5  9 5 }                  % un círculo por punto
rectangle(fill="steelblue") { 0 0  4 3 }
text("masa $m_e$", align="center") { 5 1 }
```

Los **structs** son el corazón del lenguaje: un grupo con nombre que puedes colocar,
escalar, girar y repetir, en coordenadas homogéneas, sin recurrir a otro lenguaje de
programación.

```text
struct Cuadro() {
    polyline(closed=true) { -1 -1  1 -1  1 1  -1 1 }
}

for i = 0 to 11 {
    Cuadro(rotate = i*7.5, scale = 1 + i*0.35)
}
```

El lenguaje completo está en [`especificacion_mg.md`](especificacion_mg.md), y `man mg`
es la referencia. MetaGráfica **no** pretende ser un lenguaje de programación de
propósito general: tiene variables, expresiones, `for` e `if`, y ahí se detiene.

## Ejemplos

En [`examples/`](examples/) está el corpus vivo: todos sus archivos compilan con el
binario actual y se verifican en cada cambio.

```bash
bin/mg examples/fig6-4.mg sal.svg
```

| | |
|---|---|
| `quickstart.mg` | la gráfica de arriba |
| `fig2-5.mg` | la ilustración de arriba (structs, arcos, flechas) |
| `fig2-3.mg` | gráfica lineal con retícula y rótulos de eje |
| `fig6-4.mg` | eje **logarítmico**, rótulos matemáticos, anotaciones sobre los datos |
| `fig4-5.mg` | tres paneles, ejes interiores, curvas analíticas |
| `fig_polybar.mg` | histograma de barras con trama |
| `primitives.mg`, `fill_styles.mg`, `line_patterns.mg` | láminas de referencia |

## Estado del proyecto

**Esto es una beta**, y de ahí se siguen dos cosas. El **lenguaje todavía puede cambiar**:
los nombres y los argumentos no están congelados —`axis` acaba de renombrar `title` a
`label` y `labels` a `tick_labels`—, así que una figura que compila hoy puede necesitar un
ajuste mañana (los nombres viejos fallan de forma ruidosa, nunca en silencio). Y **faltan
piezas**: la especificación describe cosas que aún no existen. Lo que sí está se ejercita
con el corpus de regresión en cada cambio, y ha compuesto libros publicados.

`mg` es el compilador de la **versión 3** (`MG_VERSION 3.0.0-beta`). La gramática vieja de
dos letras (`PL`, `CR`, `GNNUM`…) quedó congelada en la rama `v1-legacy`, y su corpus vive
en [`examples/v1/`](examples/v1/) como oráculo de migración. **Esos archivos no compilan
con este binario.**

Cada cambio pasa por una red de regresión sobre todo el corpus (`bash test/run.sh check`):
comparación byte a byte contra la salida bendecida de los tres backends, una pasada de
Ghostscript sobre el EPS, y una prueba de paridad entre backends.

## Historia

MetaGráfica se ha reescrito tres veces, y cada versión compuso algo:

| | año | lenguaje | salida |
|---|---|---|---|
| **0** | 1988 | Pascal + ensamblador | manejaba directo una impresora láser HP — el primer artículo publicado |
| **1** | 1991 | C | el primer libro |
| **2** | 1999–2024 | C++ / STL | solo EPS — tres libros más |
| **3** | 2026 | C++14 | EPS, SVG, PDF |

La versión 0 se escribió cuando ninguna aplicación gráfica daba la calidad que un
documento científico necesitaba para publicarse. La versión 2 generaba únicamente
Encapsulated PostScript, con texto en Latin-1 y la fuente `symbol` para griegas y
matemáticas; PostScript se estancó y no alcanzó la revolución de Unicode, pero sigue
soportado por casi todo y se convierte a PDF sin dificultad.

Esta versión conserva el núcleo descriptivo y agrega los backends SVG y PDF, un modelo de
coordenadas isométrico, Latin Modern Math para los símbolos, y la familia `plot` para
figuras de datos. La gramática de dos letras desapareció; ver *Estado del proyecto*.

## Licencia

GPL 3.0 — Copyright © 1988–2026 Alejandro Aguilar Sierra (algsierra@gmail.com)

El rango abarca la vida de la obra — ver *Historia*. Texto completo en
[`LICENSE`](LICENSE).
