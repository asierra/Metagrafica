% EXPECT_NO_WARN: cambia la cara tipográfica
% El reverso de `cara_al_final.mg`. Un aviso lleno de falsos positivos es peor que no
% tenerlo —enseña a ignorarlo—, y ésa es la otra forma de matarlo.
%
% ⚠️ Este fixture fija una MEDICIÓN, no una intuición. Al diseñar el aviso del `m/s`
% se consideró una heurística más amplia: sospechar de toda barra PEGADA a una letra,
% que es como se escribe `m/s`. Se midió sobre el corpus y tiene falso positivo: el
% `$\mu/rm$` de fig2-5 lleva la `/r` pegada a la `u` de `\mu` y es legítima —la barra
% viene justo después de un símbolo, no de una letra de texto, y el compilador no
% distingue una cosa de la otra a esa altura—. Por eso el aviso se quedó en el único
% caso sin escapatoria (cambio de cara al final de la cadena), y por eso ese caso
% medido va aquí abajo: si alguien vuelve a ampliar la heurística, esta prueba se lo
% dice antes de que el aviso se vuelva ruido.

display_size 12 6
world_window 0 12 0 6
font_size 10

% Cambios de cara con texto después: lo normal.
text("una {/bnegrita} y /eénfasis el resto.") { 0.5 5 }
text("/sen sanserif y /rde vuelta a la romana.") { 0.5 4 }

% El caso MEDIDO: barra pegada a la `u` de `\mu`, legítima (fig2-5:  µm en romana).
text("$\Delta T$ (BT 10.3 - 12.3 $\mu/rm$)") { 0.5 3 }

% `/n` rompe renglón; tampoco es un cambio de cara y no debe avisar.
text("dos/nrenglones") { 0.5 2 }

% Una barra cuyo siguiente carácter NO es código de cara cae al literal sin ruido.
text("W/m2 y 10/12") { 0.5 1 }
