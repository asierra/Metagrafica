#!/usr/bin/env python3
"""Genera la galería: cada ejemplo del corpus junto a su render y su fuente.

    python3 tools/galeria.py            # escribe docs/galeria.html y docs/gallery.html
    python3 tools/galeria.py --check    # falla si alguna de las dos está rancia

Auxiliar FUERA del compilador, como hist2mg.py: no se liga a bin/mg y el lenguaje
no depende de ella.

DOS PÁGINAS, una por idioma (2026-07-27): `docs/galeria.html` (es) y
`docs/gallery.html` (en), enlazadas entre sí. La galería es la puerta de entrada
del proyecto —la sirve GitHub Pages y es lo que ve quien llega de fuera—, así que
tiene la misma cortesía que el README, que ya venía en dos idiomas.

De dónde sale el texto de cada tarjeta. En español, del encabezado del propio .mg,
que sigue esta convención:

    % Título de una línea.            ← título de la tarjeta
    %
    % Dos a cinco líneas de qué es    ← descripción
    % y qué enseña del lenguaje.
    %
    % NOTAS ------------------------  ← para quien mantiene; NO se publica
    % ...

Así no hay lista de descripciones que mantener aparte: el archivo se describe a sí
mismo. Lo que sigue a "% NOTAS" nunca sale de aquí.

⚠ En inglés NO hay de dónde sacarlo: los .mg están comentados en español (política
del proyecto) y meterles un segundo encabezado los volvería ilegibles. Por eso las
traducciones viven en la tabla TRAD de este archivo. Un ejemplo sin traducir
**igual aparece** en la página inglesa, con su texto en español y un aviso por
stderr al generar: la regla del proyecto es que un ejemplo nuevo salga solo, y una
tarjeta a medio traducir es mejor que una figura ausente.

⚠ QUÉ ENTRA, y por qué NO es la misma regla de la compuerta `imgfail`. Aquí entra
un examples/X.mg que tenga docs/img/X.svg. La compuerta itera al revés —sobre lo
que hay en docs/img— y por eso vigila además las variantes que existen solo para
los ensayos (franck_condon_anarm, turning_points_nodos, parabola_vs_arco), que no
son ejemplos del corpus y no van a la galería. Son dos reglas parecidas y
distintas; no las unifiques.

Un ejemplo sin render no entra, y eso es deliberado: curvas3.mg es una biblioteca
de datos que compila a una página en blanco.
"""

import html
import pathlib
import sys

# Orden de presentación, y es una decisión editorial: ABRE con las figuras que
# mejor explican de qué se trata esto —una órbita que se oculta sola detrás del
# globo, un árbol de 511 segmentos escrito en cuatro líneas— y deja los catálogos
# al final. Antes abría con `quickstart` y las dos figuras más vistosas caían en
# "Más ejemplos", al pie de la página.
#
# Lo que no esté aquí cae al final, en "Más ejemplos": un ejemplo nuevo aparece
# solo, sin tocar esta lista.
GRUPOS = [
    (("Figuras que se calculan solas",
      "Figures that compute themselves"),
     ("Nada está medido a ojo: se dan los parámetros físicos y la geometría se "
      "deduce. Cambia un número y la figura entera se reacomoda.",
      "Nothing is measured by eye: you give the physical parameters and the "
      "geometry follows. Change one number and the whole figure rearranges itself."),
     ["orbita_polar", "gravitacion_orbita", "elevacion_solar", "fractal_tree",
      "franck_condon", "turning_points"]),
    (("Escenas pseudo-3D", "Pseudo-3D scenes"),
     ("Se declara una cámara y la figura se describe en coordenadas del espacio: "
      "cada círculo del dibujo es un círculo de la escena, y la elipse exacta de "
      "su proyección la calcula el compilador. Cambia la cámara y todo se mueve junto.",
      "You declare a camera and describe the figure in coordinates of space: every "
      "circle in the drawing is a circle of the scene, and the exact ellipse of its "
      "projection is worked out by the compiler. Change the camera and it all moves "
      "together."),
     ["angulo_solido", "onda_electromagnetica"]),
    (("Gráficas de datos", "Data plots"),
     ("El plot mapea unidades de datos a una caja en centímetros; los ejes "
      "heredan los rangos y se rotulan solos.",
      "A plot maps data units to a box in centimetres; the axes inherit the "
      "ranges and label themselves."),
     ["quickstart", "fig6-4", "fig4-4", "fig_polybar", "fig1", "tiro_parabolico"]),
    (("Ilustraciones y diagramas", "Illustrations and diagrams"),
     ("Diagramas de aparato y esquemas, donde mandan las estructuras, la "
      "colocación y los marcadores que se orientan solos.",
      "Apparatus diagrams and schematics, where what matters is structures, "
      "placement and markers that orient themselves."),
     ["espectro", "fig2-5", "fig2-1", "fig4-1", "rpstest"]),
    (("Láminas de referencia", "Reference sheets"),
     ("Catálogos: cada forma, cada relleno y cada símbolo en su expresión más "
      "simple, para copiar y pegar.",
      "Catalogues: every shape, fill and symbol in its simplest form, to copy "
      "and paste."),
     ["primitives", "fill_styles", "line_patterns", "markers-demo", "symbols",
      "texto", "path_sample", "sines"]),
]

