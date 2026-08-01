# MetaGráfica — language reference

This reference can be read **straight through once** or consulted while building a new figure. The tables
at the end are the summary.

> **Draft — V3 beta.** The V3 grammar can still change, and this reference tracks it. It is the English
> version of [`referencia.md`](referencia.md) (Spanish); the two are kept in step.

---

## 1. What it is

You describe *what the figure is* and `mg` compiles it:

```bash
mg figure.mg              # → figure.eps
mg figure.mg output.svg   # the format is chosen by the extension: .eps .svg .pdf
```

A minimal file produces output with no preamble:

```octave
circle(1) { 0 0 }
polyline { 0 0  2 1 }
```

**What MG is not.** It is not a general-purpose programming language: it has variables, expressions, `for`,
`if` and little more. It does not analyze data — `polybar` receives already-counted intervals, not
observations; to get from a spreadsheet to a `.mg` there is `tools/hist2mg.py`. It does no 3D —what
[§13](#13-pseudo-3d-scenes) has is a camera that projects, with no hidden surfaces and no lighting—. It
does not typeset paragraphs.

---

## 2. The canvas and units

```octave
display_size 12 8              % PHYSICAL size of the drawing, in centimeters
world_window -1 11 -2 6        % visible region of the plane: xmin xmax ymin ymax
font_size 9                    % body text size in typographic points
```

`display_size` is the size of the box the figure occupies when embedded in another document —not that of a
page—, in centimeters.

**Two classes of quantity, and they don't mix:**

| class | unit | what it is | examples |
|---|---|---|---|
| **world** | those of `world_window` | transformed by the coordinate system | coordinates, `circle` radius, `grid` `xstep` |
| **physical** | typographic points (pt) | immune to the window and to transforms | `line_width`, `font_size`, `dot`/`marker` radius, `hatch_gap`, and all of `plot`'s furniture (`margin`, `tick_size`, `col_widths`…) |

That is why `dot` is the right marker inside a distorted chart: it lands where it should and does not turn
into an ellipse.

> ⚠️ **If you see nothing, or half a figure, check `world_window` before anything else:** it is a fixed
> crop, it does not adjust to your data. It is stumble #1 ([details](#15-common-mistakes)).

**The engine is isometric:** the scale is the same in x and y. If the proportion of `world_window` does not
match that of `display_size`, the figure ends up centered with margins (*letterbox*); to fill the canvas,
match the two proportions.

> 🔑 **And that is where the rule for choosing a tool comes from.** `world_window` is for figures whose two
> axes **share a unit** —a plane, an orbit, a map, a geometric figure—, because there isometry is exactly
> what you want. If x and y measure different things, or their ranges differ by more than two or three
> times, the same scale on both flattens the figure against an edge: `world_window 0 1 0 6` on a 12 × 8 cm
> canvas leaves the whole x axis 1.3 cm wide. That is what `plot` is for ([§11](#11-charts)): it maps data
> to a box and stretches each axis on its own. **It is not a shortcut for making charts: it is the correct
> coordinate system when the axes aren't comparable** — and it brings axes, ticks and labels along.

> 💡 **`plot` is the exception, and it's deliberate.** A `plot` maps its **data units** to its box, and
> there x and y stretch independently: seconds against volts have no common proportion to respect. The
> isometric rule governs the **plane of the figure** —centimeters against world units—; a `plot`'s data
> mapping is another thing ([§11](#11-charts)).

> ⚠️ **Shrinking `display_size` does not shrink the text** — it enlarges it relatively ([details](#15-common-mistakes)).

---

## 3. Paths

A **point** is a pair of coordinates; a **path** is a list of points. It is the basic data structure of the
language: almost everything drawn consumes a path, and that list can be written literally, built with
operations, or named and reused.

The common form is the **literal block** each primitive carries:

```octave
polyline { 0 0  1 2  3 1 }
```

But a path is also a **value** with its own name, declared with `path` and passed to a primitive by
prefixing `&`:

```octave
path profile = { 0 0  1 2  3 1  4 0 }
polyline(&profile)                         % draw it…
polygon(&profile)                          % …or fill it, it's the same path
```

**A path is the same list of points, whoever interprets it.** So `&profile` serves `polyline` (traces it),
`polygon` (fills it) or `polybar` (reads each point as the top of a bar) alike — each primitive reads it its
own way.

That is enough to draw. A path can also be **operated on** —chained, mirrored, smoothed, sampled— and that
lives apart, in §10: it is not needed to start.

> ⚠️ **`&path` goes as the FIRST argument, always**, with the rest named after: `dot(&p, size=2)` ✅,
> `dot(2, &p)` ❌ ([details](#15-common-mistakes)).

---

## 4. Primitives

Each primitive carries its **coordinates in `{ }`** and its **arguments in `( )`**:

```octave
polyline { 0 0  1 2  3 1 }                 % open polyline
polyline(closed=true) { 0 0  1 0  1 1 }    % closed contour, without repeating the vertex
polygon { 0 0  1 0  1 1 }                  % filled
rectangle { 0 0  4 3 }                     % two opposite corners
rectangle(w=4, h=3, at=(2, 1.5))           % …or center + size (at = center; only w = square)
circle(2) { 5 5  9 5 }                     % one circle per point; 2 = radius (world)
ellipse(3, 1.5) { 5 5 }                    % x, y radii
arc(2, from=0, to=120) { 0 0 }             % degrees, positive = counterclockwise
arc(4, 2, from=270, to=450) { 0 0 }        % ELLIPTICAL arc (rx, ry); also rx=, ry=
dot(2) { 1 1  2 3 }                        % disc of PHYSICAL radius 2 pt
marker(3, shape="cross") { 1 1 }           % symbol of physical radius
bezier { 0 0  1 2  3 2  4 0 }              % p0 c1 c2 p1 [c1 c2 p2 …]  (3k+1 points)
smooth { 0 0  1 2  3 1  4 3 }              % NODES: the tangents are derived by the compiler
polybar(width=0.5) { 1 3  2 5  3 4 }       % each point is the top center of a bar
sine(half_cycles=2, amplitude=1) { 0 0  4 0 }
```

> ⚠️ **The `{` goes on the SAME LINE as the primitive.** Inside them, both `( )` and `{ }` may
> span as many lines as you like; what you cannot have is a line break **before** the `{`
> ([details](#15-common-mistakes)).

`polyline`, `polygon` and `bezier` accept **disjoint subpaths** separated by `;`:

```octave
polyline { 0 0  1 1 ;  2 0  3 1 }          % two strokes, same style
```

In the block, a coordinate can be a pair of scalars (`x y`) **or a point `[x,y]`** —a list of two, like
`point_at` (§10) returns, or a literal—, and they mix:

```octave
marker(shape="x") { point_at(&curve, 0.5) }   % a direct point
polyline { 0 0  (p)  5 5 }                     % scalars and a point p mixed
```

**`marker` shapes:** `circle`, `square`, `diamond`, `cross`, `x`, `triangle`, `arrow`, `circle-dot`. Also
the name of your own `struct`. `dot(r)` is the disc shortcut and carries no `shape=`.

**Arrows and markers ON a line.** This is what you almost always want, and it is **not** done by
stamping a separate `marker` at the tip: every path primitive takes them as attributes and places
and **orients** them by itself.

```octave
polyline(marker_end="arrow") { 1 2  4 2 }                       % head at the end
polyline(marker_start="arrow", marker_end="arrow") { 1 1  4 1 }  % at both ends
polyline(marker="circle", marker_size=2) { 5 1  6 2  7 1 }       % at EVERY vertex
polyline(marker_end="arrow", marker_end_shift=0.3) { 5 2  7 2 }  % pulled back along the line
```

| attribute | |
|---|---|
| `marker_start` / `marker_mid` / `marker_end` | where: first vertex, interior ones, last |
| `marker` | the shape **at every** vertex (or just the shape, when `marker_at` is present) |
| `marker_size` | **physical** radius, in pt |
| `marker_color` / `marker_fill` | colour; by default a marker **inherits its line's** |
| `marker_orient` | `"auto"` (tangent, the default), `"reverse"`, `"fixed"`, or an angle |
| `marker_start_orient` / `marker_end_orient` | the same, for one end only |
| `marker_start_shift` / `marker_end_shift` | slide the marker along the line |

The arrow is **oriented to the local tangent** without being asked, so a polyline or an arc gets
it properly turned — which is why stamping a loose `marker` at the tip is worse: you have to
repeat the coordinate and work out the angle by hand.

**Markers on an arc or an ellipse (`marker_at`).** `arc`, `ellipse` and `circle` take markers at
**parametric** positions, not just at the ends, each one oriented to the local tangent:

```octave
ellipse(3, 5, marker="arrow", marker_at=[0, 180]) { 0 0 }   % direction-of-travel arrows
arc(2, from=0, to=180, marker="arrow", marker_at=45) { 0 0 } % a single angle, no list
```

The values are in **degrees**, the same parameter as the primitive's own `from`/`to` (they are not
`t ∈ [0,1]` as in `point_at`/`sample`, which travel by arc length). With `marker_at` present,
`marker=` is only the **shape**: the endpoints must be asked for separately with
`marker_start`/`marker_end`. ⚠️ On an ellipse, parametric angles are **not** evenly spaced along
the curve. A marker is **physically** sized and of a single colour; to stamp a whole struct, with
its colours, on that same arc, use `place(..., at=)`.

**Trimming an arc: drawing only the stretch that shows.** `from`/`to` are not normalised —the
sweep is `to − from`, it may exceed 360° and it may start anywhere—, so an `arc` is how you draw
*part* of an ellipse: the half of an orbit that the planet does not hide, for instance. And the
cut angles are computed **inside the `.mg` itself**, because the evaluator carries trigonometry
([§7](#7-expressions-and-control-flow)): no measuring on the drawing, no splitting the curve by
hand.

```octave
% Orbit with semi-axes a, b, concentric with a globe of radius R. The hidden stretch is
% the one that runs behind AND falls inside the disc; the crossings come from |P(t)|² = R².
s  = sqrt((R*R - a*a)/(b*b - a*a))
tc = deg(asin(s))                               % the half-angle, in degrees
arc(a, b, from=(180+tc), to=(540-tc)) { 0 0 }   % skips the stretch t ∈ (180−tc, 180+tc)
```

The ends of the stroke land on the limb **by construction**, with nothing adjusted, because the
globe's circle and the equation share centre and radius. Note that this is not a contour
intersection problem —path algebra ([§10](#10-path-algebra)) would not help—: occlusion is a
matter of depth, and in 2-D you have to *decide* which half is behind. Full figure in
[`examples/orbita_polar.mg`](../examples/orbita_polar.mg).

**`compound`** joins several primitives into **a single** filled stroke:

```octave
compound(fill="orange") { circle(2) { 0 0 }   circle(1) { 0 0 } }
```

---

## 5. Style: two registers

This is where people stumble most, so it's worth being clear from the start.

**State statement** — holds from where it appears until another changes it:

```octave
color "red"
line_width 0.8
polyline { 0 0  1 1 }      % red
polyline { 1 1  2 0 }      % also red
```

**Per-primitive attribute** — holds only for that primitive:

```octave
polyline(color="red", line_width=0.8) { 0 0  1 1 }
polyline { 1 1  2 0 }      % NOT red
```

| available as a statement | and as an attribute of any primitive |
|---|---|
| `color`, `fill`, `line_width`, `dash`, `hatch`, `hatch_gap`, `gradient`, `gradient_angle`, `outlinefill`, `font`, `font_size`, `align`, `valign` | `color=`, `fill=`, `line_width=`, `dash=`, `hatch=`, `hatch_gap=`, `hatch_angle=`, `gradient=`, `gradient_angle=` |

> ⚠️ **`outlinefill`, `font`, `font_size`, `align` and `valign` are not generic style attributes:**
> `polyline(align="center")` is an error. But **`text()` does accept them**, because there they are not
> borrowed style but its own arguments — `text("m", align="center", font="italic", font_size=9) { 5 1 }` is
> correct. They are two different things that look alike: the **style attribute**, valid on any primitive,
> and the **own argument** of a specific primitive (like `shape=` on `marker` or `closed=` on `polyline`).
> `outlinefill` is valid in neither form: the outline over a fill is requested by giving `color=` alongside
> `fill=`.

**Fill and outline.** `fill=` turns on the fill; `color=` is the stroke. Together = outlined fill.
`fill="none"` turns the fill off.

**Colors:** the **148 CSS names** (`"steelblue"`, `"orange"`…), `"#rrggbb"`, or `gray(0.4)`. No exceptions:
`green` and `orange` are worth what they are in CSS.

**Lines:** `line_width` in pt. `dash` accepts `"solid"`, `"dashed"`, `"dotted"`, `"longdashed"`,
`"shortdashed"`, `"dashdot"`, `"dashdotdot"`.

**Gradients:** `gradient` takes a **list of colours** —the stops, spread evenly along the axis—
and `gradient_angle` the angle in degrees, 0 = left→right:

```octave
rectangle(gradient=["orange", "#D93622", "black"], color="black") { 0 0  10 2 }
rectangle(gradient=[gray(0.8), gray(0.3)], gradient_angle=90) { 0 3  10 5 }
```

> ⚠️ **The angle is the PAGE's, not the figure's.** A rectangle rotated 30° with
> `gradient_angle=0` still shades left to right on the paper. It is the same rule as
> `hatch_angle`, and not an oversight: they are the two ways of filling an area with something
> that is not a flat colour, and both orient themselves against the sheet.

`gradient` and `hatch` are **the same slot**, so a per-primitive `gradient=` overrides an ambient
`hatch` and is restored on the way out, like any attribute. Giving both **on the same primitive**
is an error: there is no order to interpret there. With `color=` alongside, the gradient gets
outlined, just like a `fill=`. See [`examples/espectro.mg`](../examples/espectro.mg).

**Hatching:** `hatch` accepts a **free angle** in degrees or a named style (`"hatch"` 45°, `"hatchback"`
135°, `"crosshatch"` both). `hatch_gap` is the spacing in pt; `hatch_angle` is the **base orientation** (it
decouples the angle from the type, as `hatch_gap` decouples the pitch): on `"crosshatch"` it turns the whole
grid, so `hatch_angle=0` **straightens** it to 0°+90°. Hatching uses the **fill color** (`fill=`, not
`color=`, which outlines the border); there is no `hatch_color`.

---

## 6. Text and mathematics

```octave
text("electron mass", align="center") { 5 1 }
```

Arguments: `align` (`"left"`/`"center"`/`"right"`), `valign` (`"baseline"`/`"top"`/`"middle"`/`"bottom"`),
`font_size` (alias `size`), `color`, `font`.

**Markup inside the string:**

| | |
|---|---|
| `/b` `/i` | bold, italic |
| `/r` `/s` `/c` | roman, sans-serif, monospaced |
| `/n` | **break the line** |
| `$…$` | math mode |
| `\alpha`, `\nabla`, … | symbol (inside or outside math) |
| `_x`  `^x`  `_{xy}`  `^{xy}` | sub- and superscript — **only inside `$…$`** |
| `'` | prime (inside math) |
| `\frac{a}{b}` | fraction (numerator over denominator) — inside `$…$` |
| `\hat{n}` `\vec{v}` | accent over its base: unit vector and vector — inside `$…$` |
| `\,` `\;` `\!` `\quad` | fine math-space adjustment (thin, thick, negative thin, 1 em) |

```octave
text("$\Delta T_1$/n(BT 10.3 - 12.3 $\mu/rm$)", align="center") { 5 1 }
```

Mathematics is set with **Latin Modern Math**, which `mg` embeds in the output: the figure looks the same on
any machine, with no fonts or TeX installed.

> ⚠️ **It's `/n`, not `\n`.** The backslash consumes all the alphabetic text that follows —that's how
> `\alpha` and `\nabla` are read—, so `"one\ntwo"` would look for a symbol called `ntwo`.

In multiline text, `valign` aligns **the whole block**, not each line.

### Math spacing and fractions

Inside `$…$` **the spacing is set by `mg`, not by you**: the spaces you type are ignored and the gap between
symbols comes from its role —relation, binary operator, function—, as in TeX. So `$F = ma$` and `$F=ma$`
look identical (the `=` gets its space on both sides) and `$a + b$` separates the `+` as binary, while a
leading `-` (`$-x$`) stays tight because it is unary. For the fine adjustment the rule doesn't get right
there are four commands: `\,` (thin), `\;` (thick), `\!` (negative thin) and `\quad` (one em).

**Fractions.** `\frac{numerator}{denominator}` composes the fraction in two dimensions —draws the bar and
centers numerator and denominator over it—. They **nest** (`\frac{1}{1+\frac{1}{x}}`) and place their parts
according to the **real height** of the content, so the numerator's subscripts and the denominator's
superscripts clear the bar. They are set at full (display) size and compose inline within a larger formula:

```octave
text("$F = G \frac{m_1 m_2}{r^2}$", size=12) { 1 7 }
```

**Accents.** `\hat{n}` puts a circumflex and `\vec{v}` an arrow, centred over their base and sized to its
width — over a wide letter they come out wide, without having to ask for that separately. The height comes
from the content, so `\hat{n}` rests on the x-height and `\hat{A}` rises to the capital.

```octave
text("$\vec{F} = m\vec{a}$   $\hat{n} \cdot \hat{r}$") { 1 5 }
```

> The mark is **drawn**, it does not come from the font, and that is deliberate: a combining accent has zero
> advance and is placed using the font's internal tables, which MG does not read — the positioning code
> would have to be written anyway. It is the same criterion by which the bar of `\frac` is a stroke. That is
> why there are `\hat` and `\vec` and not the whole family: `\bar`, `\tilde` and `\dot` will arrive when a
> figure asks for them.

### The symbols written `\command`

There are **110**, and this is the full list. They can be written inside or outside `$…$`. The sheet
[`examples/symbols.mg`](../examples/symbols.mg) shows them drawn, one by one.

**Greek letters** (41) — lowercase, their variants, uppercase, and ħ:

```
\alpha \beta \gamma \delta \epsilon \zeta \eta \theta \iota \kappa \lambda \mu
\nu \xi \pi \rho \sigma \tau \upsilon \phi \chi \psi \omega
\varepsilon \vartheta \varpi \varrho \varsigma \varphi
\Gamma \Delta \Theta \Lambda \Xi \Pi \Sigma \Upsilon \Phi \Psi \Omega
\hbar
```

**Operators and relations** (34):

```
\int \prod \sum \partial \nabla \surd \pm \cdot \times \div \oplus \otimes \oslash
\wedge \vee \cap \cup \diamond \bullet \sharp \neg
\leq \geq \neq \approx \cong \equiv \sim \propto \mid \in \ni \colon \angle
```

**Sets, arrows and delimiters** (23):

```
\subset \supset \subseteq \supseteq \forall \exists \therefore \bot
\leftarrow \rightarrow \leftrightarrow \Leftarrow \Rightarrow \Leftrightarrow
\uparrow \downarrow \Uparrow \Downarrow
\langle \rangle \lceil \rceil \lfloor \rfloor
```

**Others** (12):

```
\aleph \wp \Re \Im \infty \prime \textdegree
\clubsuit \diamondsuit \heartsuit \spadesuit
```

> ⚠️ A name not in this list **does not abort compilation**: it warns on the error output (`Warning: symbol
> name unknown alfa`) and **discards the symbol**, so the figure is generated with a gap where the glyph
> went. If a symbol is missing, the warning is in the terminal even though the `.svg` was written.

---

## 7. Expressions and control flow

```octave
n = 60
r = 2 * sqrt(n) / 3
caption = "v = " + str(r, 2)           % concatenation with +
xs = [0, 1.5, 3, 4.5]                  % list
first = xs[0]
```

**Functions:** `sin` `cos` `tan` `asin` `acos` `atan` `atan2(y,x)` `sqrt` `abs` `exp` `ln`
`mod(a,b)` `deg(radians)` `rad(degrees)` `len(list)` `str(x)` `str(x,decimals)` `gray(g)`.
Constants: `pi`, `true`, `false`.

The trigonometric functions work in **radians**, while the angles of the primitives (`arc`,
`rotate`, `view3d`…) are in **degrees**: `deg` and `rad` are the bridge, and it is worth using
them instead of multiplying by `180/pi` by hand — they carry the same constant the compiler
does, so an angle you write and one it computes cannot drift apart in the last digit.

With `asin`/`acos` you solve the angles of a geometric construction **inside the `.mg`** —where
to trim an arc, for instance ([§4](#4-primitives))— instead of measuring them on the drawing.

> ⚠️ **`sqrt`, `asin`, `acos` and `ln` ABORT outside their domain**, saying which value they
> got. This is deliberate: out of range they do not give an odd result, they give `NaN`, and a
> `NaN` that enters a coordinate propagates silently all the way to the output file. If your
> construction can step outside through rounding, or because the limiting case is legitimate
> —an arc that ends up entirely hidden—, clamp the value yourself first: that is what
> `angulo_solido` does with its clamp to [−1, 1], and that saturation **is** the geometry, not
> a patch.

**Operators:** `+ - * / ^`, comparison `== != < <= > >=`, logical `and` `or` `not`.

```octave
for i = 0 to n-1 {
    x = i/n
    polyline { (x) (x*x)   ((x+1/n)) (((x+1/n))*((x+1/n))) }
}

if r > 2 and n < 100 { text("large") { 0 0 } } else { text("small") { 0 0 } }
```

`for` accepts `step`: `for t = 0 to 1 step 0.05 { … }`.

> ⚠️ **In a `{ }` block values are separated by spaces**, so a `+` or `-` inside a coordinate splits it
> in two: `{ 12 y-11 }` is **three** terms (`12`, `y`, `-11`); what you want is `{ 12 (y-11) }`.
> **Parenthesize any coordinate that adds or subtracts**; products, quotients, powers and a leading minus
> go bare (`x*2`, `x/n`, `x^2`, `-x`). Mixing bare variables with parenthesized coordinates is fine:
> `{ x y (x+1) (y+1) }` ([details](#15-common-mistakes)).

> ⚠️ **A function call is GLUED to its parenthesis: `f(x)`.** With a space in between —`f (x)`— the `(`
> is a separate term, not a call; that's why in `{ x y (x+1) }` the `y` doesn't swallow the following
> parenthesis. Always write calls without a space: `sqrt(n)`, `point_at(&p, 0.5)`.

> ⚠️ **A list literal cannot be indexed.** `[10,20,30][1]` is a syntax error; you must go through a variable
> (`xs = [10,20,30]` and then `xs[1]`).

### All the ways to repeat

There is **a single general loop**, `for`. The rest are constructs that repeat something specific, and what
distinguishes them is not *what* they repeat but **where the copies go**:

| construct | repeats | where the copies go |
|---|---|---|
| `for v = a to b [step s] { }` | whatever you write in the body | wherever the body says — you compute it |
| coordinate block `{ p1 p2 … }` | one **primitive** | one instance per point |
| `place(Struct) { p1 p2 p3 … }` (§8) | one **struct** | one instance per point (3 or more), oriented to the tangent |
| `place(Struct, count=N) { p1 p2 }` (§8) | one **struct** | N spread evenly between the two points |
| `repeat(Struct, count=…)` (§8) | one **struct** | progression, each copy **relative to the previous** |
| `numbers`, `ticks` (§11) | one label / one mark | progression `at` + `advance`·k |
| `sample(&p, n)` (§10) | — | repeats nothing: **produces** n points |

💡 **A coordinate block is already a loop.** `dot(2) { 0 5  1 5  2 5 }` is three points, `circle(0.4) { c1 c2 }`
two circles, `text("×") { p1 p2 }` the same string stamped twice, `polybar(width=0.4) { … }` one bar per
point. **All** primitives work this way, and you don't have to do anything for them to. With **three or
more** points, `place` is exactly that for **structs**, which don't fit in a coordinate block — they are not
two ideas, they are one with two names, and the separate name exists because a struct is not a primitive.

> ⚠️ **With TWO points `place` is another thing: a guide line with something on top, and it draws the
> line.** To seed copies, give 3 or more points, or use `count=` ([details](#15-common-mistakes)).

💡 **What justifies `repeat` is accumulation.** It is the only one that composes the transform: with
`transform=rotate(30)` copy *k* is turned 30°·*k* from the previous one, and out of that come fans and
spirals. Without accumulating, a `for` that invokes the struct does the same — and reads better. If you're
not accumulating, use `for`.

💡 **`sample` is not iteration**: it doesn't draw instances, it returns a path of *n* points spread by arc
length. It produces **data**, and whoever draws it is the primitive you pass it to.

> ⚠️ **`to` is inclusive; `count` is a quantity.** `for i = 0 to 4` gives **five** turns (0,1,2,3,4) and
> `repeat(…, count=4)` gives **four** copies. It's the house off-by-one; the constructs that count
> (`repeat`, `numbers`, `ticks`) use `count`, and only `for` uses `to`.

---

## 8. Structures

A `struct` groups elements and is placed, scaled, rotated and repeated as a unit. Its body lives in a **unit
box** that is mapped to wherever you invoke it.

```octave
struct Frame(side = 1) {
    circle(side/2) { 0 0 }
    polyline(closed=true) { -1 -1  1 -1  1 1  -1 1 }
}

Frame()                                    % in its natural place
Frame(at=(3,2), scale=0.5, rotate=30)      % placed
for i = 0 to 11 { Frame(rotate=i*7.5, scale=1+i*0.35) }
```

Placement arguments: `at=(x,y)`, `scale=`, `rotate=` (degrees), `transform=`.

**Path-typed parameters** — marked with `&` in the declaration:

```octave
struct Level(&wave, w = path_width(&wave)) {
    world_window 0 w -2 2
    bezier(&wave)
}
Level(&pw3)
```

**Place and fit:**

```octave
place(Frame) { 0 0  3 0  3 3 }             % 3+ points: one instance per point
place(Frame, count=5) { 0 0  4 0 }         % 2 points: 5 spread evenly
place(Frame, gap=0.5) { 0 0  4 0 }         % 2 points: guide line WITH a gap, 1 instance
fit(Frame) { 1 1  4 3 }                    % fitted to that rectangle
fit(Frame, stretch=true) { 1 1  4 3 }      % stretching (otherwise MEET, centered)
repeat(Frame, count=6, at=(0,0), advance=(1.2,0), rotate=15)
```

> The order of `fit`'s corners **matters and reflects**: `{ .5 0  0 1 }` mirrors in x.

> ⚠️ **`place`'s locus is written literally**: today it takes no named path, neither in the block
> (`place(P) { &data }`) nor as an argument (`place(P, &data)`). It is the exception to the rule in §3 —the
> `&` works in the drawing primitives, not here—, and to seed a struct over computed points you have to
> write them inside the braces.

**Recursion** — a struct can invoke **itself**, and that's where a short definition produces a figure you
couldn't draw by hand. The stopping condition is an `if`:

```octave
struct tree(theta, phi, n, s) {
    polyline { 0 0  0 0.5 }                          % the trunk
    if n > 0 {                                       % <- the stopping condition
        { translate 0 0.5  scale s  rotate theta   tree(theta, phi, n-1, s) }
        { translate 0 0.5  scale s  rotate phi     tree(theta, phi, n-1, s) }
    }
}
tree(28, -28, 8, 0.6)        % 511 segments from a four-line struct
```

A trunk and, at its tip, two smaller copies of itself. Each branch goes in **its own block** because
`rotate`/`scale` are scoped state (§9): so the second doesn't inherit the first's rotation. Changing the two
angles changes the whole tree ([`examples/fractal_tree.mg`](../examples/fractal_tree.mg) draws two with the
same struct).

`max_depth n` sets the expansion ceiling; **32** by default. It counts **nesting, not invocations**: a
thousand copies placed side by side don't touch it, and it applies to all four ways of invoking (direct,
`place`, `fit`, `repeat`).

> `max_depth` is the **net**, not the brake. The brake is the `if`. A recursion without a stopping condition
> doesn't draw "as far as it reaches": it aborts with the depth error, naming the struct.

---

## 9. Transformations

Statements that affect what follows in the block, in writing order:

```octave
{ translate 3 2   rotate 30   scale 2 1   shear 0.4 0
  Frame() }
```

Also as a per-primitive or placement argument: `polyline(transform=rotate(30)) { … }`,
`Frame(transform=scale(2))`. (One constructor; several juxtaposed only in `repeat`.)

> Under a `transform`, text moves its **anchor**; the glyphs are not deformed (except `rotate`, which does
> turn them).

> ⚠️ **`scale` with two VARIABLE factors: parentheses on the second one if another statement follows on the
> same line.** `scale` is the only transformation of variable arity (one or two factors), so a bare
> identifier behind it is ambiguous: it may be its second factor or it may be the start of the next
> statement. The rule is that it is taken as a factor only if it **ends the statement**; to force it, use
> parentheses, since no statement begins with `(`: `{ scale sx sy }` ✅ (`sy` ends the statement, it is
> taken) and `{ scale sx (sy)   shear kk 0 }` ✅ (with another statement behind, parentheses).
>
> Without them, `sy` starts a new statement and swallows what follows; the error talks about what came
> after —"undefined variable: shear"— and points at a line that is written correctly. With literals
> (`scale 2 1`) there is no ambiguity and nothing is needed.

A `rotate`, a `scale` with different factors in x and y, or a `shear` applied to a `circle`, an
`arc` or an `ellipse` yield the **rotated** ellipse they should, with their angles intact: the
whole figure is transformed, not its radii separately. This holds in all three output formats.

> ⚠️ **`rotate` turns the plane, not the figure about its own centre.** If the centre is not at the
> origin, rotating displaces it. To turn something in place, bring the origin to the centre first
> and draw there: `{ translate 0 cy   rotate 15   ellipse(a, b) { 0 0 } }`. This is the trap that
> sends two concentric ellipses drifting to opposite sides.

---

## 10. Path algebra

A path is not only written: it is **operated on**. Everything in this section takes paths and returns
another path (or a measure of one), so it nests and composes. None of this is needed to draw —§3 is enough—,
but it is what lets you build a curve from others instead of typing its points.

**Operations** — produce another path, so they nest:

| | |
|---|---|
| `concat(a, b, …)` | chains, in that order — as many as you like, and **without reversing them on their own** (to orient, `reverse`) |
| `reverse(p)` | reverses the order of the points |
| `flip_x(p)` `flip_y(p)` | mirror |
| `transpose(p)` | swaps x↔y |

```octave
path half = { 0 0  1 2  2 2  3 0 }
path full = concat(reverse(flip_x(&half)), &half)   % a symmetric profile
bezier(&full)
```

**Generators** — build a path from parameters, not from written points:

```octave
path wave  = sine(half_cycles=2, amplitude=1) { 0 0  4 0 }   % half-cycle between two ends
path curve  = smooth { 0 0  1 2  3 1  4 3 }                   % passes through those NODES
```

> ⚠️ **A `{ }` block is always literal, and the generators take nothing but a block.**
> `smooth { &nodes }` is invalid —inside the braces go coordinates, not paths— and
> `smooth(&nodes)` is invalid **too**: `smooth` and `sine` require their points written right
> there. The parenthesized `&` form belongs to the primitives that CONSUME a path
> —`polyline(&p)`, `polygon(&p)`, `bezier(&p)`, as in §3—, not to the ones that generate it.
> Smoothing a path you already hold in a variable **is not possible today**; its nodes have to
> be written literally.

> ⚠️ **To FILL a generated curve use `bezier(&p, fill=)`, not `polygon(&p, fill=)`.** `sine` and
> `smooth` return Bézier **control** points, and `polygon` reads them as vertices: the fill comes
> out with straight corners —a kite instead of the lobe— and nothing warns you, because both are
> legitimate readings of the same list of points. `bezier` interprets them as what they are, and
> the fill closes itself along the chord joining the ends. It is what
> `onda_electromagnetica` does to paint each half-cycle.

**Accumulate in a loop** — for a curve whose number of pieces depends on a variable, something `concat`
doesn't cover, because its pieces must be written one by one:

```octave
path w = { 0 0 }
for k = 0 to n {
    path w += sine(half_cycles=1, phase=90, amplitude=amp) { (k) 0  (k+1) 0 }
}
```

> ⚠️ **`+=` WELDS relative pieces**: each is translated to continue where the previous ended, so they are
> written relative, not absolute ([details](#15-common-mistakes)).

**A computed curve, point by point.** It is the most common case in the language —the curve comes from a
formula, not from measured coordinates— and has its own form, because a **single-point piece is added
as-is, in absolute coordinates**:

```octave
A = 1   tau = 4   T = 1.5   n = 200

path wave = { 0 (A) }                       % seed: the first point, literal
for i = 1 to n {
    t = i * 10 / n
    path wave += { (t) (A*exp(-t/tau)*cos(2*pi*t/T)) }
}
polyline(&wave)
```

Each turn computes its point and adds it; at the end the path is drawn **once**, with whatever primitive you
want (`polyline` for a polygonal line, `bezier` or `smooth` for a curve). It's what
[`examples/tiro_parabolico.mg`](../examples/tiro_parabolico.mg) does.

> 🔑 **The two rules of `+=`, which are not the same:**
> - A **single-point** piece → added **absolute**. It's the form above, that of a sampled curve: you compute
>   each point in the figure's system.
> - A **two-or-more-point** piece → the first point is an **anchor**: it attaches to the end of what's
>   accumulated and is not duplicated, and the rest are **offsets from it**. The anchor's value doesn't
>   matter: `+= { 5 5  6 6 }` and `+= { 0 0  1 1 }` produce exactly the same. That's why multi-point pieces
>   are written starting at `{ 0 0 … }`.

> ⚠️ **A `for` CANNOT go inside a coordinate block.** `polyline { for i = … }` is a syntax error: the `{ }`
> block is a list of coordinates, not a body of statements. To generate points with a loop, accumulate them
> in a `path` as above.

> ⚠️ **`path x = …` is evaluated at DRAW time; `path x += …` on the spot.** That's why the seed of an
> accumulator must be a literal, with no variables the loop will overwrite ([details](#15-common-mistakes)).

**Path→number reductions** — read a measure from a path: `path_width(&p)`, `path_x_min_at_y(&p, y [, expand])`,
`path_x_max_at_y(&p, y [, expand])`. They operate on the control polygon, so they are exact on monotone
paths and approximate on a genuinely curved bezier.

**Sampling** — read geometry from a path at a parameter `t ∈ [0,1]`, traversed by **arc length** (so
`t = 0.5` is the *geometric* middle, not half the segments):

<!-- mg-noexec: signatures, not code: the brackets mark the optional argument -->
```octave
sample(&p, n [, curve=b])       % n points equally spaced by arc → a PATH
point_at(&p, t [, curve=b])     % the point at t → [x, y]
angle_at(&p, t [, curve=b])     % angle (degrees) of the tangent at t → number
```

The **`curve`** argument (named or positional) sets how the path is interpreted: `false` (default) treats
the points as **vertices** (linear interpolation — exact for a polyline; on a bezier it touches the *control
polygon*, not the curve), `true` treats them as **bezier controls** (evaluates the curve — touches the real
*curve*). Typical uses:

```octave
polyline(sample(&curve, 60, curve=true))          % densify a coarse bezier
dot(sample(&curve, 8, curve=true), size=2)        % 8 markers spread by arc
Mark(at=point_at(&curve, 0.5, curve=true))        % place a struct at the middle
marker(shape="arrow",
       marker_orient=angle_at(&curve, 0.5, curve=true)) { point_at(&curve, 0.5, curve=true) }
```

The last line shows the two ways to use the point: in a **struct**'s `at=`, or **directly in a primitive's
`{ }` block** (§4: the block accepts a point where a pair of scalars would go). A marker is oriented with
`marker_orient=` (degrees), not with `rotate=`.

---

## 11. Charts

`plot` maps **data units** to a physical box and draws its content inside:

```octave
plot(x=(0,10), y=(0,100), box=(0,0, 9,4.5), frame=true) {
    polyline { 0 0  1 1  2 4  3 9  4 16  5 25 }
    marker(size=4, shape="cross") { 0.9 10  2.5 15 }

    xaxis(step=2, label="x")
    yaxis(step=25, label="$y = x^2$", grid=true, grid_dash="dashed")

    rule(x=3, color="red", dash="dashed", label="threshold", label_at="legend")

    legend(at="top-left", font_size=8)

    table(at="top-right", col_widths=(20,30), decimals=3, label_col=true) {
        row("Mean", 4.21)
        row("SD",   0.87)
    }
}
```

**`plot`**: `x=(from,to)`, `y=(from,to)`, `box=(x0,y0,x1,y1)` (world; default = the window),
`xscale`/`yscale="log"`, `grid=`, `grid_dash=`, `frame=`.

**Axes** (`xaxis`/`yaxis` inside `plot`, or standalone `axis` with its two-point block):

| | |
|---|---|
| grid | `step`, `start`, `ticks` (`"out"`/`"in"`/`"both"`/`"none"`/`"grid"`), `tick_size`, `minor` |
| tick labels | `tick_labels` (true/false), `decimals`, `strip_zero`, `tick_label_gap`, `tick_label_size`, `tick_label_font`, `tick_label_align`, `tick_label_valign` |
| axis name | `label`, `label_at` (`"center"`/`"start"`/`"end"`), `label_gap`, `label_size`, `label_font` |
| data range | `from`, `to` (inside a `plot` inherited from `x=`/`y=`; on a standalone `axis` you set them) |
| geometry and style | `base=`, `extend=`, `scale="log"`, `field=`, `color`, `line_width`, `dash`, `grid`, `grid_dash` |

> **The nomenclature, which is what everyone uses:** `label` is the **axis name** (matplotlib's `xlabel`)
> and `tick_labels` are **the numbers on the marks**. `title` is reserved for the plot's heading.

**`rule`** — the notable value (a threshold, a level), distinct from the regular grid: `x=` or `y=` (one of
the two), `to=`, `label=`, `label_at` (`"axis"`/`"legend"`), `color`, `dash`, `line_width`. **Without `to=`
the line crosses the whole box**, which is usually what's wanted; `to=` cuts it at that data value.

> ⚠️ **"Nonlinear" doesn't mean "log".** The log scale is for *multiplicative* data and doesn't exist at
> values ≤ 0; if your points are just poorly spread, what you want are guide lines, not a scale
> ([details](#15-common-mistakes)).

**With a logarithmic axis, `plot` maps positions, not shapes.** A linear axis wraps the content in a
matrix and transforms all of it; log cannot —it is not an affine transformation— so it remaps
**coordinate by coordinate**. The consequence is a rule, not a corner case: whatever is not a
coordinate stays as you wrote it, in page units. That already held for `line_width`, the type and
the radius of `dot`; it holds just the same for the **radius** of `circle`/`arc`/`ellipse` and the
`width` of `polybar`.

```octave
plot(x=(1,1000), y=(0,10), xscale="log") {
    circle(0.5) { 10 5 }     % circle of radius 0.5 ON THE PAGE, centred on the datum (10,5)
    dot(3)      { 10 5 }     % the right way to mark a datum: physical, 3 pt
}
```

> ⚠️ **The same `circle(0.5)` measures different things depending on the axis.** With a linear axis
> the matrix transforms it along with everything else (and in an anisotropic frame it comes out an
> ellipse); with a log axis it keeps its page size. This is not an oversight: under a logarithm a
> circle of data is neither a circle nor an ellipse —it is an asymmetric egg—, so there is no
> "correct" shape to draw. To mark data use `dot`, which is physical and therefore behaves the same
> on both routes.

**`legend`** — `at=` combines `"top"`/`"center"`/`"bottom"` with `"-left"`/`"-right"`; plus `margin`,
`sample_width`, `sample_height`, `gap`, `row_gap`, `font_size` (all in pt). With a block, each entry
declares its sample; **without a block**, the entries are set by the `rule`s with `label_at="legend"`:

```octave
legend(at="top-right") {
    entry("Theory")     { polyline { 0 0.5  1 0.5 } }
    entry("Experiment") { marker(3, shape="cross") { 0.5 0.5 } }
}
```

**`table`** — `col_widths=(…)` in pt (mandatory; its size sets the number of columns), `row_height`,
`decimals`, `align`, `border`, `fill`, `label_col`, `label_font`, `font_size`, `margin`. `at=` accepts a
**named corner** (inside a plot) or a **point `(x,y)`** (outside).

### Standalone generators

`numbers`, `ticks`, `axis` and `grid` also work **outside** a `plot`, in world coordinates. The first three
carry no mapping: you place the first one and say how much each step advances, with the pair `at=` (where it
starts) and `advance=` (how much it moves per step). It is the compact mold of which `axis` is the
self-assembling version.

```octave
numbers(from=0, by=0.1, count=10, decimals=1, at=(0.5, 0.2), advance=(1, 0))
ticks(10, mark=(0, 0.15), at=(0.5, 0.35), advance=(1, 0))
grid(xstep=1, ystep=0.5) { 0 0  10 5 }
axis(from=0, to=100, step=25, label="v") { 0 0  10 0 }
```

| | |
|---|---|
| **`numbers`** | `from` (first **value**), `by` (how much it grows), `count`, `decimals`, `prefix`, `suffix` (strings that wrap the number), `at`, `advance` |
| **`ticks`** | `count` (also positional: `ticks(10, …)`), `mark=(dx,dy)` (the segment drawn at each mark, and its direction), `at`, `advance` |
| **`grid`** | `xstep`, `ystep` over the rectangle of the block `{ lower-left  upper-right }`, inclusive at both ends |
| **`axis`** | everything in the table above; the block is **two points** (the axis endpoints) and the data range goes in `from=`/`to=` |

`numbers`' labels inherit the current text state (`font`, `font_size`, `align`, `color`), like a `text()`.

> ⚠️ **Under a log scale**, placing structs inside the content is an error (their matrix doesn't compose
> with a nonaffine mapping), as are bare `grid()`/`ticks()`/`axis()`: use `grid=` and `xaxis`/`yaxis`.

---

## 12. Libraries

A **library is a `.mg` of structs** brought in with `include`:

```octave
include "pseudo3d.mg"      % relative to the file that includes it
prisma(2, 1, 1.5)               % width, height, depth
```

The `include` must precede the use, and **compilation fails** if the file doesn't resolve. In `lib/` come
`pseudo3d.mg` (pieces placed in the camera's scene of [§13](#13-pseudo-3d-scenes):
`prisma` and `lamina`, of flat faces, and `cono` and `cilindro`, whose **silhouette is computed**), two **icons** —`satellite.mg` (`struct Satellite`) and `sun.mg` (`struct Sun`, the Sun: a
radius-1 disc plus a ring of rays, `rays=` how many)— and three **world maps** in orthographic
projection, generated from real data (Natural Earth) with `tools/geo2mg.py`: `polar_map.mg`
(`PolarMap`, seen from the north pole), `fulldisk_map.mg` (`FullDiskMap`, equatorial) and
`mapa_p30_n55.mg` (`Mapa`, lat 30, lon −55).

All three are ordinary structs normalised to **radius 1**, so they are placed like any other one
—`scale` is the globe's radius in world units— and they take `grid=false` to drop the graticule and
`limb=false` to drop the outline of the disc, plus `ocean=`/`land=`/`grid_color=`/`grid_width=`:

```octave
include "../lib/mapa_p30_n55.mg"
Mapa(scale=5, at=(0, 0.5), grid=false)      % globe of radius 5 centred at (0, 0.5)
```

The pieces of `pseudo3d.mg` are placed in **scene coordinates**, with `pos=`, and live under the current
`view3d`. In `cono` and `cilindro`, `axis` is the vector going from `pos` to the other end: it carries both
direction and length, so there is no height parameter. `prisma` also takes `ex`/`ey`/`ez`, the directions of its
three edges, to set it **tilted**: they are given as vectors —the same style in which `plane3d` takes `u`
and `v`— rather than as an angle, because an angle forces you to choose what to rotate about, and that
depends on the figure.

```octave
include "../lib/pseudo3d.mg"
view3d(azimuth=-25, elevation=22)
prisma(1.2, 0.8, 1.0, pos=[0, 0, 0])
g = rad(14)                                  % the same box, tilted
prisma(1.2, 0.8, 1.0, pos=[0, 1.4, 0], ex=[cos(g), sin(g), 0], ey=[-sin(g), cos(g), 0])
cono(0.9, axis=[0, 3.2, 0], pos=[3.5, 0, 0])
cilindro(0.85, axis=[2.4, 1.3, -0.6], pos=[6.5, 0.6, 0])
```

> They are **wireframe**: the edges are drawn, the body is not filled. And a cone's silhouette requires the
> apex to fall **outside** its projected base; if it does not, you are looking at the cone from the inside
> and only the base is drawn.

**Where `include` looks:** first **next to your file** (relative path), and then in the **installed
library** (`make install` copies `lib/*.mg` to `$PREFIX/share/metagrafica/lib`). Local **overrides**
installed. That's why `include "satellite.mg"` alone uses the system lib, and in the repo the relative path
is written (`include "../lib/satellite.mg"`).

---

## 13. Pseudo-3D scenes

MG draws in the plane. What this section adds is a **camera**: you declare it once and then describe the
figure in coordinates of space, instead of working out by hand where each thing lands on the paper.

What you gain is not syntax, it is that **the camera becomes a parameter**: change the elevation and the
grid, the rays and the solid pieces all move together, because they all come from the same number. It is
the opposite of placing each piece until it looks right.

> ⚠️ This is a **simulation** of 3-D in 2-D: there is no z-buffer, no hidden surfaces, no lighting and no
> perspective. Paint order is **writing order** —draw the far things first—, and which face of a body ends
> up behind is your decision. For real 3-D, Blender; MG is not going to replace it.

### What to reach for

One of these figures is almost never *one* thing: it is usually three or four kinds of piece in the same
scene. It pays to look at it part by part and ask, of each one, which of these it belongs to:

| what you want to draw | with what | worked example |
|---|---|---|
| a circle, an arc or a curve that **lives in a plane** of space | `plane3d`, and inside it you draw in 2-D | `angulo_solido` |
| a **filled curve**, a wave, markers on it | `plane3d` + the usual primitives | `onda_electromagnetica` |
| a **ray**, a dimension line, a label hanging off a point: something belonging to no piece | `xyz()` | all three |
| the **silhouette** of a cone or a cylinder | computed in the `.mg`; the recipe is in the header of `irradiancia` | `irradiancia` |
| a **box**, a plate, a **cone** or a **cylinder** | `prisma` `lamina` `cono` `cilindro` from `lib/pseudo3d.mg` ([§12](#12-libraries)) | — |
| an **angle arc**, an arrow, a freehand outline | nothing special: it is drawn on the paper | `irradiancia` |

**Where to start.** If your figure is mostly circles or curves placed on planes, read `angulo_solido`: it
is the pure case and uses nothing else. If you also need fills or markers, `onda_electromagnetica`. And if
a round body shows up, `irradiancia`, the only one that computes a silhouette.

> ⚠️ **Two warnings about the last row.** The arc marking an angle is annotation on the **paper**, not on
> the scene, because the angle between two projected directions is **not** the projection of the angle in
> space. If you want it to measure for real, draw it in the plane containing both directions —which is what
> `irradiancia` does with its φ— and then the sweep of the arc *is* the angle.

### The axes

**x to the right, y up, z towards the viewer.** The plane of the paper is **xy**; the ground, if the figure
has a ground, is **xz**, and a height is `y`.

The convention was chosen so that `view3d(azimuth=0, elevation=0)` is the **identity**: with no camera —or
with the camera head-on— the language behaves exactly as in the rest of this reference, and a flat figure
pays nothing for this existing.

### `view3d` — the camera

A state statement with scope, like `translate` or `rotate` ([§9](#9-transformations)): it holds from where
it appears to the end of the block. There are two projections and **a single statement**, with `type=`: a
figure declares *one* camera.

```octave
view3d(azimuth=35, elevation=25)                     % axonometric — the default
view3d(type="oblique", angle=45, foreshorten=0.5)    % oblique (cavalier/cabinet)
```

**Orthographic axonometric** (`type="axonometric"`, or nothing) — azimuth θ, which spins the scene about
the vertical, and elevation φ, which lifts the camera above the horizon:

    X = x·cos θ + z·sin θ
    Y = y·cos φ + (x·sin θ − z·cos θ)·sin φ

The cases worth keeping at hand to orient yourself: θ=φ=0 is the identity; θ=0, φ=90° gives the **plan**
view (seen from above); θ=90°, φ=0 gives the view from +x.

**Oblique** (`type="oblique"`) — the front face, the one at `z=0`, **always keeps its true shape**,
whatever `angle` is; only depth recedes, in the direction `angle` and shortened by `foreshorten`
(1 = cavalier, 0.5 = cabinet):

    X = x − z·f·cos(angle)
    Y = y − z·f·sin(angle)

It is the projection of textbook figures where one face has to be measurable. Defaults: `azimuth=0
elevation=0`; `angle=45 foreshorten=0.5`. Angles are in **degrees**.

### `plane3d` — drawing on a plane of the scene

A statement with scope, like the transformations: inside the block, coordinates are **local to the plane**
and ordinary drawing projects itself.

```octave
plane3d(at=[0,0,0], u=[1,0,0], v=[0,1,0])   % the defaults: the plane of the paper
```

`at` is the local origin, and `u` and `v` are the two vectors of the plane, which **carry the scale**:
local `(0,0)` lands on `at`, `(1,0)` on `at+u` and `(0,1)` on `at+v`.

```octave
view3d(azimuth=35, elevation=25)

% an upright screen, and a ring drawn ON it
{ plane3d(at=[8,0,0], u=[0,0,1], v=[0,1,0])
  rectangle { 0 0  1.4 4.2 }
  circle(0.6) { 0.7 2.1 } }

% the meridians of a sphere: seven ellipses in two lines
for k = 0 to 6 {
    lam = k * pi / 7
    { plane3d(u=[cos(lam), 0, sin(lam)], v=[0, 1, 0])
      circle(2) { 0 0 } }
}
```

📌 **That `circle` comes out as the exact ellipse of its projection**, not as an approximation nor as an
ellipse someone had to measure: the engine represents an ellipse by its centre and its conjugate
semi-diameters, which is precisely the closed form of projecting a circle of space. It holds equally in
EPS, SVG and PDF.

📌 **Inside a `plane3d` the usual 2-D drawing works** —`sine`, `smooth`, `bezier`, a filled `polygon`,
`place` of a struct—, because the only thing that changed is the current matrix. A wave on a plane of
space is an ordinary `sine`; nothing has to be sampled by hand. And the `from`/`to` of an `arc` is still
an angle **of the plane**, not of the page.

### `xyz(x, y, z)` — a loose point of space

For what belongs to no plane: rays, construction lines, the label hanging off a point. It returns the
point **already projected**, so it is written where a coordinate goes.

```octave
view3d(azimuth=35, elevation=25)
h = 6   d = 1.2

polyline { xyz(0, h, 0)   xyz(3*d, 0, 5*d) }     % a ray, from the optics to the ground
text("CIV") { xyz(3*d, 0, 5*d) }
```

> ⚠️ **`xyz()` goes OUTSIDE every `plane3d`.** The point it returns is already in document coordinates;
> inside a `plane3d` it would be transformed **twice** and land anywhere. Inside a plane you draw in local
> coordinates, which is exactly what makes `xyz()` unnecessary there.

### Solid pieces

`lib/pseudo3d.mg` ([§12](#12-libraries)) brings two, built on the above: `prisma(w, h, d, pos=[x,y,z])`, a
box of three visible faces shaded from light to dark, and `lamina(h, d, pos=…)`, the single-face plate
with hatching. Both are placed in **scene** coordinates with `pos=` — `at=` is still the placement word of
a struct ([§8](#8-structures)), and serves to shift the piece on the page *after* projecting it.

⚠️ The order in which `prisma` paints its faces is the right one for a viewer up and to the right
(positive elevation, or an oblique `angle` between 0 and 90°). With the camera on the other side you have
to reorder by hand: with no z-buffer that is **modelling**, and no library can guess it.

---

## 14. How the compiler fails

MG **aborts** rather than producing a half-figure: an evaluation error, an `include` that doesn't resolve or
an odd count of coordinates end compilation with code 1. **Non-fatal warnings** do continue, and go to
`stderr`: an unknown color (falls back to black), a character with no glyph (dropped), or a figure whose
entire drawing landed off the canvas ([§15](#15-common-mistakes)) — there the compiler has nothing to fix,
but you do have something to look at.

This is deliberate: **an inconsistent document should produce no output**. In a figure derived from
formulas, a physics error usually shows up as a compilation error — getting a Morse well's depth wrong makes
its turning point cease to exist, and what pops up is `ln: non-positive argument`.

**Compile in parts.** `exit`, on a line at the **top level** of the file, stops reading there: what follows
is ignored, including its syntax errors. It serves to raise a figure in stages without commenting out the
rest or keeping copies.

```octave
polyline { 0 0  1 1 }
exit                       % from here down, as if it weren't there

polygon { 2 2   3 2        % half-written: brace not closed, and one coord too many
```

> `exit` is a **read**-time thing, not a draw-time one: inside an `if` it would not be conditional (the
> condition is evaluated later), so nesting it is an error. And it doesn't stop a recursion — for that, the
> `if` of §8.

> ⚠️ **`exit` silences the syntax errors further down, but not the lexical ones** (a stray `@`, an accented
> letter outside a string) ([details](#15-common-mistakes)).

---

## 15. Common mistakes

The stumbles that recur, symptom first — because when they happen you don't yet know the cause. The first
three are, by far, the most frequent.

**Nothing shows, or half of it shows, or "only 3 points come out".** Almost always the data falls **outside
`world_window`**, which is a fixed crop and does not adjust to what you draw. If your curve lives in `y` from
9 to 15 and the window goes from 0 to 8, the compiler drew fine: off-frame. **There is no error**, and
that's why the symptom looks like an engine bug (an empty figure, an almost-blank EPS). Before suspecting the
engine, check that your data range fits in the window. It happens even to those who know the language.

> When **everything** falls outside, MG does warn (`la figura sale EN BLANCO`) and tells you, in cm, where
> the drawing ended up and where the canvas is. If it only sticks out in part, it stays quiet: running off
> one edge is often deliberate.

And the sibling case, which is not a crop but a **proportion**: the data fits in the window and the figure
still comes out as an unreadable strip. The engine is isometric ([§2](#2-the-canvas-and-the-units)), so a
`world_window 0 1 0 6` in a `display_size 12 8` uses the same scale on both axes —the one that makes the
long axis fit— and the whole x axis ends up 1.3 cm of the 12. There is no warning because nothing is wrong:
drawing that way is what you asked for. **The world window is for figures whose two axes share a unit** (a
plane, an orbit, a map). As soon as x and y measure different things, or their ranges differ by more than
two or three times, what you want is a [`plot`](#11-charts), which maps data to a box and stretches each
axis on its own.

**A coordinate `{ }` complains about an odd number, or the figure comes out distorted.** Inside a block
values are separated by **spaces**, so a `+` or a `-` inside a coordinate splits it in two: `{ 12 y-11 }` is
**three** terms (`12`, `y`, `-11`), not two — what you want is `{ 12 (y-11) }`. The rule: **parenthesize any
coordinate that adds or subtracts**; products, quotients and powers go bare (`x*2`, `x/n`, `x^2`), and bare
variables coexist with parenthesized coordinates (`{ x y (x+1) (y+1) }`). The odd count is a compilation
error, with line and column, so at least it doesn't fail silently. And a **function call is glued** (`f(x)`):
with a space, `f (x)`, the `(` is a separate term, not a call.

**`dot(2, &p)` doesn't compile and the error talks about an unexpected expression.** The path goes **always
as the first argument** and the rest named after: `dot(&p, size=2)`, `marker(&p, shape="x")`,
`polybar(&p, width=0.5)`. In the `{ }` block it doesn't work either.

**The text looks huge when you shrink the figure.** Text is a **physical** quantity (pt) and the panels are
a **world** quantity, so shrinking `display_size` doesn't shrink the text: it enlarges it relative to the
drawing, and the labels overlap. For a smaller figure, lower `font_size`; the canvas is not the lever.

**`polyline(…, outlinefill)` errors, and so does `polyline(align="center")`.** None of `outlinefill`,
`font`, `font_size`, `align` and `valign` is a generic style attribute: as an attribute in parentheses they
are only valid where they are the primitive's **own** arguments, that is in `text()` —`text("m",
align="center", font_size=9) { 5 1 }` is correct—. Elsewhere they go as a **state statement**. `outlinefill`
is not valid as an attribute on any primitive: the outline over a fill is requested by giving `color=`
alongside `fill=`, which already implies outlining.

**A curve accumulated with `+=` comes out shifted or overlapped.** `+=` **welds relative pieces**: each is
translated so its first point continues where the previous ended. That's why pieces are written relative
(`{ 0 0  … }`) and not with absolute coordinates — if you write them absolute expecting them to accumulate
as-is, the piece shifts.

**A `path` accumulator gives strange values inside a loop.** `path x = …` is **deferred**: it stores the
expression and evaluates it at draw time, so if the seed carries variables the loop overwrites, it will read
the final values. `path x += …` evaluates on the spot. The seed, therefore, must be a literal. And when
reassigning inside the loop you repeat the word `path`.

**`place` drew me a line I didn't ask for.** With **two** points `place` is not "one copy per point" but a
**guide line with something on top** —the good old labeled arrow— and it draws the line: `shift` moves the
instance, `both_sides=true` puts two, `gap=` splits the line to leave room for a label, and the shape over
an **arc** (`r=`, `from=`, `to=`) also draws it. To seed copies without a line: give **3 or more** points,
or use `count=` over the two.

**The logarithmic grid doesn't pass through my points.** "Nonlinear" doesn't mean "log". The log scale is
for **multiplicative** data —each step multiplies— and doesn't exist at values ≤ 0. If your points are
simply not regularly spaced (a parabola, a `1/r`, something crossing zero), no log grid will pass through
them. What you want is not an axis scale but **guide lines at the points**, drawn in the same loop that
generates the data. A regular grid and "lines where the points are" are different things — the same
distinction as between `grid=` and `rule`. See `examples/tiro_parabolico.mg`.

**"This `{` opens a scope block and its contents are statements, but it starts with a number".**
The coordinate block must start on the **same line** where the head of the primitive ended — the
`)` of the arguments, or the name if there are none:

<!-- mg-expect-error -->
```octave
rectangle(fill="red") { 0 0  4 3 }         % ✅
rectangle(fill="red")
    { 0 0  4 3 }                           % ❌ error, pointing at the 0
```

Inside there is no restriction: the arguments, a list and the coordinate block itself may be
split across lines and carry comments, as long as the `{` has already opened.

```octave
rectangle(gradient=["magenta",      % a list split across lines…
                    "red"]) { 0 0  4 3 }   % ✅ …but the { is still up there
polyline { 0 0
           4 4 }                           % ✅ an open block continues on the next line
```

**Why the compiler puts it that way:** a bare `{ }` **is a valid construct** — the scope block of
[§9](#9-transformations), as in `{ translate 3 2  Frame() }`. So it does not see a broken
primitive: it sees a primitive with no coordinates followed by a scope block full of numbers
where it expected statements. The error names that cause and gives you **the line to join up**.
Inside the body of a `for` or an `if` the same `{ 0 0 }` gets the other half of the diagnostic:
there is nothing to join there, and what you want is to accumulate the points into a `path` with
`+=` ([§10](#10-path-algebra)).

**I put `exit` and it still fails further down.** `exit` silences the later **syntax** errors —unclosed
braces, a misspelled primitive, odd coordinates—, which is the normal case of half-written code. It does not
silence the **lexical** ones: a character the language doesn't recognize (`@`, or an accented letter outside
a string) aborts anyway, because the file is turned into tokens **whole** before being read. A prose note
below the `exit` goes in a `%` comment.

> 💡 **`%` comments do admit accents** —and ñ, and anything: the lexer skips the whole comment without
> looking at it. The accent rule applies to the **code**, not to what you write about it.

**A loop runs one turn too many or too few.** `to` is **inclusive** and `count` is a **quantity**:
`for i = 0 to 4` gives five turns and `repeat(…, count=4)` gives four copies.

---

## 16. Quick reference

**Primitives** · `polyline` `polygon` `rectangle` `circle` `ellipse` `arc` `dot` `marker` `bezier` `smooth`
`polybar` `sine` `compound` `text`

**State statements** · `color` `fill` `line_width` `dash` `hatch` `hatch_gap` `outlinefill` `font`
`font_size` `align` `valign`

**Configuration** · `display_size` `world_window` `max_depth` `exit`

**Transformations** · `translate` `rotate` `scale` `shear`

**Pseudo-3D scene** · `view3d(azimuth=, elevation=)` / `view3d(type="oblique", angle=, foreshorten=)` ·
`plane3d(at=, u=, v=)` · function `xyz(x, y, z)`

**Control** · `for … to … [step …] { }` · `if … { } else { }` · `struct` (recursive) · `include`

**Placement** · `place` `fit` `repeat` · invocation `Name(at=, scale=, rotate=, transform=)`

**Paths** · `path x = …` · `path x += …` · `&name` · `concat` `reverse` `flip_x` `flip_y` `transpose` ·
generators `sine` `smooth` · reductions `path_width` `path_x_min_at_y` `path_x_max_at_y` · sampling
`sample` `point_at` `angle_at` (`curve=` argument)

**Charts** · `plot` `xaxis` `yaxis` `axis` `rule` `legend`/`entry` `table`/`row` `grid` `numbers` `ticks`

**Functions** · `sin` `cos` `tan` `asin` `acos` `atan` `atan2` `sqrt` `abs` `exp` `ln` `mod`
`deg` `rad` `len` `str` `gray` `xyz` · constants `pi`
`true` `false`

<!-- translated-from: referencia.md @ 932feb31dab2ce3bf75b6c98e11e08648c5ac399 -->
