---
title: mg
section: 1
footer: mg 3.0.0-beta
date: 2026
author: Alejandro Aguilar Sierra <algsierra@gmail.com>
---

# NAME

mg - MetaGráfica, a descriptive language for technical and scientific figures.

# SYNOPSIS

**mg** [**-h**|**-v**] *figure.mg* [*output*]

# OPTIONS

**-h**
:   Display a friendly help message.

**-v**
:   Display the current version.

The **output format is chosen by the extension** of the output file: `.eps`
(default), `.svg` or `.pdf`. With no output file, `mg figure.mg` writes
*figure.eps*.

# DESCRIPTION

MetaGráfica is a language for describing figures. You write *what the figure is* —
points, lines, shapes, labels — and **mg** compiles it. There is no canvas to click
on: a figure is source code, so it can be kept under version control, diffed,
parameterized and regenerated.

Being vectorial, the basic element is not a pixel but a **point**, a pair of
coordinates *x y*. A series of points is a **path**, and paths are what the drawing
commands consume. Coordinates are yours to choose — you declare the range you want
to work in — while sizes that must not depend on that choice (line widths, text,
markers) are given in typographic points, 1/72 inch.

Figures can be grouped into named **structures** that are placed, scaled, rotated
and repeated. That, and not the drawing commands, is where the language earns its
keep for a complicated figure.

## Beta

This is version 3.0.0-**beta**, and both halves of that word matter:

*  **The language can still change.** Names and arguments are not frozen: `axis`
   recently renamed `title` to `label` and `labels` to `tick_labels`. A figure that
   compiles today may need an edit tomorrow. The old names fail loudly, never
   silently.
*  **Parts are unbuilt.** The specification describes features that do not exist
   yet (see *SEE ALSO*); this page documents only what the binary accepts.

What is here is exercised by a regression corpus on every change, and has typeset
published books.

# A FIRST FIGURE

```
display_size 6 6                            % 6 x 6 cm on paper
world_window -2.2 2.2 -4.6 0.6              % your coordinates

polyline { -1 0  1 0 }                      % the support
polyline(dash="dashed") { 0 0  0 -4 }       % the vertical
polyline { 0 0  1.9 -3.2 }                  % the string
dot(4) { 1.9 -3.2 }                         % the bob, 4 pt across
arc(1.3, from=270, to=301) { 0 0 }          % the angle
text("$\theta$") { 0.75 -1.55 }
```

Every command follows the same shape: a name, its **options in parentheses**, and
its **path in braces**. Comments run from `%` to the end of the line.

# THE CANVAS

`display_size dx dy`
:   Size of the figure **on paper**, in centimetres (default 10 10).

`world_window xmin xmax ymin ymax`
:   The coordinate range **you** want to work in (default 0 1 0 1). Everything you
    draw is expressed in these units.

The world window is mapped onto the display area **without distortion**: one unit in
*x* is as long as one unit in *y*, so circles stay round. If the two aspect ratios
differ, the figure is centred and the leftover becomes margin. Give
`world_window` the aspect ratio of `display_size` when you want to fill the page.

Line widths, text sizes and markers are **physical**: they are given in typographic
points and do not change when you change the window.

# SHAPES

Each command draws one instance **per point** (or per pair of points) of its path,
so one line can draw many things. A path is a series of points `x1 y1 x2 y2 …`; a
`;` separates disjoint subpaths.

`polyline { path }`
:   Join the points with straight lines. `polyline(closed=true)` closes the outline
    without filling it. `marker_start=`, `marker_mid=`, `marker_end=` put a marker
    on the first point, the interior ones or the last. They take the same names as
    `marker`'s `shape=`, structures included — here the argument is called `marker`
    because it names *what* you put, while in the primitive it is `shape` because
    the primitive already *is* the marker. An arrow turns to follow the line;
    `marker_orient="fixed"` stops it. `marker_size=`, `marker_color=` and
    `marker_fill=` style it.

`polygon { path }`
:   Closed and filled.

`rectangle { x1 y1  x2 y2 }`
:   One rectangle per **pair** of points: lower-left and upper-right corners.

`circle(r) { path }`
:   A circle of radius `r` centred on every point.

`arc(r, from=, to=) { path }`
:   Arc of radius `r`, from `from` to `to` degrees, centred on every point.

`ellipse(rx, ry) { path }`
:   Like `circle`, with horizontal and vertical radii.

`bezier { path }`
:   Cubic Bézier. Four control points per segment: the first and last are on the
    curve, the middle two are the tangents.

`sine(amplitude=, half_cycles=, phase=) { x1 y1  x2 y2 }`
:   A sine segment between two points.