# Título y descripción en inglés. El español sale del encabezado del .mg; esto es
# lo único que hay que escribir a mano al añadir un ejemplo (y si no se escribe,
# la tarjeta sale en español con un aviso).
TRAD = {
    "angulo_solido": (
        "Projection of the solid angle — a wire sphere and the cap A = πr²",
        "A wire sphere in orthographic axonometry, with the circular cap that subtends a "
        "solid angle from its centre. The whole drawing is made of CIRCLES OF SPACE "
        "—every meridian, every parallel and the rim of the cap— and not one "
        "semi-axis is computed here: `view3d` sets the camera, `plane3d` puts the "
        "coordinates on a plane of the scene, and a `circle` drawn there comes out as the "
        "exact ellipse of its projection. What belongs to no plane —the three dotted "
        "dimension lines— uses `xyz()`. Change `azd` or `eld` and the whole figure "
        "moves together."),
    "onda_electromagnetica": (
        "Electromagnetic wave — E and B in perpendicular planes",
        "Two waves in phase, each in its own plane: the electric field in the vertical one, "
        "the magnetic in the horizontal, propagating along x. Each wave is an ordinary `sine` "
        "INSIDE a `plane3d`, filled half-cycle by half-cycle, and its combs and its λ dimension "
        "are plain 2-D polylines of the same plane: nothing is sampled or projected by hand, "
        "and the arrowheads orient themselves. The only thing belonging to no plane is the "
        "axis, which uses `xyz()`. The two amplitudes differ on purpose, and the factor comes "
        "from the camera: the horizontal plane is foreshortened, so drawing both with the same "
        "amplitude would suggest one field is smaller than the other."),
    "espectro": (
        "The electromagnetic spectrum, with the infrared window expanded",
        "An outreach figure made of rectangles, text and arrows. What sets it apart is "
        "the GRADIENT fill: infrared runs orange to red to black, microwaves from light "
        "to dark grey, and the visible band is a continuous rainbow from magenta to red "
        "\u2014 a hue sweep, written as the six stops that are the corners of the RGB cube. "
        "It shows `gradient=` with two, three and six stops, combined with `color=` to "
        "outline, and arrowheads as an attribute of the line "
        "(`marker_start`/`marker_end`), which orient themselves."),
    "elevacion_solar": (
        "Solar elevation angles \u2014 the geometry of illumination in remote sensing",
        "A section through the observer's meridian at solar noon: the globe, the tangent "
        "plane (the local horizon), the local vertical with the satellite at the zenith, "
        "and the three rays of the June solstice, the equinoxes and the December "
        "solstice. The three elevation angles are not placed by eye: they follow from the "
        "latitude and the obliquity of the ecliptic \u2014the two numbers declared at the "
        "top\u2014 and the labels print the very value that governed the drawing. Change "
        "`lat` and the whole figure recomputes. The globe is a real vector map included "
        "from `lib/`, in an equatorial orthographic view: its limb is a meridian seen "
        "edge-on, so a point on the limb at angle \u03c6 lies at exactly latitude "
        "\u03c6, and the observation point sits on it with no correction at all."),
    "orbita_polar": (
        "Polar orbits — two satellites over a globe in orthographic projection",
        "Two polar orbits inclined ±15° around the Earth, with a satellite on each and "
        "arrows for the direction of travel. Each orbit is drawn as an ELLIPTICAL ARC "
        "whose sweep skips the stretch that runs behind the globe: the occlusion comes "
        "from a closed formula evaluated in the `.mg` itself, with nothing trimmed by "
        "hand. The satellites sit on the orbit by their parametric angle, already "
        "oriented to the tangent, and the globe is a real vector map included from `lib/`."),
    "gravitacion_orbita": (
        "Circular orbit — gravitation and centripetal force",
        "A satellite in circular orbit around the Earth —seen from the north pole, over a "
        "REAL vector map— with the gravitational force curving it towards the centre (red) "
        "and its tangential velocity (green), plus the two formulas typeset "
        "mathematically. It introduces `\\frac`, TWO libraries via `include` "
        "(`lib/satellite.mg` and `lib/polar_map.mg`, an iconic map generated from Natural "
        "Earth data with `tools/geo2mg.py`) placed with `scale`/`rotate`/`at`, and "
        "arrowheads that INHERIT the colour of their line."),
    "fractal_tree": (
        "Fractal trees — a structure that contains itself",
        "Both trees are the same four-line structure —a trunk with two smaller copies of "
        "itself at the tip— invoked with different branch angles; each one is 511 "
        "segments. The stopping condition is an ordinary `if`, and `max_depth` is the "
        "safety net."),
    "franck_condon": (
        "The Franck-Condon principle — two Morse potentials, PARAMETRIC",
        "Nothing is measured: you give the Morse parameters (a, re, we, xe) of each "
        "electronic state plus Te, and from those the curve, the vibrational levels, the "
        "turning points and the wavefunctions all follow in closed form. Change one "
        "number and the whole figure rearranges itself coherently."),
    "turning_points": (
        "How ψ behaves with energy — the classical turning points",
        "Oscillatory where E > V, exponential where E < V, and the turning points as the "
        "boundary between them. A PARAMETRIC figure: you give the asymptotes of the "
        "potential, its minimum, the three turning points and the three energies, and "
        "from those follow the V(x) curve in closed form and, through WKB, the "
        "wavelengths, the amplitudes and the tails."),
    "quickstart": (
        "The README example — y = x², with axes, grid and legend",
        "The shortest figure that shows the whole plotting language: `plot` maps data "
        "units to a box in centimetres, and the axes inherit the ranges and label "
        "themselves."),
    "fig6-4": (
        "Alpha-decay half-lives — the Geiger-Nuttall law",
        "LOGARITHMIC y axis: the data are given in real units and the plot maps them to "
        "its box; the axes inherit the ranges and label themselves, without a single "
        "power-of-ten label written by hand. The isotopes are text anchored to their data."),
    "fig4-4": (
        "Three potential wells and the structure of their spectra",
        "Each panel is a plot in physical units, with its true origin. The energy levels "
        "and the turning points are DERIVED from E: move one energy and its line, its "
        "ticks and its labels rearrange themselves. Several plots in one document, and "
        "axes with `base=` (the V axis centred in a, the x axis on V=0 in c)."),
    "fig_polybar": (
        "CO₂ absorption spectra — histograms with `polybar`",
        "Two CO₂ path lengths superposed; the hatched area between them is the difference "
        "between the two spectra. Exercises polybar and hatched fill with no outline."),
    "fig1": (
        "Water content of the atmosphere as a function of altitude",
        "The solid line is the computed values; the circles and the crosses are two sets "
        "of observed values. Markers over data, with axis labels in mathematical notation."),
    "tiro_parabolico": (
        "Projectile motion — a trajectory sampled point by point",
        "Three things at once: `path +=` builds the curve inside a `for`, and the same "
        "coordinates feed the projections onto each axis; the grid is neither regular nor "
        "logarithmic, but falls where the physics puts the points; and the cannon is a "
        "struct with its MUZZLE at its local origin, placed with the same variable that "
        "fixes the start of the trajectory, so moving y0 moves cannon and curve TOGETHER."),
    "fig2-5": (
        "Electron diffraction — the Davisson-Germer experiment",
        "The illustration example from the README. The detector is a struct placed at 37°; "
        "the beam, the dimension line and the detector's rotation are markers that ORIENT "
        "THEMSELVES to the tangent of their line or arc and anchor to the vertex, without "
        "a single angle worked out by hand."),
    "fig2-1": (
        "Black-body model — the cavity and its aperture",
        "The box and the arc of the aperture join into ONE closed path with `compound`: on "
        "closing, the ends meet by themselves, with no line connecting them. Hatched fill "
        "with an outline."),
    "fig4-1": (
        "Electron diffraction — four panels, single and double slit",
        "The intensity curves live in curvas3.mg as natural data (position against "
        "intensity); here they are TRANSPOSED so that intensity runs horizontally, and are "
        "placed with `fit` into their rectangle, rotating nothing. The common base "
        "—screens and beam— is factored out into a struct."),
    "rpstest": (
        "Repeating structures — the ways to place a struct many times",
        "A box repeated with `repeat`: advancing in x, along a diagonal and deformed, and "
        "turning about a single point, where `transform=` accumulates the rotation in each "
        "copy. The last one is not a repeat but a single placement with at= and scale=."),
    "primitives": (
        "Basic shapes — reference sheet of the geometric primitives",
        "A catalogue, not a figure: each primitive in its simplest form, and then the same "
        "ones filled and hatched. The shapes are written FLAT, not wrapped in structs, so "
        "that whoever comes looking for «how do I draw an ellipse» finds the line that "
        "draws it. Sister sheet to fill_styles.mg and line_patterns.mg."),
    "fill_styles": (
        "Area fills — reference sheet of hatches, greys and outlines",
        "The catalogue of hatch patterns (hatch, hatchback, crosshatch, and the crosshatch "
        "straightened into a square grid with hatch_angle=0), the greyscale, and fill "
        "combined with an outline. Sister sheet to primitives.mg and line_patterns.mg."),
    "line_patterns": (
        "Line patterns and widths — reference sheet",
        "The six `dash` styles on the left and a `line_width` scale in typographic points "
        "on the right, generated with a `for` over a list. Sister sheet to primitives.mg "
        "and fill_styles.mg."),
    "markers-demo": (
        "Markers — the seven catalogue shapes, and how they orient",
        "The shapes, tangent orientation along a curve (`arrow` orients itself) and "
        "independent colour between marker and curve. The size is PHYSICAL, in points: "
        "immune to the window, because only the position is transformed."),
    "symbols": (
        "Catalogue of the mathematical symbols written as \\command",
        "All 110 names, typeset in Latin Modern Math: 69 of operators, relations, arrows "
        "and delimiters, and 41 of Greek letters —lowercase, variants, uppercase and the ħ "
        "of `\\hbar`."),
    "texto": (
        "Text — alignment, markup, mathematics and fine typography",
        "Labels anchored to guide lines to show off align and valign; bold and emphasis; "
        "subscripts, superscripts and Greek letters; typographic quotes, dashes, bullets "
        "and symbols outside Latin-1; and multi-line text with /n."),
    "path_sample": (
        "Sampling a path: sample, point_at and angle_at",
        "This family READS the geometry of a curve at a parameter t between 0 and 1 "
        "travelled by ARC LENGTH, so t=0.5 is the GEOMETRIC midpoint and not half the "
        "segments. The `curve` flag sets how the path is interpreted, which is neutral: in "
        "green the points as vertices (touching the hull), in red as Bézier controls "
        "(touching the curve)."),
    "sines": (
        "Waves — the `sine` primitive",
        "Each `sine` takes a two-point baseline, a number of half cycles and an amplitude, "
        "and generates the Bézier curves internally: the wave oscillates perpendicular to "
        "its baseline, whatever its orientation. Phase 0 and the square variant are shown."),
}

