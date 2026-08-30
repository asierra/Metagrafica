% EXPECT_NO_WARN: symbol name unknown
% El reverso del fixture del símbolo desconocido: tres cosas LEGÍTIMAS dentro de un
% sub/superíndice sin llaves, que tienen que compilar limpias en los tres backends.
%
% La rama de `_`/`^` es una copia más pobre del escáner de `\comando`, y hasta el
% 2026-08-30 le faltaban las tres: `\{` (el escape E1) se comía los dos caracteres
% SIN avisar y encima descuadraba el resto de la cadena —el `{` seguía abriendo
% grupo—; `\,` dibujaba una COMA en vez de un espacio fino; y `\sin` avisaba
% «symbol name unknown sin», la misma palabra que a dos caracteres de distancia
% —`$\sin x$`, `$e^{\sin}$`— sale bien.
%
% ⚠️ Solo la tercera deja rastro en stderr, y por eso es la que se afirma aquí: las
% otras dos fallaban EN SILENCIO, que es justo lo que esta compuerta no puede ver.
% Sus GLIFOS —que la llave se dibuje, que no aparezca una coma— los fija el golden
% de `examples/texto.mg`, que los lleva.

display_size 8 4
world_window 0 8 0 4

text("$x^\{$ y $x_\}$") { 0.5 3 }
text("$x^\sin$ y $\sin x$") { 0.5 2 }
text("$x^\,y$ y $x^\quad y$") { 0.5 1 }
