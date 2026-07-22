% EXPECT: no van dentro de fit(Struct(...))
% at=/rotate=/scale= dentro de la invocación interior de fit son ERROR, no se
% ignoran: competirían con la matriz del propio fit.
display_size 5 5
struct S() { polyline { 0 0  1 1 } }
fit(S(at=[1,1])) { 0 0  1 1 }
