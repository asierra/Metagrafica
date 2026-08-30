% EXPECT_WARN: symbol name unknown circ
% Un símbolo que no existe, escrito DENTRO de un sub/superíndice sin llaves
% (`$^\circ$`), tiene que avisar y descartarse igual que fuera de él (§6: «avisa por
% la salida de error y DESCARTA el símbolo»). La rama de `_`/`^` es una SEGUNDA
% implementación, más pobre, del escaneo de `\comando`, y por eso divergió: decía
% «Error: Unrecognized variable <circ>» —que ni era un error, porque salía con
% código 0, ni descartaba— y además dejaba los caracteres sobrantes del nombre en la
% figura como texto literal: `text("($^\circ$C)")` dibujaba «(irc C)».
%
% ⚠️ Lo que este fixture puede ver es SOLO el mensaje (la compuerta mira stderr y el
% código de salida, nada más). La mitad que de verdad dolía —la basura dibujada— le
% es invisible: si alguien conservara el aviso y perdiera el avance del índice, este
% archivo seguiría pasando con «(irc C)» en la figura. Fijar el GLIFO pediría una
% línea en el golden de `examples/texto.mg`, y ahí el precio es que el corpus
% imprimiría un aviso en cada compilación —ruido permanente en la única salida donde
% un aviso significa «revisa esto»—, así que la afirmación se parte: el mensaje aquí,
% y los glifos, no.
%
% Es también el modo de fallo que peor sienta en una cadena automática, que es de
% donde vino el reporte: un compilador que dice «Error» y devuelve 0 no deja al
% guion de arriba ni fallar ni confiar.

display_size 6 4
world_window 0 6 0 4

text("($^\circ$C)") { 0.5 2 }
