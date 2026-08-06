% EXPECT_WARN: sin nada que escapar
% Una cadena que termina en `\` casi siempre es un escape a medio escribir. Antes
% del escape E1 se descartaba sin decir nada —el escaneo alfabético no encontraba
% nombre, no entraba al `if` y la barra se perdía—, que es la conducta que este
% aviso vino a sustituir.
%
% El diagnóstico NO es fatal, por consistencia con `\frac` y `\hat`: son errores de
% markup de texto, no del documento. Lo que el fixture protege es que siga saliendo,
% porque su regresión no rompe nada visible —la salida queda byte-idéntica, con la
% barra ausente igual que antes— y las demás compuertas seguirían en verde.

display_size 6 4
world_window 0 6 0 4
text("barra colgante \") { 0.5 2 }