`marker(d, shape="…") { path }`
:   A symbol of diameter `d` **in typographic points** at every point — its size is
    physical, so it survives any scaling and never distorts. `shape=` is
    `"circle"` (default), `"square"`, `"diamond"`, `"cross"`, `"x"`, `"arrow"`, **or
    the name of one of your own structures**. Filled by default; `color=` without
    `fill=` leaves it hollow — there is no separate "disc" shape because shape and
    fill are independent. An `arrow` turns to follow the path; anything else stays
    fixed unless you ask for `marker_orient="auto"`.

`dot(d) { path }`
:   Shorthand for the common case, a filled disc: `dot(d)` is `marker(d)`. It takes
    no `shape=` — for any other shape the primitive is `marker`.

`polybar(width=w) { path }`
:   One bar per point, from a common base, `w` wide in world units; each point is
    the **top centre** of its bar. `dir="horizontal"` grows along *x* instead.

`compound { … }`
:   Fuse several of the above into a single path, so that one fill covers them all.

# APPEARANCE

These are **state**: they apply to everything after them, until the end of the
enclosing block. Any of them can also be given to a single shape as an argument —
`polyline(color="red", line_width=0.4) { … }` — in which case it applies to that
shape only.

`color c`
:   Colour of lines and text. A CSS name (`"red"`, `"steelblue"`), a hex string
    (`"#a03000"`), or `gray(g)` with *g* from 0 (black) to 1 (white).

`fill c`
:   Fill closed shapes with colour `c`. `fill "none"` stops filling.

`line_width w`
:   In typographic points. `line_width 0` is the thinnest line the device can draw.

`dash "style"`
:   `"solid"`, `"dashed"`, `"dotted"`, `"longdashed"`, `"dashdot"`.

`hatch a [gap]`
:   Fill with a hatch pattern instead of a solid colour: `a` is the angle in degrees
    (any value), or a name — `"hatch"`, `"hatchback"`, `"crosshatch"`. `hatch_gap`
    sets the spacing in points.

`outlinefill`
:   Also stroke the outline of filled shapes. Without it, a fill has no contour.

# TEXT AND MATHEMATICS

`text("string") { path }`
:   Draw the string at every point of the path.

`font "face"`, `font_size n`
:   Face is one or more of `roman` (default), `sanserif`, `courier`, `bold`,
    `italic`. Size in typographic points.

`align "a"`, `valign "v"`
:   Horizontal `left` (default), `center`, `right`; vertical `baseline` (default),
    `top`, `middle`, `bottom`.

Inside the string, `$…$` switches to **mathematics**, where `_` and `^` are
subscript and superscript (`{ }` groups them), and `\name` gives a Greek letter or
symbol — `\alpha`, `\varphi`, `\infty`. Outside `$…$` those characters are literal.
`/i`, `/b`, `/r`, `/s` switch to italic, bold, roman and sans-serif for what
follows.

```
text("$\Delta E = h\nu$") { 2 3 }
text("mass $m_e$ at $t_0$", align="center") { 5 1 }
```

Mathematics is set in **Latin Modern Math**, which **mg** embeds in the output, so
the figure needs no fonts installed to render elsewhere. Greek, operators,
relations, arrows, italic variables and upright digits all travel as Unicode
codepoints and come out identical in EPS, SVG and PDF. Note that a math `-` is set
as a minus sign, not a hyphen.

## Encoding

Source files are **UTF-8**.

Running text covers the whole repertoire of the standard PostScript fonts:
accented Latin, `¿¡ «» ° × ± µ`, and the typographic punctuation those fonts carry
but that Latin-1 cannot name — curly quotes, en and em dashes, ellipses, bullets,
daggers, per-mille, trademark, euro, œ. Each backend resolves these natively: SVG
emits UTF-8, PDF its own encoding, EPS a custom encoding vector.

The limit is the **font's repertoire**, not the encoding. A character with no glyph
in the text fonts — Greek in running prose, Cyrillic, CJK, Vietnamese tone marks —
is dropped, and **mg** warns on standard error naming the codepoint and the string
it appeared in. Greek in *mathematics* is unaffected: write `$\alpha$` rather than
a literal α.

# PLOTS AND AXES

`plot(x=(x0,x1), y=(y0,y1), box=(bx0,by0, bx1,by1)) { … }`
:   Everything inside the block is written in **data units** and mapped onto `box`,
    a rectangle in world coordinates. `xscale=`/`yscale="log"` make an axis
    logarithmic. `grid=true` (or a colour) draws a grid behind the content.

