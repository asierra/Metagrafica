# MetaGráfica

![C++ Standard](https://img.shields.io/badge/C%2B%2B-14-blue.svg)
![License](https://img.shields.io/badge/License-GPL%203.0-green.svg)
![Version](https://img.shields.io/badge/version-3.0.0--beta-orange.svg)

**English** · [Español](README.es.md)

**A descriptive graphics language for high-quality technical and scientific figures.**

You describe *what the figure is* — points, paths, structures, transformations — and
`mg` compiles it to any of **EPS**, **SVG** or **PDF**, for print or online publishing.
No GUI, no mouse needed: a figure is source code, so it can be versioned, diffed,
parameterized and regenerated.

Originally created for scientific publications such as Ana María Cetto and Luis de la
Peña's Quantum Mechanics textbooks, *[Quantum Mechanics: A Physical
Approach](https://doi.org/10.1017/9781009679633)* (Cambridge University Press, 2025) and
*[Introducción a la mecánica
cuántica](https://www.fondodeculturaeconomica.com/Ficha/9786071601766/F)* by Luis de la
Peña (FCE/UNAM, 3rd ed.), and other scientific papers, it has evolved in stages over
nearly four decades.

## Quick start

Nothing beats an example for a first impression of the language.

```text
display_size 9 5.5
font_size 9
world_window -2 11 -1.5 5.5

plot(x=(0,10), y=(0,100), box=(0,0, 9,4.5), grid=true) {
    line_width 0.8
    polyline { 0 0  1 1  2 4  3 9  4 16  5 25  6 36  7 49  8 64  9 81  10 100 }
    marker(size=4, shape="cross") {
        0.9 10.0
        2.5 15.0
        4.2 30.0
        6.75 60.2
    }  
    legend(at="top-left", margin=10, sample_width=20, gap=5, font_size=8) {
        entry("Theoretical") { polyline { 0 0.5  1 0.5 } }
        entry("Experimental") { marker(3, shape="cross", color="black") { 0.5 0.5 } }
    }
    xaxis(step=2, label="x")
    yaxis(step=25, label="$y = x^2$")
}
```

```bash
bin/mg examples/quickstart.mg quickstart.svg
```

![y = x² plotted by MetaGráfica](docs/img/quickstart.svg)

That is the whole file ([`examples/quickstart.mg`](examples/quickstart.mg)). `plot` maps
**data units** onto a physical box in centimetres; the axes inherit the `x=`/`y=` ranges
and label themselves. The `$…$` in the axis label is math markup (a LaTeX subset) —
MetaGráfica embeds a TeX font for Greek letters, symbols, superscripts and subscripts.

## Illustrations

It shines at plots, but MetaGráfica is just as capable at **illustrations** — apparatus
diagrams, sketches, anything a paper needs — and that is where structures, placement
along arcs, arrows and math labels stand out:

![Electron diffraction apparatus](docs/img/fig2-5.svg)

> Figure 2.5 of Ana María Cetto & Luis de la Peña, *Quantum Mechanics: A Physical
> Approach*, Cambridge University Press, 2025.
> [doi:10.1017/9781009679633](https://doi.org/10.1017/9781009679633) — reproduced here
> as [`examples/fig2-5.mg`](examples/fig2-5.mg), the source that typeset it.

Under 60 lines of MetaGráfica: the detector is a `struct` (a grouping of several graphics
elements) placed at 37°, the beam and detector-swing arrows are markers that **orient
themselves** to their line or arc — no angles or positions worked out by hand — and `φ`
is set in Latin Modern Math.

## Text and mathematics

Source files are **UTF-8**.

**Mathematics is Unicode from end to end.** Greek, operators, relations, arrows, italic
variables and upright digits all travel as codepoints and are set in **Latin Modern
Math**, which `mg` embeds in the output — the same typeface TeX produces, and identical
across the three backends. A figure needs no fonts installed to render elsewhere.

**Running text covers the whole repertoire of the standard PostScript fonts**: accented
Latin, `¿¡ «» ° × ± µ`, and the typographic punctuation those fonts have always carried
but that Latin-1 could not name — “curly quotes”, ‘single’ ones, en and em dashes,
ellipses, bullets, daggers, ‰, ™, €, œ. Each backend resolves them natively: SVG emits
UTF-8, PDF its own encoding, EPS a custom encoding vector.

**The ceiling is the font's repertoire, not the encoding.** Other writing systems —
Greek *prose*, Cyrillic, CJK, or Vietnamese tone marks — are dropped with a warning that
names the character, because the glyph simply is not in the font. Supporting them means
embedding a Unicode text font, the way Latin Modern Math is embedded for mathematics.
Greek in *mathematics* works today: write `$\alpha$`, not a literal α.

## Building

```bash
make                 # builds bin/mg and the man page
sudo make install    # optional: puts mg on your PATH
```

Requires a C++14 compiler (`clang++` or `g++`), `flex`, and `pandoc` for the man page.
The library for PDF output, [libharu](http://libharu.org/), is vendored in
`third_party/`, and there is nothing else to install.

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

**Structures** are the heart of the language: you can group different graphics elements
together with their attributes and place, scale, rotate and repeat them as a unit, in
homogeneous coordinates, using just their name.

```text
struct Square() {
	circle(0.5) { 0 0 }
    polyline(closed=true) { -1 -1  1 -1  1 1  -1 1 }
}

for i = 0 to 11 {
    Square(rotate = i*7.5, scale = 1 + i*0.35)
}
```

The full language is specified in [`especificacion_mg.md`](especificacion_mg.md) *(in
Spanish)*; `man mg` is the reference in English. MetaGráfica is deliberately **not** a
general-purpose programming language: it has variables, expressions, `for` and `if`,
logical expressions, and not much more.

## Examples

[`examples/`](examples/) holds the working corpus, where you can see the different
features in action — every file there compiles with the current binary and is checked on
every change:

```bash
bin/mg examples/fig6-4.mg out.svg
```

| | |
|---|---|
| `quickstart.mg` | the figure above |
| `fig2-5.mg` | the illustration above (structures, arcs, arrows) |
| `fig6-4.mg` | **log** axis, math labels, data annotations |
| `fig4-4.mg` | three panels, interior axes, analytic curves |
| `franck_condon.mg`, `turning_points.mg` | **fully computed figures**: you give the physical parameters and the geometry follows |
| `fig_polybar.mg` | bar histogram with hatching |
| `primitives.mg`, `fill_styles.mg`, `line_patterns.mg` | reference sheets |

**[Computing instead of measuring](docs/computing_instead_of_measuring.md)** develops that
last case: figures whose geometry comes out of the formulas rather than out of measuring a
drawing. Change one number —the anharmonicity of a potential, the node count of a state— and
the whole figure rearranges itself, because everything else is derived.

## Project status

**This version is still beta**, and two things follow from that. The **language can
still change**: names and arguments are not frozen, so a figure that compiles today may
need an edit tomorrow (the old names fail loudly, never silently). And **parts are
unbuilt**: the specification describes features that do not exist yet. What is here is
exercised by the regression corpus on every change.

Four things stand between here and release ([§22.7](especificacion_mg.md) carries the full
list), and they come **in this order**:

1. **Finishing `plot`** — `table` is still reserved and unbuilt, waiting for figures that
   ask for it. (`rule` and `legend` are done: see [§13.8](especificacion_mg.md) and the
   example above.)
2. **A user reference** — there are three documents today and none of them is the one
   that is needed: this README is a front page, `man mg` documents the *binary* rather
   than the language, and the specification is *forward-looking* (it describes things that
   do not exist yet). What is missing is the document that describes, completely and
   without history, **what is there**.
3. **Real use by other people** — a period of figures written by someone other than the
   author, with whatever improvements that motivates. **If you use MG at this stage, your
   opinion on the names and the ergonomics is exactly what is missing**, and it can still
   be acted on at no cost to anyone. It comes after the reference because without
   something to read, what you get back is "I couldn't find how to do X" rather than "I
   found it and it is awkward", and only the second one helps decide names.
4. **Freezing the grammar** — a figure that compiles keeps compiling. It comes last on
   purpose: it is the promise that makes every later correction expensive.

> **Why the order matters, and is not bureaucracy.** What the word "beta" buys you is
> *permission to break things*: today a rename costs a `sed`, and after 1.0 it costs a
> migration and a major version number. So freezing has to come last. The case that proves
> it happened in this project: `axis(title=)` meant the *axis name* and `axis(labels=)` the
> *tick numbers* — the two names anyone reaches for first meant something other than they
> do everywhere else. It was fixed in an afternoon because there were no users; with twenty
> other people's figures in the wild, it would not have been.

*(Text encoding used to be on this list. Source files are UTF-8 and running text now
covers the full repertoire of the standard PostScript fonts — accented Latin plus
typographic punctuation: “curly quotes”, en and em dashes, ellipses, bullets, ‰, ™, €.
Mathematics is Unicode end to end. See [Text and mathematics](#text-and-mathematics).)*

`mg` is the **version 3** compiler (`MG_VERSION 3.0.0-beta`). The earlier two-letter grammar
(`PL`, `CR`, `GNNUM`…) — **version 2** in the table above — is frozen on the `v1-legacy`
branch, and its corpus lives in [`examples/v1/`](examples/v1/) as the migration oracle.
*(Branch and directory were named before the numbering was aligned with the publication
history, and keep the old name so existing links do not break.)* **Those sources do not
compile with this binary.**

Every change is gated by a regression harness over the whole corpus (`bash test/run.sh
check`): byte-exact goldens for the three backends, a Ghostscript pass over the EPS, and
a cross-backend parity check.

## History

MetaGráfica has been rewritten four times, and every version has evolved. Like other
graphics languages, it was initially loosely inspired by MetaPost (hence some
conventions, like `%` comments). Its output can be embedded in a LaTeX document:

| Version | Year | Language | Output |
|---|---|---|---|
| **0** | 1988 | Pascal + assembler | first published paper |
| **1** | 1991 | C | first book, with figures embedded in TeX |
| **2** | 1999–2024 | C++ / STL | EPS only — more books and papers |
| **3** | 2026 | C++14 | EPS, SVG, PDF |

Version 0 drove a laser printer directly and was written when no graphics application
produced output of the quality a scientific paper needed. Version 2 began in 1999 with
the decision to emit Encapsulated PostScript — then *the* graphics language for
publishing. Its text was Latin-1, with the standard `symbol` font for Greek and
mathematics. PostScript stagnated and never caught the Unicode revolution, but it is
still widely supported and converts trivially to PDF — and, as it turns out, its fonts
were never the limitation the encoding made them look (see below).

This version keeps the descriptive core and adds the SVG and PDF backends, an isometric
coordinate model, Latin Modern Math for symbols, and the `plot` family for data figures.
The two-letter grammar is gone and the language stopped resembling assembly and is now
much more powerful. See *Project status*.

## License

GPL 3.0 — Copyright © 1988–2026 Alejandro Aguilar Sierra (algsierra@gmail.com)

The range spans the life of the work — see *History* above. Full text in
[`LICENSE`](LICENSE).
