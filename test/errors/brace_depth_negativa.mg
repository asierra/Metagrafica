% EXPECT: para voltear la llave, invierte los dos puntos del vano
% El costado de una llave lo decide el ORDEN de los dos puntos del vano, no el
% signo de `depth` (ver bracePlaced en brace.h: el eje +x local es el vano girado
% −90°). Una profundidad negativa no voltea nada.
%
% ⚠️ Y sin esta guardia no fallaba: `braceRadius` toma el VALOR ABSOLUTO de depth,
% así que `depth=-8` dibujaba exactamente lo mismo que `depth=8`. Quien lo
% escribiera esperando el espejo se quedaría mirando una llave que no se mueve, sin
% una sola pista de por qué — y la salida sería un SVG perfectamente válido. Es la
% clase de fallo que este proyecto persigue: mudo y plausible.
display_size 8 6
world_window 0 8 0 6
brace(depth=-8) { 4 1  4 5 }
