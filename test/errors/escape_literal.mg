% EXPECT_NO_WARN: \frac requiere dos grupos
% El escape E1 (2026-08-06): `\` seguido de un carácter NO ALFABÉTICO es ese
% carácter, literal y sin función. Antes de existir, `{`, `}`, `$`, `\` y la barra
% seguida de un código de cara eran INALCANZABLES en una cadena — y no fallaban
% ruidosamente: el escaneo de `\` consume lo alfabético que sigue, así que ante
% `\{` no leía ningún nombre y se comía los DOS caracteres en silencio.
%
% ⚠️ El fragmento vigilado es el del `\frac` de abajo, y está ELEGIDO POR MEDICIÓN,
% no por parecido temático. Se probó primero con `symbol name unknown` y NO SERVÍA:
% se corrió este mismo archivo contra un binario sin el escape y salía limpio, o sea
% que el fixture habría pasado con la característica rota. Es la consecuencia directa
% de que el fallo de E1 sea SILENCIOSO: no hay diagnóstico cuya ausencia delate su
% pérdida. El del `\frac` sí sale —con un binario sin E1, este archivo lo emite—,
% porque una llave escapada desbalancea el conteo de grupos. Es un aviso indirecto,
% y es el único que esta compuerta puede ver.
%
% Quien fija los GLIFOS —que `\{` dibuje una llave y no nada— es el golden de
% `examples/texto.mg`, que los lleva. Esta compuerta no puede: solo mira stderr y el
% código de salida.
%
% ⚠️ Y va a los TRES backends por el `\\` de la tercera línea, que es el caso
% peligroso de verdad. En PostScript la barra invertida es el carácter de escape
% de un literal de cadena: un `\` crudo produciría `(\) show`, cadena sin cerrar,
% y Ghostscript daría /syntaxerror. Es exactamente el bug que tuvo `\therefore`
% —el único símbolo cuyo byte es 92— hasta el 2026-07-20, y que NADIE cazó porque
% el golden bendice un EPS sintácticamente roto (es byte-estable). Hasta hoy ese
% byte solo se alcanzaba con aquel símbolo; el escape abre la puerta a que
% cualquiera escriba una barra, así que la salida EPS tiene que aguantarlo.

display_size 12 8
world_window 0 12 0 8
font_size 10

text("llaves a\{b\}c") { 0.5 7 }
text("barra literal 5 m\/s") { 0.5 6 }
text("contrabarra suelta \\ y precio \$5") { 0.5 5 }
text("guion bajo a\_b y techo a\^b") { 0.5 4 }
text("parentesis \(escapado\) y (crudo)") { 0.5 3 }

% En math el literal sigue siendo un ÁTOMO y toma su clase de espaciado: `\{` abre
% (MC_OPEN) y `\}` cierra (MC_CLOSE), como `(` y `[`. Con la clase equivocada el
% espaciado no desaparecería, saldría MAL POCO — que es lo difícil de ver.
text("$\{ x : x > 0 \}$") { 0.5 2 }

% Llave escapada DENTRO del grupo de un \frac. Es la trampa: `extractGroup` cuenta
% llaves para hallar el grupo balanceado, así que sin saltar los pares escapados
% cortaría los dos grupos en el sitio equivocado, y en silencio, porque las llaves
% *parecen* balanceadas.
text("$\frac{a\{}{b}$") { 0.5 1 }