REPO = "https://github.com/asierra/Metagrafica/blob/main/examples/"

# Cadenas de la página, por idioma. ES primero, EN después.
T = {
    "es": {
        "lang": "es",
        "archivo": "galeria.html",
        "otro_archivo": "gallery.html",
        "otro_idioma": "English",
        "title": "MetaGráfica — galería de ejemplos",
        "h1": "MetaGráfica — galería",
        "intro": "Un lenguaje descriptivo para figuras técnicas y científicas. "
                 "Cada figura de esta página es un archivo de texto: despliega el "
                 "código bajo cualquiera de ellas y lo verás entero, tal como se "
                 "compiló.",
        "intro2": "El mismo archivo produce EPS, SVG o PDF según la extensión que "
                  "le pidas. No hay ratón: una figura es código fuente, así que se "
                  "versiona, se compara y se regenera.",
        "probar_h": "Pruébalo",
        "probar_p": "Necesitas un compilador de C++ y <code>flex</code>. Compila en "
                    "segundos y no arrastra LaTeX.",
        "probar_code": "git clone https://github.com/asierra/Metagrafica\n"
                       "cd Metagrafica && make\n"
                       "bin/mg examples/orbita_polar.mg figura.svg",
        "beta": "Versión 3.0.0-beta: la gramática todavía puede cambiar, y cambia "
                "cuando una figura nueva lo pide. Si escribes figuras con ella, lo "
                "que te resulte incómodo puede acabar en el lenguaje.",
        "nav_repo": "Repositorio",
        "nav_ref": "Referencia del lenguaje",
        "nav_build": "Cómo compilarlo",
        "mas": "Más ejemplos",
        "ver_github": "ver en GitHub",
        "ver_codigo": "Ver el código que la dibuja",
        "pie": "Página generada por <code>tools/galeria.py</code> a partir de "
               "<code>examples/</code> y <code>docs/img/</code>. El título y la "
               "descripción de cada tarjeta salen del encabezado de su propio "
               "<code>.mg</code>.",
    },
    "en": {
        "lang": "en",
        "archivo": "gallery.html",
        "otro_archivo": "galeria.html",
        "otro_idioma": "Español",
        "title": "MetaGráfica — example gallery",
        "h1": "MetaGráfica — gallery",
        "intro": "A descriptive language for technical and scientific figures. "
                 "Every figure on this page is a text file: unfold the source "
                 "under any of them and you get the whole thing, exactly as it was "
                 "compiled.",
        "intro2": "The same file produces EPS, SVG or PDF depending on the "
                  "extension you ask for. There is no mouse: a figure is source "
                  "code, so it can be versioned, diffed and regenerated.",
        "probar_h": "Try it",
        "probar_p": "You need a C++ compiler and <code>flex</code>. It builds in "
                    "seconds and drags no LaTeX along.",
        "probar_code": "git clone https://github.com/asierra/Metagrafica\n"
                       "cd Metagrafica && make\n"
                       "bin/mg examples/orbita_polar.mg figure.svg",
        "beta": "Version 3.0.0-beta: the grammar can still change, and it does "
                "change when a new figure asks for it. If you write figures with "
                "it, whatever you find awkward may well end up in the language.",
        "nav_repo": "Repository",
        "nav_ref": "Language reference",
        "nav_build": "How to build it",
        "mas": "More examples",
        "ver_github": "view on GitHub",
        "ver_codigo": "See the source that draws it",
        "pie": "Page generated by <code>tools/galeria.py</code> from "
               "<code>examples/</code> and <code>docs/img/</code>. The sources are "
               "commented in Spanish, which is the project's working language.",
    },
}