`xaxis(…)`, `yaxis(…)`
:   Written **inside** a `plot`, they inherit its ranges and scale. Options:
    `step` between ticks, `start` for the first one, `decimals`, `ticks=`
    (`"out"`, `"in"`, `"both"`, `"none"`), `tick_labels=` (`true`/`false`),
    `label="name of the axis"` with `label_at=` (`"center"`, `"start"`, `"end"`),
    `base=v` to cross the other axis at *v* rather than at the edge of the box,
    `extend=` to run the line past its ticks, and `minor=true` for the minor ticks
    of a log axis.

`axis(…) { x1 y1  x2 y2 }`
:   The same axis, standalone, between two points, with `from=`/`to=` for its range.

`grid(xstep=, ystep=) { x1 y1  x2 y2 }`, `ticks(…)`, `numbers(…)`
:   Lower-level generators, for when `plot` does not fit.

```
plot(x=(0,10), y=(0,100), box=(0,0, 9,4.5), grid=true) {
    polyline { 0 0  1 1  2 4  3 9  4 16  5 25 }
    xaxis(step=2, label="x")
    yaxis(step=25, label="$y = x^2$")
}
```

# STRUCTURES

A **structure** is a named group of anything — shapes, state, transformations,
other structures. Defining one draws nothing; using it does.

`struct Name(p1, p2) { … }`
:   Define. Parameters are optional and behave like variables inside.

`Name()`
:   Draw it. `Name(at=(x,y), scale=s, rotate=deg)` places it: `at` moves it, `scale`
    resizes, `rotate` turns it.

`place(Name, scale=) { path }`
:   Put a copy at every point of the path, **turned to follow** it — the path may be
    a line, or an arc with `r=`, `from=`, `to=`. `both_sides=true` mirrors it.

`fit(Name) { x1 y1  x2 y2 }`
:   Squeeze the structure into that rectangle. `stretch=true` allows distorting it.

`repeat(Name, count=n, at=, advance=)`
:   *n* copies, each `advance`d from the last.

A structure may declare its own `world_window`, and then it is written in its own
convenient coordinates wherever it ends up.

```
struct Square() {
    polyline(closed=true) { -1 -1  1 -1  1 1  -1 1 }
}

for i = 0 to 11 {
    Square(rotate = i*7.5, scale = 1 + i*0.35)
}
```

# TRANSFORMATIONS

`translate tx ty`, `rotate deg`, `scale sx [sy]`, `shear sx sy`

They apply to everything that follows **in the current block**, and are undone on
leaving it, so `{ rotate 30  Name() }` turns that one use and nothing else. They
compose in order of appearance.

# EXPRESSIONS AND CONTROL

Variables need no declaration: `n = 60`. Arithmetic is `+ - * / ^` with the usual
precedence, and `+` also joins strings. Functions: `sin`, `cos`, `tan`, `atan2`,
`sqrt`, `abs`, `mod`, `len`, `gray`, and `str(x[, decimals])` to put a number in a
string. Angles are in **radians** for these functions, but in **degrees** for
`rotate` and `arc`. Lists are `[a, b, c]`, indexed `list[i]` from 0.

`for i = a to b { … }`
:   Repeat, with `i` running from *a* to *b*.

`if cond { … } else if cond { … } else { … }`
:   Conditionals; comparisons are `== != < <= > >=`, joined with `and` and `or` and
    negated with `not`. Zero is false, anything else true.

`include "file.mg"`
:   Insert another file — how a library of structures is shared.

`{ … }`
:   A block on its own bounds state and transformations, and nothing more.

> In coordinate braces, an identifier followed by `(` reads as a function call:
> `polyline { x (y+1) … }` parses `x(y+1)`. Parenthesize every coordinate of the
> path, or none.

# EXAMPLES

A pendulum: see *A FIRST FIGURE* above.

Data, with axes that label themselves — the source of *examples/quickstart.mg*:

```
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

```
mg quickstart.mg quickstart.svg
```

# FILES

*examples/*
:   The working corpus: every file there compiles with this binary and is checked on
    every change. Start with `quickstart.mg` (a plot) and `fig2-5.mg` (an
    illustration). Some of its files are meant to be `include`d rather than
    compiled — `arrow.mg` defines the arrow markers, `curvas3.mg` the wave curves.

*lib/*
:   Structure libraries: `pseudo3d.mg` builds oblique-projection boxes and planes.

*examples/v1/*
:   The frozen corpus of the old two-letter grammar, kept as a migration reference.
    **Those files do not compile with this binary.**

# SEE ALSO

*especificacion_mg.md*, in the source tree, is the language specification. It is a
**design** document: it also describes what is not built yet, and why. This page
describes what works.

# BUGS

Please report any bugs you find, with the output of `mg -v` and the smallest
*.mg* file that shows the problem.

# COPYRIGHT

License: GPL 3.0
Copyright (c) 1988-2026 Alejandro Aguilar Sierra (algsierra@gmail.com)
