% Texto — alineación, marcado, matemáticas y tipografía fina.
%
% Rótulos anclados a líneas guía para ver align y valign; negrita y énfasis;
% subíndices, superíndices y letras griegas; comillas tipográficas, rayas,
% viñetas y símbolos fuera de Latin-1; y texto de varios renglones con /n.
%
% NOTAS --------------------------------------------------------------------
% ⚠ COBERTURA EXCLUSIVA, y por partida doble. Es la única cobertura de los
% caracteres fuera de Latin-1 —que las fuentes base-14 SÍ tienen y que hasta
% 2026-07-20 se descartaban en silencio, porque la codificación no sabía
% nombrarlos— y la única de TextBlock, el texto multilínea: sin las últimas
% líneas de este archivo, el interlineado y el valign de BLOQUE quedan sin red.
% Los tres rótulos de valign sobre la línea de la pluma ejercitan además la ruta
% de texto SIMPLE (un solo run), distinta de la de texto compuesto.
%
% ⚠ Y una tercera cobertura exclusiva: las tres últimas líneas son la ÚNICA red
% de que `font` COMO SENTENCIA alcance a un `text()`. Cerrar ese no-op no movió
% un solo golden, así que perderlo tampoco lo movería —ninguna compuerta puede
% verlo, igual que el bug de ámbito que guarda `seccion_eficaz`—. Las tres fijan
% cosas distintas y hay que conservar las tres: la ambiente llegando a un text()
% sin marcado, la ambiente SOBREVIVIENDO a un run math (un `$…$` dejaba su cara
% puesta para el resto del documento), y `font=` por-primitiva ganándole a la
% ambiente. Detalle en docs/bitacora.md, 2026-08-19.

display_size 10 7.2
font_size 8
world_window -5 5 -3.6 3

line_width 1
polyline { 0 -2.5 0 2.75 }

text("") { }

text("\hbar{/bk}$_0$") { 0 -2 }
text("$x_0 2x_\gamma x^{2x}\delta$") { 0 -1.5 } 
text("Un subíndice $x_0$ y un superíndice $x^2$") { 0 -1 } 

% centrado
align "center"
line_width 0.2
polyline { -2 0 2 0 ; -2 0.5 2 0.5  ; -2 1 2 1 }

text("Una letra griega \alpha y otra no.") { 0 0 }
valign "middle"
text("Aep $2x_0 ek$.") { 0 0.5 } 
%text("Aep $2x_0 ek$.", valign="middle") { 0 0.5 } 
valign "top"
text("\beta empezando por griega.") { 0 1 } 
%text("\beta empezando por griega.", valign="top") { 0 1 } 

% alineado a la derecha
align "right"

text("una {/bnegrita} y /eénfasis el resto.") { 0 2 }
text("un número \infty de posibilidades.") { 0 2.5 }

% --- valign con texto SIMPLE (sin marcado) y align por defecto (left) ---
% La línea horizontal es la pluma (y = 2.7): cada rótulo la ancla distinto.
%   top    → el texto cuelga por debajo de la línea
%   middle → el texto queda centrado sobre la línea
%   bottom → el texto se apoya (su base) en la línea
% Son rótulos de un solo run: ejercitan la ruta de texto simple (Text::draw), no
% la de texto compuesto; y con align=left comprueban que valign no mueve la x.
align "left"
line_width 0.2
polyline { -4.5 2.7 4.5 2.7 }
valign "top"
text("arriba") { -4.5 2.7 }
valign "middle"
text("medio") { -1.5 2.7 }
valign "bottom"
text("abajo") { 1.7 2.7 }



% --- Tipografia fuera de Latin-1 -------------------------------------
% Comillas «españolas» y “de imprenta”, raya de diálogo, puntos suspensivos, vinetas
% y el resto de kExtraTextGlyphs: caracteres que las fuentes base-14 SI tienen y
% que hasta 2026-07-20 se DESCARTABAN, porque ISOLatin1Encoding no sabia
% nombrarlos. Es la unica cobertura que tienen; sin esta linea, volverian a
% perderse en silencio.
valign "bottom"
align "left"
text("Comillas “inglesas” y ‘simples’, raya — y puntos…") { -4.5 -3.05 }
text("Viñeta • daga † por mil ‰ marca ™ euro € œuvre") { -4.5 -3.45 }

% Multilinea: `/n` rompe el renglon. Unica cobertura de TextBlock en el
% corpus -- sin esta linea, el interlineado y el valign de BLOQUE quedan sin red.
% Se prueban las tres cosas que solo pasan con varios renglones: el estado
% tipografico sobrevive al corte (la negrita sigue), un renglon vacio consume
% interlinea sin dibujar, y `valign` centra el CONJUNTO y no cada linea.
align "center"
valign "middle"
text("/bdos renglones/nen negrita") { -3.2 -1.2 }
text("con un renglon/n/nvacio en medio") { -3.2 -2.15 }

% --- Escapes: `\` + caracter NO alfabetico = ese caracter, literal -----
% Unica cobertura del escape en el corpus, y la unica compuerta que fija los
% GLIFOS que produce: las pruebas de test/errors solo miran stderr y el codigo
% de salida, asi que no pueden ver si `\{` dibuja una llave o no dibuja nada.
% Sin estas dos lineas, perder el escape volveria a la conducta que tenia hasta
% el 2026-08-06 —comerse los dos caracteres EN SILENCIO— sin mover un golden.
%
% La segunda va en math a proposito: ahi la llave no es solo un caracter, es un
% DELIMITADOR, y su clase de espaciado (abre/cierra, como `(`) decide el hueco a
% los lados. Con la clase equivocada el espaciado no desaparece, sale mal poco.
% Ademas `{` y `}` NO estan en el subset de LM Math, asi que este par ejercita
% la caida a Times-Italic que hacen los tres backends para un byte que la fuente
% math no tiene —y que `text_width` mide con esa misma tabla, o el centrado se
% iria a la deriva—.
align "left"
valign "bottom"
text("Escapes: a\{b\} \$5 m\/s \\") { 1.4 -2.05 }
text("y en math $\{x : x > 0\}$") { 1.4 -2.6 }

% ...y DENTRO de un sub/superindice sin llaves, que es una rama distinta del
% parser de texto —una copia mas pobre del escaner de `\comando`— y donde el
% escape, el espaciado explicito y las seis funciones se perdian EN SILENCIO
% hasta el 2026-08-30. Aqui la llave tiene que salir en tamano de indice y
% elevada, `sin` en redonda, y `\,` NO tiene que dibujar una coma.
text("índice: $x^\{$ $x^\sin$ $x^\,y$") { 2.2 -3.05 }

% --- La cara de fuente AMBIENTE -------------------------------------------
% `font` como sentencia tiñe los text() que siguen; un run math en medio no se
% la lleva por delante; y `font=` por-primitiva la sobreescribe.
{ font "italic"
  text("ambiente: itálica") { 1.4 2.2 }
  text("tras math $x$ sigue itálica") { 1.4 1.8 }
  text("y font= la sobreescribe", font="roman") { 1.4 1.4 }
}
