% Formas de rellenar áreas — traducción V3 de examples/v1/fill_styles.mg
%
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

% --- Títulos ---
font_size 12
text("Different ways to fill areas") { 0 .85 }
font_size 8
text("To outline just add the parameter /ecolor") { 0 .8 }

font_size 9
text("For patterns use hatch, hatchback and crosshatch ahd hatch_gap") { 0 .13 }

line_width 0.2

% --- FPATRN: 10 tramas (V1 FILL + FPATRN 1..10) ---
% Ángulo cíclico con mod(); paso por familias desde una lista (§5).
gaps = [4, 4, 4, 2, 2, 2, 1, 1, 1]
hatchs = [ "hatch", "hatchback", "crosshatch" ]
font_size 5
for i = 0 to 8 {
    x = i * 0.11111111
    hh = hatchs[mod(i, 3)]
    gap = gaps[i]
    rectangle(hatch=hh, hatch_gap = gap) { x 0  (x+0.1) .1 }
    text(hh+str(gap, 0), font_size=6) { x -0.055}
}

% --- FGRAY: grises con gray(x), x de 0 (negro) a 1 (blanco) ---
% El oráculo (V1 FGRAY 10..90, izq→der) va de OSCURO a claro: FGRAY g → gray(g/100),
% así que FGRAY 10 = gray(0.1) (#191919) y FGRAY 90 = gray(0.9). El gris se computa
% del índice: i=0..8 → gray(0.1)..gray(0.9).

font_size 9
text("For grays use gray(0.0) to gray(1.0)")  { 0 .43 }
for i = 0 to 8 {
    x = i * 0.1
    rectangle(fill = gray(i*0.1)) { x .3  (x+0.1) .4 }   % FGRAY 10..90
}
rectangle(fill="white", color=gray(0)) { .9 .3  1 .4 }       % FGRAY -100: negro, contorneado

% --- FCOLOR: colores con nombre (V1: negativo/guion contorneaba; en V3, color=) ---
% Los colores son una lista; el índice da la posición (§5.1).

text("For colors use fill=/icolorname") { 0 .73 }
fills = ["black", "blue", "brown", "cyan", "green", "magenta", "orange", "red", "white", "yellow"]
font_size 6
for i = 0 to 9 {
    x = i * 0.1
    % el blanco (i=8) se contornea para que se vea sobre el fondo (V1: FCOLOR -white)
    if i == 8 { 
        rectangle(fill="white", color="black") { x .6  (x+0.1) .7 } 
        text("white") { 0.8 .55 }
    }    
    else { 
        rectangle(fill=fills[i]) { x .6  (x+0.1) .7 } 
        text(fills[i])   { (0.1*i) .55 }
    }
}

% --- Etiquetas numéricas (V1 GNNUM) — pluma y avance en la propia llamada (§13.2) ---
numbers(from=0.1, by=0.1, count=10,  decimals=1, at=(0.025,  .25),   advance=(0.1, 0))  % 10..90 en la fila de grises

