% EXPECT: argumento duplicado (posicional y nombrado) en la struct
% Antes ganaba el posicional sin decir nada.
display_size 5 5
struct S(a) { polyline { 0 0  (a) 1 } }
S(1, a=2)
