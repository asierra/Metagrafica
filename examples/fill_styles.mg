% Rellenos de área — lámina de referencia de tramas, grises y contornos.
%
% El catálogo de tramados (hatch, hatchback, crosshatch, y el crosshatch
% enderezado a rejilla recta con hatch_angle=0), la escala de grises y la
% combinación de relleno con contorno. Hermana de primitives.mg y line_patterns.mg.
%
% NOTAS --------------------------------------------------------------------
% La separación de la trama va en PUNTOS y no necesita calibrarse: el tramado se
% barre en el espacio de dispositivo YA transformado (EPS aplica la matriz antes
% de emitir; SVG usa un <pattern> en unidades de usuario), así que hatch_gap
% 4/2/1 son 4/2/1 pt en la salida, venga de donde venga la ventana.

display_size 10 10
world_window -0.1 1.1 -0.18 0.92

% --- Títulos ---
font_size 12
text("Different ways to fill areas") { 0 .85 }
font_size 8
text("To outline just add the parameter /ecolor") { 0 .8 }

yp = -0.05                                   % base de la fila de patterns (empujada abajo)
font_size 9
text("For patterns: hatch, hatchback, crosshatch, hatch_gap, hatch_angle") { 0 (yp+0.13) }

line_width 0.2

% --- Tramas: un swatch por estilo, rótulo limpio (paso fijo de 4 pt) ---
% El cuarto es el crosshatch ENDEREZADO con hatch_angle=0 (rejilla recta). Se pasan
% los ángulos por defecto (45/135) en los demás para que la salida no se mueva.
styles = ["hatch", "hatchback", "crosshatch", "crosshatch"]
angles = [45,      135,         45,           0]
labels = ["hatch", "hatchback", "crosshatch", "hatch_angle=0"]
font_size 6
for i = 0 to 3 {
    x = i * 0.25
    rectangle(hatch=styles[i], hatch_gap=4, hatch_angle=angles[i]) { (x) (yp)  (x+0.13) (yp+0.1) }
    text(labels[i], font_size=6) { (x) (yp-0.055) }
}

% --- Grises con gray(x), de 0 (negro) a 1 (blanco) ---
% Van de OSCURO a claro, de izquierda a derecha: el gris se computa del índice,
% i = 0..8 → gray(0.1)..gray(0.9).

yg = 0.25                                    % base de la fila de grises (empujada abajo)
font_size 9
text("For grays use gray(0.0) to gray(1.0)")  { 0 (yg+0.13) }
for i = 0 to 8 {
    x = i * 0.1
    rectangle(fill = gray(i*0.1)) { (x) (yg)  (x+0.1) (yg+0.1) }   % gray(0.1)..gray(0.9)
}
rectangle(fill="white", color=gray(0)) { .9 (yg)  1 (yg+0.1) }       % blanco, contorneado en negro

% --- Colores con nombre; el contorno se pide con color= ---
% Los colores son una lista; el índice da la posición.

text("For colors use fill={/icolorname} or hex code, all 148 CSS names/nare supported") { 0 .73 }
fills = ["black", "blue", "brown", "cyan", "green", "magenta", "orange", "red", "white", "yellow"]
font_size 6
y = 0.55
for i = 0 to 9 {
    x = i * 0.1
    % el blanco (i=8) se contornea para que se vea sobre el fondo
    if i == 8 { 
        rectangle(fill="white", color="black") { (x) (y)  (x+0.1) (y+0.1) }
        text("white") { 0.8 (y-0.05) }
    }    
    else { 
        rectangle(fill=fills[i]) { (x) (y)  (x+0.1) (y+0.1) }
        text(fills[i])   { (0.1*i) (y-0.05) }
    }
}

% --- Etiquetas numéricas: pluma y avance en la propia llamada ---
numbers(from=0.1, by=0.1, count=10,  decimals=1, at=(0.025,  (yg-0.05)),   advance=(0.1, 0))  % 10..90 en la fila de grises

