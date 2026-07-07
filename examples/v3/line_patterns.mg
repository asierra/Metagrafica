% Patrones y anchos de línea — traducción V3 de examples/v1/line_patterns.mg
%
% ⚠ AÚN NO COMPILABLE (no hay parser V3). Fixture de spec; su render debe igualar
% examples/v1/reference/line_patterns.svg (salvo diferencias intencionales: los
% alias de dash V3 —§4.10— no son byte a byte los arreglos LPATRN de V1).
% Ejercita §4.10 (dash, line_width) y §7 (estado) — cubierto por la gramática actual.

display_size 14 7
font_size 8
world_window -1 13 -0.5 6.5

% marco
rectangle { -0.95 -0.45  12.95 6.45 }

% Patrones de línea (V1 LPATRN 0..5 → alias de dash, §4.10).
% dash por primitiva: cada línea usa uno distinto; line_width compartido como estado.
line_width 0.2
polyline(dash="solid")      { 0 0  5 0 }   % LPATRN 0 (continuo)
polyline(dash="solid")      { 0 1  5 1 }   % LPATRN 1
polyline(dash="dashed")     { 0 2  5 2 }   % LPATRN 2
polyline(dash="dotted")     { 0 3  5 3 }   % LPATRN 3
polyline(dash="dashdot")    { 0 4  5 4 }   % LPATRN 4
polyline(dash="dashdotdot") { 0 5  5 5 }   % LPATRN 5
% item 0 lleva anotación propia → text suelto; los uniformes 1..5, un numbers
text("Pattern 0 (continuous)") { 0 0.1 }
numbers(from=1, by=1, count=5, decimals=0, prefix="Pattern ", at=(0, 1.1), advance=(0, 1))

text("Line Patterns LPATRN") { 1 6.0 }

% Anchos de línea (V1 LWIDTH n → line_width = n*0.2 pt; LWIDTH 0 → hairline 0.1)
polyline(line_width=0.1) { 6 0  11 0 }   % LWIDTH 0 (el más delgado)
polyline(line_width=0.2) { 6 1  11 1 }   % LWIDTH 1
polyline(line_width=0.4) { 6 2  11 2 }   % LWIDTH 2
polyline(line_width=0.6) { 6 3  11 3 }   % LWIDTH 3
polyline(line_width=0.8) { 6 4  11 4 }   % LWIDTH 4
polyline(line_width=1.0) { 6 5  11 5 }   % LWIDTH 5
text("Width 0 (the slimest)") { 6 0.1 }
numbers(from=1, by=1, count=5, decimals=0, prefix="Width ", at=(6, 1.1), advance=(0, 1))

align "center"
text("Line Widths LWIDTH") { 8.5 6.0 }
font_size 6
text("(units of 0.2 typographic points)") { 8.5 5.7 }