CSS = """
:root {
  --fondo: #ffffff; --texto: #1a1a1a; --tenue: #5b6472; --borde: #e2e5ea;
  --tarjeta: #ffffff; --acento: #2b5c8a; --codigo: #f6f7f9;
}
@media (prefers-color-scheme: dark) {
  :root {
    --fondo: #14171c; --texto: #e6e8ec; --tenue: #99a2b0; --borde: #2b3038;
    --tarjeta: #1b1f26; --acento: #7fb2e0; --codigo: #101318;
  }
}
:root[data-theme="dark"] {
  --fondo: #14171c; --texto: #e6e8ec; --tenue: #99a2b0; --borde: #2b3038;
  --tarjeta: #1b1f26; --acento: #7fb2e0; --codigo: #101318;
}
:root[data-theme="light"] {
  --fondo: #ffffff; --texto: #1a1a1a; --tenue: #5b6472; --borde: #e2e5ea;
  --tarjeta: #ffffff; --acento: #2b5c8a; --codigo: #f6f7f9;
}
* { box-sizing: border-box; }
body {
  margin: 0; padding: 2.5rem 1.25rem 4rem;
  background: var(--fondo); color: var(--texto);
  font: 16px/1.6 -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
}
.envoltura { max-width: 1180px; margin: 0 auto; }
header { border-bottom: 1px solid var(--borde); padding-bottom: 1.75rem; margin-bottom: 2.5rem; }
.barra { display: flex; justify-content: flex-end; font-size: .85rem; margin-bottom: .5rem; }
h1 { font-size: 1.9rem; margin: 0 0 .4rem; letter-spacing: -.02em; }
header p { margin: .4rem 0; color: var(--tenue); max-width: 62ch; }
a { color: var(--acento); }
h2 { font-size: 1.25rem; margin: 3rem 0 .3rem; letter-spacing: -.01em; }
h2:first-of-type { margin-top: 0; }
.grupo-desc { color: var(--tenue); margin: 0 0 1.5rem; max-width: 62ch; font-size: .95rem; }
.probar {
  margin: 1.5rem 0 0; padding: 1rem 1.15rem; background: var(--codigo);
  border: 1px solid var(--borde); border-radius: 8px; max-width: 62ch;
}
.probar h2 { font-size: 1rem; margin: 0 0 .35rem; }
.probar p { margin: 0 0 .6rem; }
.probar pre {
  margin: 0; padding: .7rem .8rem; background: var(--tarjeta);
  border: 1px solid var(--borde); border-radius: 6px;
  overflow-x: auto; font-size: .82rem; line-height: 1.5;
}
.beta { margin-top: 1rem; font-size: .88rem; }
.rejilla { display: grid; gap: 1.5rem; grid-template-columns: repeat(auto-fill, minmax(330px, 1fr)); }
.tarjeta {
  border: 1px solid var(--borde); border-radius: 10px; overflow: hidden;
  background: var(--tarjeta); display: flex; flex-direction: column;
}
.lienzo {
  background: #fff; border-bottom: 1px solid var(--borde);
  padding: 1rem; display: flex; align-items: center; justify-content: center;
  min-height: 210px;
}
.lienzo img { max-width: 100%; height: auto; max-height: 380px; }
.cuerpo { padding: 1rem 1.15rem 1.15rem; flex: 1; display: flex; flex-direction: column; }
.cuerpo h3 { font-size: 1.02rem; margin: 0 0 .45rem; }
.cuerpo p { margin: 0 0 .9rem; color: var(--tenue); font-size: .9rem; flex: 1; }
.pie { display: flex; align-items: center; gap: .9rem; font-size: .85rem; }
.pie code { background: var(--codigo); padding: .15rem .4rem; border-radius: 4px; font-size: .85em; }
details { margin-top: .9rem; }
details summary { cursor: pointer; font-size: .85rem; color: var(--acento); }
details pre {
  margin: .7rem 0 0; padding: .85rem; background: var(--codigo);
  border: 1px solid var(--borde); border-radius: 6px;
  overflow-x: auto; font-size: .78rem; line-height: 1.45; max-height: 460px;
}
footer { margin-top: 4rem; padding-top: 1.5rem; border-top: 1px solid var(--borde);
         color: var(--tenue); font-size: .9rem; }
"""


