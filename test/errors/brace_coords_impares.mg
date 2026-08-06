% EXPECT: el número de coordenadas no es par
% Cada PAR de puntos es un vano, como en `rectangle`. Un punto suelto no describe
% ninguna llave, y descartarlo en silencio dejaría al autor con una llave menos de
% las que escribió — la familia de «coordenadas sobrantes descartadas» que ya
% mordió a este proyecto.
display_size 8 6
world_window 0 8 0 6
brace(depth=8) { 2 1  2 5   6 1 }
