% Rellenos de área — lámina de referencia de tramas, grises y contornos.
%
% El catálogo de tramados (cuatro ángulos por tres densidades), la escala de
% grises y la combinación de relleno con contorno. Hermana de primitives.mg y
% line_patterns.mg.
%
% NOTAS --------------------------------------------------------------------
% La separación de la trama va en PUNTOS y no necesita calibrarse: el tramado se
% barre en el espacio de dispositivo YA transformado (EPS aplica la matriz antes
% de emitir; SVG usa un <pattern> en unidades de usuario), así que hatch_gap
% 4/2/1 son 4/2/1 pt en la salida, venga de donde venga la ventana.

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

% --- Tramas: el ángulo cicla por familias y el paso por densidades ---
% Ángulo cíclico con mod(); paso por familias desde una lista.
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

% --- Grises con gray(x), de 0 (negro) a 1 (blanco) ---
% Van de OSCURO a claro, de izquierda a derecha: el gris se computa del índice,
% i = 0..8 → gray(0.1)..gray(0.9).

font_size 9
text("For grays use gray(0.0) to gray(1.0)")  { 0 .43 }
for i = 0 to 8 {
    x = i * 0.1
    rectangle(fill = gray(i*0.1)) { x .3  (x+0.1) .4 }   % gray(0.1)..gray(0.9)
}
rectangle(fill="white", color=gray(0)) { .9 .3  1 .4 }       % blanco, contorneado en negro

% --- Colores con nombre; el contorno se pide con color= ---
% Los colores son una lista; el índice da la posición.

text("For colors use fill=/icolorname") { 0 .73 }
fills = ["black", "blue", "brown", "cyan", "green", "magenta", "orange", "red", "white", "yellow"]
font_size 6
for i = 0 to 9 {
    x = i * 0.1
    % el blanco (i=8) se contornea para que se vea sobre el fondo
    if i == 8 { 
        rectangle(fill="white", color="black") { x .6  (x+0.1) .7 } 
        text("white") { 0.8 .55 }
    }    
    else { 
        rectangle(fill=fills[i]) { x .6  (x+0.1) .7 } 
        text(fills[i])   { (0.1*i) .55 }
    }
}

% --- Etiquetas numéricas: pluma y avance en la propia llamada ---
numbers(from=0.1, by=0.1, count=10,  decimals=1, at=(0.025,  .25),   advance=(0.1, 0))  % 10..90 en la fila de grises