def encabezado(texto):
    """Devuelve (título, descripción) del bloque de comentario inicial."""
    lineas = []
    for linea in texto.split("\n"):
        if not linea.startswith("%"):
            break
        lineas.append(linea[1:].strip())
    parrafos, actual = [], []
    for linea in lineas:
        if linea.startswith("NOTAS"):
            break
        if linea:
            actual.append(linea)
        elif actual:
            parrafos.append(" ".join(actual))
            actual = []
    if actual:
        parrafos.append(" ".join(actual))
    titulo = parrafos[0] if parrafos else ""
    desc = parrafos[1] if len(parrafos) > 1 else ""
    return titulo, desc


def marcado(texto):
    """Escapa a HTML y convierte `código` en <code>, que es el único marcado que
    usan los encabezados."""
    partes = html.escape(texto).split("`")
    return "".join(p if i % 2 == 0 else "<code>%s</code>" % p
                   for i, p in enumerate(partes))


def tarjeta(raiz, nombre, t, avisos):
    fuente = (raiz / "examples" / (nombre + ".mg")).read_text(encoding="utf-8")
    titulo, desc = encabezado(fuente)
    if t["lang"] == "en":
        if nombre in TRAD:
            titulo, desc = TRAD[nombre]
        else:
            avisos.append(nombre)
    e = html.escape
    return f"""      <article class="tarjeta">
        <div class="lienzo"><img src="img/{nombre}.svg" alt="{e(titulo.replace('`', ''))}" loading="lazy"></div>
        <div class="cuerpo">
          <h3>{marcado(titulo)}</h3>
          <p>{marcado(desc)}</p>
          <div class="pie"><code>{nombre}.mg</code>
            <a href="{REPO}{nombre}.mg">{e(t["ver_github"])}</a></div>
          <details><summary>{e(t["ver_codigo"])}</summary>
            <pre>{e(fuente.rstrip())}</pre></details>
        </div>
      </article>
"""


