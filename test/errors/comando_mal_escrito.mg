% EXPECT: no es un comando conocido
% EXPECT_AT: 6:1
% Señala al NOMBRE, no al `{` que le sigue: ninguna sentencia de estado toma un
% bloque, así que un `{` prueba que el identificador no era un comando.
display_size 5 5
polylnie { 0 0  1 1 }
