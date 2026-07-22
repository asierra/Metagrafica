% EXPECT: el literal de path recibió un número impar de coordenadas
% EXPECT_AT: 4:19
display_size 5 5
path p = { 0 0  1 }
polyline(&p)