def construir(raiz, t, avisos):
    i = 0 if t["lang"] == "es" else 1
    disponibles = sorted(
        p.stem for p in (raiz / "examples").glob("*.mg")
        if (raiz / "docs" / "img" / (p.stem + ".svg")).exists()
    )
    partes, usados = [], set()
    for nombres, descs, ejemplos in GRUPOS:
        presentes = [x for x in ejemplos if x in disponibles]
        if not presentes:
            continue
        usados.update(presentes)
        partes.append(f"    <h2>{html.escape(nombres[i])}</h2>\n"
                      f"    <p class=\"grupo-desc\">{html.escape(descs[i])}</p>\n"
                      "    <div class=\"rejilla\">\n")
        partes += [tarjeta(raiz, x, t, avisos) for x in presentes]
        partes.append("    </div>\n")
    sobrantes = [x for x in disponibles if x not in usados]
    if sobrantes:
        partes.append(f"    <h2>{html.escape(t['mas'])}</h2>\n"
                      "    <div class=\"rejilla\">\n")
        partes += [tarjeta(raiz, x, t, avisos) for x in sobrantes]
        partes.append("    </div>\n")

    return f"""<!doctype html>
<html lang="{t["lang"]}">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>{html.escape(t["title"])}</title>
<style>{CSS}</style>
</head>
<body>
<div class="envoltura">
<header>
  <div class="barra"><a href="{t["otro_archivo"]}">{t["otro_idioma"]}</a></div>
  <h1>{html.escape(t["h1"])}</h1>
  <p>{t["intro"]}</p>
  <p>{t["intro2"]}</p>
  <p><a href="https://github.com/asierra/Metagrafica">{html.escape(t["nav_repo"])}</a> ·
     <a href="https://github.com/asierra/Metagrafica/blob/main/docs/referencia.md">{html.escape(t["nav_ref"])}</a> ·
     <a href="https://github.com/asierra/Metagrafica#building">{html.escape(t["nav_build"])}</a></p>
  <div class="probar">
    <h2>{html.escape(t["probar_h"])}</h2>
    <p>{t["probar_p"]}</p>
    <pre>{html.escape(t["probar_code"])}</pre>
  </div>
  <p class="beta">{html.escape(t["beta"])}</p>
</header>
{''.join(partes)}<footer>
  {t["pie"]}
</footer>
</div>
</body>
</html>
"""


def main():
    raiz = pathlib.Path(__file__).resolve().parent.parent
    check = "--check" in sys.argv
    avisos, rancias = [], []
    for clave, t in T.items():
        destino = raiz / "docs" / t["archivo"]
        pagina = construir(raiz, t, avisos)
        if check:
            actual = destino.read_text(encoding="utf-8") if destino.exists() else ""
            if actual != pagina:
                rancias.append(t["archivo"])
        else:
            destino.write_text(pagina, encoding="utf-8")
            print("escrito %s" % destino)
    if avisos and not check:
        print("aviso: sin traducción al inglés (salen en español): %s"
              % ", ".join(sorted(set(avisos))), file=sys.stderr)
    if check:
        if rancias:
            print("%s RANCIA(S): corre python3 tools/galeria.py"
                  % ", ".join(rancias))
            return 1
        print("galería al día (es + en)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
