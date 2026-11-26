% `exit` compila lo anterior y AVISA: sin el aviso, un `exit` olvidado deja una
% figura incompleta con código de salida 0.
% EXPECT_WARN: con 'exit' se compila hasta la línea 6
polyline { 0 0  1 1 }
% la polylnie de abajo prueba además que lo posterior no se lee
exit
polylnie { 0 1  1 0 }
