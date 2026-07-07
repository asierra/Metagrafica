% Formas de rellenar áreas — traducción V3 de examples/v1/fill_styles.mg
%
% ⚠ AÚN NO COMPILABLE (no hay parser V3). Fixture de spec; su render debe igualar
% examples/v1/reference/fill_styles.svg.
% Ejercita §4.11 (hatch), §4 (gris con gray()/color/contorno) y §13.2 (numbers).
%
% FPATRN → hatch/hatch_gap es 1:1 (Display::patternFor = 4 ángulos × 3 densidades):
%   idx:  1    2    3    4    5    6    7    8    9   10
%   áng:  0   45   90  135    0   45   90  135    0   45
%   gap:  4    4    4    4    2    2    2    2    1    1
% ⚠ el valor de hatch_gap (4/2/1) está en unidades de patrón de V1; su unidad en
%   §4.11 es "pt" → el traductor debe calibrar para reproducir exactamente.

display_size 10 10
world_window -0.1 1.1 -0.08 0.92

width 0.2

% --- FPATRN: 10 tramas (V1 FILL + FPATRN 1..10) ---
rectangle(hatch=0,   hatch_gap=4) { 0  0  .1 .1 }
rectangle(hatch=45,  hatch_gap=4) { .1 0  .2 .1 }
rectangle(hatch=90,  hatch_gap=4) { .2 0  .3 .1 }
rectangle(hatch=135, hatch_gap=4) { .3 0  .4 .1 }
rectangle(hatch=0,   hatch_gap=2) { .4 0  .5 .1 }
rectangle(hatch=45,  hatch_gap=2) { .5 0  .6 .1 }
rectangle(hatch=90,  hatch_gap=2) { .6 0  .7 .1 }
rectangle(hatch=135, hatch_gap=2) { .7 0  .8 .1 }
rectangle(hatch=0,   hatch_gap=1) { .8 0  .9 .1 }
rectangle(hatch=45,  hatch_gap=1) { .9 0  1  .1 }

% --- FGRAY: grises con gray(x), x de 0 (negro) a 1 (blanco) ---
% V1 FGRAY g medía % de negro → gray(1 - g/100). El contorno en V3 es color= (§4).
rectangle(fill=gray(0.9)) { 0  .3 .1 .4 }   % FGRAY 10
rectangle(fill=gray(0.8)) { .1 .3 .2 .4 }   % FGRAY 20
rectangle(fill=gray(0.7)) { .2 .3 .3 .4 }   % FGRAY 30
rectangle(fill=gray(0.6)) { .3 .3 .4 .4 }   % FGRAY 40
rectangle(fill=gray(0.5)) { .4 .3 .5 .4 }   % FGRAY 50
rectangle(fill=gray(0.4)) { .5 .3 .6 .4 }   % FGRAY 60
rectangle(fill=gray(0.3)) { .6 .3 .7 .4 }   % FGRAY 70
rectangle(fill=gray(0.2)) { .7 .3 .8 .4 }   % FGRAY 80
rectangle(fill=gray(0.1)) { .8 .3 .9 .4 }   % FGRAY 90
rectangle(fill=gray(0), color=gray(0)) { .9 .3 1 .4 }  % FGRAY -100 (contorneado)

% --- FCOLOR: colores con nombre (V1: negativo/guion contorneaba; en V3, color=) ---
rectangle(fill="black")   { 0  .6 .1 .7 }
rectangle(fill="blue")    { .1 .6 .2 .7 }
rectangle(fill="brown")   { .2 .6 .3 .7 }
rectangle(fill="cyan")    { .3 .6 .4 .7 }
rectangle(fill="green")   { .4 .6 .5 .7 }
rectangle(fill="magenta") { .5 .6 .6 .7 }
rectangle(fill="orange")  { .6 .6 .7 .7 }
rectangle(fill="red")     { .7 .6 .8 .7 }
rectangle(fill="white", color="black") { .8 .6 .9 .7 }  % FCOLOR -white: contorno o no se ve
rectangle(fill="yellow")  { .9 .6 1 .7 }

% --- Títulos ---
font_size 12
text(0, .85) { "Different ways to fill areas" }
font_size 10
text(0, .73) { "FCOLOR" }
text(0, .43) { "FGRAY" }
text(0, .13) { "FPATRN" }
font_size 8
text(0, .8) { "An hyphen or negative value indicates to outline" }

% --- Etiquetas numéricas (V1 GNNUM) — pluma y avance en la propia llamada (§13.2) ---
numbers(from=1,  by=1,  count=10, decimals=0, at=(0.05,  -0.055), advance=(0.1, 0))  % 1..10 bajo las tramas
numbers(from=10, by=10, count=9,  decimals=0, at=(0.025,  .25),   advance=(0.1, 0))  % 10..90 en la fila de grises

text(0.925, .25) { "-100" }
text(0.8, .55) { "-white" }
