% EXPECT: falta el argumento del parámetro en la struct
% La misma guarda por la OTRA vía: fit(Struct(...)) comparte bindStructParams.
display_size 5 5
struct S(a, b) { polyline { 0 0  (a) (b) } }
fit(S(1)) { 0 0  1 1 }
