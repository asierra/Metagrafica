% EXPECT: no se esperaba un path para el parámetro a
display_size 5 5
struct S(a) { polyline { 0 0  (a) 1 } }
path p = { 0 0 }
S(&p)
