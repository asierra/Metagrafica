# Computing instead of measuring

**English** · [Español](calcular_en_vez_de_medir.md)

Almost every figure in a physics book is **drawn**: someone decides where each curve and
each mark goes, and the result looks like what the physics says. This document is about the
other kind — the ones that are **computed**, where the geometry comes out of the formulas
and nobody places anything by eye.

MetaGráfica is not a data analysis tool and does not pretend to be. The boundary is
deliberate and worth stating up front: **MG computes the geometry of an illustration from
its model.** It does not reduce data. But **the package ships the bridges**. When the data
comes from a spreadsheet or a file of measurements, `tools/hist2mg.py` reads it with pandas,
computes the intervals and the statistics the figure will need, and writes a `.mg` you
include directly. And if you have older figures made with earlier versions of MG,
`tools/mg1to2.py` translates them to the current syntax.

The split is the one you would want: **reducing is one step, drawing is another**, and
either can be redone without touching the other. If more measurements arrive tomorrow, you
run the bridge again and the figure regenerates without having to be reopened.

What does happen inside is the other case, and it is the one this document argues for: when
a figure has a **model** behind it —a potential, a quantization condition, a turning point—
that model can live *in the figure's source file* instead of in the author's head or in a
script that gets lost.

---

## 1. A figure with parameters: Franck-Condon

`examples/franck_condon.mg` draws two Morse potentials with their vibrational levels and
their wave functions. **Nothing in it is measured.** You give five numbers per electronic
state and the rest is closed form:

```text
a1  = 1.8            % range of the potential
re1 = 1.15           % equilibrium distance
we1 = 0.56           % vibrational frequency
xe1 = 0.028          % anharmonicity
D1  = we1/(4*xe1)    % depth — NOT free, see §4
```

From those, without anyone placing them:

| what gets drawn | where it comes from |
|---|---|
| the curve | `V(r) = D(1 − exp(−a(r−re)))²` |
| the levels | `E_v = we(v+½) − we·xe(v+½)²` |
| where each wave starts and ends | `r± = re − ln(1 ∓ √(E/D))/a` (the turning points) |
| how many bound levels there are | `vmax = 1/(2·xe) − ½` |
| the amplitude of each lobe | semiclassical WKB envelope, `∝ (E−V)^(−¼)` |
| how far the wave reaches into the forbidden region | Airy scale, `∝ |V′(turning point)|^(−⅓)` |

**The proof is changing one number.** Move a single one —the anharmonicity `xe1`, from
`0.028` to `0.045`— and the whole figure rearranges itself coherently: the well gets
shallower (`D` goes from 5.0 to 3.1), the dissociation line drops with it, the levels spread
out and crowd together sooner, and the waves readjust to their new turning points. The
anharmonicity also decides **how many bound states the potential admits**: the last one goes
from v = 17 to v = 10, because `vmax = 1/(2·xe) − ½`.

| `xe1 = 0.028` | `xe1 = 0.045` |
|:---:|:---:|
| ![Franck-Condon with anharmonicity 0.028](img/franck_condon.svg) | ![The same figure with anharmonicity 0.045](img/franck_condon_anarm.svg) |

Both images come from the **same file**; one character changed between them.

None of those consequences is written in the file. The **formulas** are written, and the
figure is what follows from them.

**The detail that best sums it up:** the vertical Franck-Condon transition —the one the
principle is named after— lands on level v′≈6 of the excited state **without anyone putting
it there**. It comes out of the offset between the equilibrium distances of the two states
(`re1 = 1.15` against `re2 = 1.48`). Move the two wells closer and the transition shifts by
itself to the level it belongs on. That is the Franck-Condon principle *happening* in the
figure, not being illustrated in it.

---

## 2. The physics ties your hands, and that is the advantage

`examples/turning_points.mg` is a well with three energies: one bound state and two unbound.
You give the asymptotes of the potential, its minimum, the three turning points, the three
energies and the number of nodes of the bound state; out of those come `V(x)`, the
wavelengths, the amplitudes and the tails.

Two things that only happen when you compute:

**The curve passes through the turning points by construction, not by fitting.** The form
`V(x) = V∞ − (V∞−Vm)·exp(−|(x−xm)/w|^s)` has two free parameters on each side, and there are
exactly two conditions to impose on them (that the curve equal `E_b` at one turning point and
`E_c` at the other). They are solved for. Not fitted: solved. The labelled turning points
land on the curve because they cannot land anywhere else.

