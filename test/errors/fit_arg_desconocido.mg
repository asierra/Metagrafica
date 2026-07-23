% EXPECT: fit: `disparate=` no existe
% `fit` es el ÚNICO de la familia que descarta en PARSE-time: sus argumentos
% nombrados no llegan a exec(), así que la compuerta de checkKnownArgs no lo
% alcanza y necesita su propio chequeo. Su único nombre válido es `stretch=`.
display_size 8 6
world_window 0 10 0 10
struct Caja() { rectangle { 0 0  1 1 } }
fit(Caja, disparate=1) { 0 0  2 2 }
