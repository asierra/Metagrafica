# MetaGráfica

![C++ Standard](https://img.shields.io/badge/C%2B%2B-14-blue.svg)
![License](https://img.shields.io/badge/License-GPL%203.0-green.svg)
![Version](https://img.shields.io/badge/version-3.0.0--beta-orange.svg)

**English** · [Español](README.es.md)

**A descriptive language for technical and scientific figures.**

You describe *what the figure is* — points, paths, structures, transformations — and
`mg` compiles it to **EPS**, **SVG** or **PDF**. No GUI, no mouse: a figure is source
code, so it can be versioned, diffed, parameterized and regenerated.

It typeset the figures of Ana María Cetto & Luis de la Peña, *[Quantum Mechanics: A
Physical Approach](https://doi.org/10.1017/9781009679633)* (Cambridge University Press,
2025) and, in earlier versions, those of Luis de la Peña's *[Introducción a la mecánica
cuántica](https://www.fondodeculturaeconomica.com/Ficha/9786071601766/F)* (FCE/UNAM,
3rd ed.).

## Quick start

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

![y = x² plotted by MetaGráfica](docs/img/quickstart.svg)

That is the whole file ([`examples/quickstart.mg`](examples/quickstart.mg)). `plot` maps
**data units** onto a physical box in centimetres; the axes inherit the `x=`/`y=` ranges
and label themselves. The `$…$` in the axis label is math markup — MetaGráfica embeds a
TeX font for Greek letters, symbols, superscripts and subscripts.

## Not just plots

Data figures are one use. The language was built for **illustrations** — apparatus
diagrams, sketches, anything a paper needs — and that is where structures, placement
along arcs, arrows and math labels earn their keep:

![Electron diffraction apparatus](docs/img/fig2-5.svg)

> Figure 2.5 of Ana María Cetto & Luis de la Peña, *Quantum Mechanics: A Physical
> Approach*, Cambridge University Press, 2025.
> [doi:10.1017/9781009679633](https://doi.org/10.1017/9781009679633) — reproduced here
> as [`examples/fig2-5.mg`](examples/fig2-5.mg), the source that typeset it.

Under 60 lines of MetaGráfica: the detector is a structure placed at 37°, the beam arrows
are another structure swept along an arc, and `φ` is set in Latin Modern Math.

## Building

```bash
make                 # builds bin/mg and the man page
sudo make install    # optional: puts mg on your PATH
```

Requires a C++14 compiler (`clang++` or `g++`), `flex`, and `pandoc` for the man page.
[libharu](http://libharu.org/) is vendored in `third_party/` for PDF output; there is
nothing else to install.

## Usage

The output format is chosen by the **extension** of the output file:

```bash
bin/mg figure.mg              # → figure.eps
bin/mg figure.mg out.svg      # → SVG
bin/mg figure.mg out.pdf      # → PDF
```

| flag | |
|---|---|
| `-h` | help |
| `-v` | version |

## The language in one minute

A **point** is a pair of coordinates; a **path** is a list of points. Every primitive
takes a path in `{ }` and its style in `( )`:

```text
polyline { 0 0  1 2  3 1 }              % open polyline
polyline(closed=true) { 0 0  1 0  1 1 } % closed outline
polygon { 0 0  1 0  1 1 }               % filled
circle(2) { 5 5  9 5 }                  % one circle per point
rectangle(fill="steelblue") { 0 0  4 3 }
text("mass $m_e$", align="center") { 5 1 }
```

**Structures** are the heart of the language: a named group you can place, scale, rotate
and repeat, in homogeneous coordinates — no external programming language needed.

```text
struct Square() {
    polyline(closed=true) { -1 -1  1 -1  1 1  -1 1 }
}

for i = 0 to 11 {
    Square(rotate = i*7.5, scale = 1 + i*0.35)
}
```

The full language is specified in [`especificacion_mg.md`](especificacion_mg.md), and
`man mg` is the reference. MetaGráfica is deliberately **not** a general-purpose
programming language: it has variables, expressions, `for` and `if`, and stops there.

## Examples

[`examples/`](examples/) holds the working corpus — every file there compiles with the
current binary and is checked on every change:

```bash
bin/mg examples/fig6-4.mg out.svg
```

| | |
|---|---|
| `quickstart.mg` | the figure above |
| `fig2-5.mg` | the illustration above (structures, arcs, arrows) |
| `fig2-3.mg` | linear plot with grid and axis labels |
| `fig6-4.mg` | **log** axis, math labels, data annotations |
| `fig4-5.mg` | three panels, interior axes, analytic curves |
| `fig_polybar.mg` | bar histogram with hatching |
| `primitives.mg`, `fill_styles.mg`, `line_patterns.mg` | reference sheets |

## Project status

**This is a beta.** Two things follow from that. The **language can still change**: names
and arguments are not frozen — `axis` recently renamed `title` to `label` and `labels` to
`tick_labels`, so a figure that compiles today may need an edit tomorrow (the old names
fail loudly, never silently). And **parts are unbuilt**: the specification describes
features that do not exist yet. What is here is exercised by the regression corpus on
every change, and has typeset published books.

`mg` is the **version 3** compiler (`MG_VERSION 3.0.0-beta`). The earlier two-letter grammar
(`PL`, `CR`, `GNNUM`…) is frozen on the `v1-legacy` branch, and its corpus lives in
[`examples/v1/`](examples/v1/) as the migration oracle. **Those sources do not compile
with this binary.**

Every change is gated by a regression harness over the whole corpus (`bash test/run.sh
check`): byte-exact goldens for the three backends, a Ghostscript pass over the EPS, and
a cross-backend parity check.

## History

MetaGráfica has been rewritten three times, and every version typeset something:

| | year | language | output |
|---|---|---|---|
| **0** | 1988 | Pascal + assembler | drove an HP laser printer directly — the first published paper |
| **1** | 1991 | C | the first book |
| **2** | 1999–2024 | C++ / STL | EPS only — three more books |
| **3** | 2026 | C++14 | EPS, SVG, PDF |

Version 0 was written when no graphics application produced output of the quality a
scientific paper needed. Version 2 emitted Encapsulated PostScript only, with Latin-1
text and the standard `symbol` font for Greek and mathematics; PostScript stagnated and
never caught the Unicode revolution, but it is still widely supported and converts
trivially to PDF.

This version keeps the descriptive core and adds the SVG and PDF backends, an isometric
coordinate model, Latin Modern Math for symbols, and the `plot` family for data figures.
The earlier two-letter grammar (`PL`, `CR`, `GNNUM`…) is gone; see *Project status*.

## License

GPL 3.0 — Copyright © 1988–2026 Alejandro Aguilar Sierra (algsierra@gmail.com)

The range spans the life of the work — see *History* above. Full text in
[`LICENSE`](LICENSE).
