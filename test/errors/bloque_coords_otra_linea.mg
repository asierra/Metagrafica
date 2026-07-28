% EXPECT: este '{' abre un bloque de ámbito
% EXPECT_AT: 16:5
% El bloque de coordenadas tiene que empezar en la MISMA LÍNEA que la cabeza de la
% primitiva. Bajarlo no es un error de escritura raro: es lo primero que se intenta
% al partir una llamada larga en varias líneas (salió escribiendo espectro.mg).
%
% Lo que este fixture protege NO es que falle —ya fallaba— sino que falle DICIENDO
% la causa. Antes el error venía del parseStatement de adentro y decía «se esperaba
% un comando… pero se encontró el número 0», señalando la primera coordenada: cierto
% y sin embargo inútil, porque un '{' suelto ES válido (el bloque de ámbito), así que
% el compilador no veía una primitiva rota sino un ámbito lleno de números.
display_size 6 6
world_window 0 6 0 6

rectangle(fill="red")
    { 0 0  4 3 }
