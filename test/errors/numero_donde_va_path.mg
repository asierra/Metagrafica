% EXPECT: se esperaba un path (&nombre) para el parámetro q
display_size 5 5
struct S(&q) { polyline(&q) }
S(3)
