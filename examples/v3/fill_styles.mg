% Formas de rellenar áreas — traducción V3 de examples/v1/fill_styles.mg
%
% ⚠ AÚN NO COMPILABLE (no hay parser V3). Fixture de spec; su render debe igualar
% examples/v1/reference/fill_styles.svg.
% Ejercita §4.11 (hatch), §4 (gris con gray()/color/contorno), §13.2 (numbers)
% y §5–§6 (listas, indexado, operador %, for).
%
% FPATRN → hatch/hatch_gap es 1:1 (Display::patternFor = 4 ángulos × 3 densidades):
%   idx:  1    2    3    4    5    6    7    8    9   10
%   áng:  0   45   90  135    0   45   90  135    0   45
%   gap:  4    4    4    4    2    2    2    2    1    1
% El gap ya está en pt, sin calibrar: el tramado se barre en el espacio de
% dispositivo YA transformado (EPSDisplay::rect aplica mt antes de emitir; SVG
% usa un <pattern> en userSpace de pt). Así FPATRN gap 4/2/1 → hatch_gap 4/2/1 pt.

display_size 10 10
world_window -0.1 1.1 -0.08 0.92

line_width 0.2

% --- FPATRN: 10 tramas (V1 FILL + FPATRN 1..10) ---
% Ángulo cíclico con mod(); paso por familias desde una lista (§5).
gaps = [4, 4, 4, 4, 2, 2, 2, 2, 1, 1]
for i = 0 to 9 {
    x = i * 0.1
    rectangle(hatch = mod(i, 4) * 45, hatch_gap = gaps[i]) { x 0  (x+0.1) .1 }
}

% --- FGRAY: grises con gray(x), x de 0 (negro) a 1 (blanco) ---
% V1 FGRAY g medía % de negro → gray(1 - g/100). Aquí el gris se COMPUTA del índice.
for i = 0 to 8 {
    x = i * 0.1
    rectangle(fill = gray(0.9 - i*0.1)) { x .3  (x+0.1) .4 }   % FGRAY 10..90
}
rectangle(fill=gray(0), color=gray(0)) { .9 .3  1 .4 }       % FGRAY -100: negro, contorneado

% --- FCOLOR: colores con nombre (V1: negativo/guion contorneaba; en V3, color=) ---
% Los colores son una lista; el índice da la posición (§5.1).
fills = ["black", "blue", "brown", "cyan", "green", "magenta", "orange", "red", "white", "yellow"]
for i = 0 to 9 {
    x = i * 0.1
    rectangle(fill = fills[i]) { x .6  (x+0.1) .7 }
}
% el blanco (i=8) se contornea encima para que se vea sobre el fondo blanco
rectangle(fill="white", color="black") { .8 .6  .9 .7 }

% --- Títulos ---
font_size 12
text("Different ways to fill areas") { 0 .85 }
font_size 10
text("FCOLOR") { 0 .73 }
text("FGRAY")  { 0 .43 }
text("FPATRN") { 0 .13 }
font_size 8
text("An hyphen or negative value indicates to outline") { 0 .8 }

% --- Etiquetas numéricas (V1 GNNUM) — pluma y avance en la propia llamada (§13.2) ---
numbers(from=1,  by=1,  count=10, decimals=0, at=(0.05,  -0.055), advance=(0.1, 0))  % 1..10 bajo las tramas
numbers(from=10, by=10, count=9,  decimals=0, at=(0.025,  .25),   advance=(0.1, 0))  % 10..90 en la fila de grises

text("-100")   { 0.925 .25 }
text("-white") { 0.8 .55 }
