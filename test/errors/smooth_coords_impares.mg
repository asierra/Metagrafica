% EXPECT: número impar de coordenadas
% smooth NO acepta puntos [x,y], así que conserva el chequeo estricto de
% parse-time (con línea:columna), a diferencia de las primitivas.
display_size 5 5
smooth { 0 0  1 1  2 }