**The three waves are not independent.** The phase constant comes from the Bohr-Sommerfeld
quantization condition applied to the bound state's wave. Once it is fixed, the other two
have **no freedom at all**: it is the same particle, so it is the same constant. Changing the
bound state's node count from 3 to 4 drags all three along —the bound one goes from 3 to 4
lobes, and the other two from 23 to 30 and from 13 to 16— without touching anything else.

| `nodos = 3` | `nodos = 4` |
|:---:|:---:|
| ![Well with three energies and their wave functions, with 3 nodes](img/turning_points.svg) | ![The same figure with 4 nodes: all three waves get denser](img/turning_points_nodos.svg) |

One more node was asked for **in the bottom wave**, and all three got denser. That is not a
side effect: it is the same particle, and the quantization condition does not allow anything
else.

Drawn by hand, each of those waves is an independent aesthetic decision. Here it is a
consequence, and that is why they cannot contradict each other.

**That is why this example is not a faithful port of the published figure.** That one was
drawn by hand, as suited the tools of its time and the purpose of the figure, which is to
illustrate a concept: the wavelength was chosen region by region, so that each stretch would
read clearly. Writing it in derived form gives up that freedom —a single constant governs all
three waves— and so the two versions do not agree: where the bound wave has 4 antinodes,
quantization calls for something like 24 and 13 lobes for the other two, more than the drawing
carries. Reproducing the strokes and computing them were different goals; here computing won,
and that is why the example took a name from the physics (`turning_points`) instead of a
figure number.

---

## 3. Computing finds things measuring does not

`examples/fig4-4.mg` reproduces three potentials: the harmonic oscillator, the Coulomb one,
and the effective one with a centrifugal barrier.

![Three potentials with their classical turning points](img/fig4-4.svg)

In the original version those three curves were **digitized**: 69 points each, measured off a
drawing. On porting them it turned out the points fit exact analytic forms —`V = x²`,
`V = 1/r`, `V = 1/(2r²) − 1/r`— to within about `1e-6`. That is: the curves *always were*
those formulas; what had been preserved was a degraded copy, 69 numbers in place of one line.

Rebuilding them also turned up a detail that faithful reproduction would never have shown.
The original file placed one of the marks with `DOT 5 011 .3` —a misplaced decimal point that
sends it to `x = 454`, off the canvas— so that mark was never drawn at all. It is a trifle:
the figure reads just as well without it, and precisely for that reason it could have stayed
that way indefinitely. A missing mark leaves no trace.

What matters is not the slip but what it takes to see it. By deriving the positions from the
physics, the file **knows which marks ought to be there**, and one that fails to appear
becomes detectable. It is a check that only exists when the figure is computed, and it is the
kind of safety net MG puts under the author today: the corpus examples are recompiled and
compared on every change, so a slip like that has somewhere to surface.

---

## 4. The compiler understands more physics than you would think

The case that says the most about all this is a mistake made while writing
`franck_condon.mg`.

The depth of a Morse well **is not an independent parameter**: the relation
`D = we/(4·xe)` fixes it from the vibrational frequency and the anharmonicity, which are the
two quantities a spectrum actually measures. At first `D` was given separately, as if it were
free.

The result was not an ugly figure. It was a **compile error**:

```
Error de evaluación: ln requiere argumento positivo
```

With an inconsistent `D`, some levels end up above the dissociation energy, and then the
outer turning point `r₊ = re − ln(1 − √(E/D))/a` ceases to exist: the logarithm's argument
turns negative. The physics became impossible arithmetic, and the compiler said so and
stopped.

A drawing program would have produced a perfectly presentable figure with levels floating
above dissociation. And that, incidentally, is the strongest reason why an evaluation error
in MG is **fatal** rather than a warning: an inconsistent document should not produce output.

---

## What the figure gains from this

- **The derivation travels with the drawing.** There is no separate script that computed the
  numbers and got lost, no image whose origin has to be reconstructed. The file *is* the
  argument.
- **You can ask again.** "What if the anharmonicity were larger?" is answered by changing a
  number and recompiling, not by redoing the figure.
- **The parts cannot contradict each other.** If the levels, the turning points and the waves
  all come from the same parameters, there is no way for one to say one thing and another the
  opposite — which is the usual way a figure drawn piece by piece fails.
- **Errors become detectable.** Whether it is a lost decimal or an ignored Morse relation:
  what is derived can be contradicted, and being contradicted is what makes an error surface.
- **It is text.** It lives in version control like the manuscript of the paper, you can see
  exactly what changed between two versions and who changed it, and it regenerates twenty
  years from now without depending on the program it was drawn with still existing.

None of this demands that every figure be made this way. The MG corpus has faithful ports,
schematic figures and diagrams where there is no model to compute, and they are fine as they
are. But when there **is** a model behind it, writing it down costs the same as hiding it.
