% EXPECT: acos: argumento fuera de [-1, 1]
% Fuera de dominio, `acos` ABORTA en vez de devolver NaN. Es el punto entero de que
% exista: antes había que escribirlo como `atan2(sqrt(1-q*q), q)`, y ahí el sqrt de
% un negativo daba NaN callando, atan2 lo propagaba y la figura salía con coordenadas
% `-nan` y código 0 — el modo de falla que evalError se volvió fatal para eliminar.

display_size 5 5
world_window 0 5 0 5
q = 1.3
dot(2) { (acos(q)) 2 }
