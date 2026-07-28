% EXPECT: El cuerpo de un 'for' lleva sentencias, no coordenadas
% EXPECT_AT: 9:16
% El mismo diagnóstico, con la SEGUNDA línea distinta: aquí juntar las líneas no
% arreglaría nada, porque la causa es querer generar coordenadas con un lazo. Es el
% tropiezo que la referencia describe («un `for` NO puede ir dentro de un
% bloque de coordenadas»), visto desde el otro lado.
display_size 6 6
world_window 0 6 0 6
for i = 0 to 3 { 0 0  1 1 }
