% EXPECT: falta el argumento del parámetro en la struct
% El silencio PEOR de la familia: `b` caía a Value(0) sin avisar, así que la
% figura salía PLAUSIBLE (la línea a (1,0) en vez de (1,3)) con código 0.
display_size 5 5
struct S(a, b) { polyline { 0 0  (a) (b) } }
S(1)
