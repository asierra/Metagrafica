% EXPECT: demasiados argumentos en la struct
% El otro sentido del mismo defecto: los argumentos de más se descartaban mudos.
display_size 5 5
struct S(a) { polyline { 0 0  (a) 1 } }
S(1, 2, 3, 4)
