% Patrones y anchos de línea — traducción V3 de examples/v1/line_patterns.mg
%
% ⚠ AÚN NO COMPILABLE (no hay parser V3). Fixture de spec; su render debe igualar
% examples/v1/reference/line_patterns.svg (salvo diferencias intencionales: los
% alias de dash V3 —§4.10— no son byte a byte los arreglos LPATRN de V1).
% Ejercita §4.10 (dash, width) y §7 (estado) — cubierto por la gramática actual.

display_size 14 7
font_size 8
world_window -1 13 -0.5 6.5

% marco
rectangle { -0.95 -0.45  12.95 6.45 }

% Patrones de línea (V1 LPATRN 0..5 → alias de dash, §4.10).
% dash por primitiva: cada línea usa uno distinto; width compartido como estado.
width 0.2
polyline(dash="solid")      { 0 0  5 0 }   % LPATRN 0 (continuo)
polyline(dash="solid")      { 0 1  5 1 }   % LPATRN 1
polyline(dash="dashed")     { 0 2  5 2 }   % LPATRN 2
polyline(dash="dotted")     { 0 3  5 3 }   % LPATRN 3
polyline(dash="dashdot")    { 0 4  5 4 }   % LPATRN 4
polyline(dash="dashdotdot") { 0 5  5 5 }   % LPATRN 5
text(0, 0.1) { "Pattern 0 (continuous)" }
text(0, 1.1) { "Pattern 1" }
text(0, 2.1) { "Pattern 2" }
text(0, 3.1) { "Pattern 3" }
text(0, 4.1) { "Pattern 4" }
text(0, 5.1) { "Pattern 5" }

text(1, 6.0) { "Line Patterns LPATRN" }

% Anchos de línea (V1 LWIDTH n → width = n*0.2 pt; LWIDTH 0 → hairline 0.1)
polyline(width=0.1) { 6 0  11 0 }   % LWIDTH 0 (el más delgado)
polyline(width=0.2) { 6 1  11 1 }   % LWIDTH 1
polyline(width=0.4) { 6 2  11 2 }   % LWIDTH 2
polyline(width=0.6) { 6 3  11 3 }   % LWIDTH 3
polyline(width=0.8) { 6 4  11 4 }   % LWIDTH 4
polyline(width=1.0) { 6 5  11 5 }   % LWIDTH 5
text(6, 0.1) { "Width 0 (the slimest)" }
text(6, 1.1) { "Width 1" }
text(6, 2.1) { "Width 2" }
text(6, 3.1) { "Width 3" }
text(6, 4.1) { "Width 4" }
text(6, 5.1) { "Width 5" }

align "center"
text(8.5, 6.0) { "Line Widths LWIDTH" }
size 6
text(8.5, 5.7) { "(units of 0.2 typographic points)" }
