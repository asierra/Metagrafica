% El espectro electromagnético, con la ventana del infrarrojo ampliada
%
% Una figura de divulgación hecha de rectángulos, texto y flechas. Lo que la
% distingue es el relleno DEGRADADO: el infrarrojo va de naranja a rojo a negro,
% las microondas de gris claro a oscuro, y el visible es un arcoíris continuo de
% magenta a rojo — un barrido de tono, escrito como las seis paradas que son los
% vértices del cubo RGB. Muestra `gradient=` con dos, tres y seis paradas,
% combinado con `color=` para contornear, y las puntas de flecha como atributo de
% la línea (`marker_start`/`marker_end`), que se orientan solas.

% NOTAS ————————————————————————————————————————————————————————
% Procedencia: figura del curso de Percepción Remota (posgrado, UNAM). El original
% es un mapa de bits; aquí está redibujada, no calcada, así que los cortes de banda
% son los convencionales y no medidas del archivo.
%
% Cobertura EXCLUSIVA: es el único ejemplo del corpus con relleno degradado, o sea
% el único que ejercita la cuarta invariante de paridad entre backends (que los tres
% formatos emitan el mismo número de rellenos degradados). Si se retira del corpus,
% esa compuerta se queda sin sujeto.
%
% Los límites en micras del recuadro ampliado (0.75 / 1 / 2.5 / 3 / 5 / 8 / 12) son
% los cortes habituales de las bandas del infrarrojo en teledetección; las dos franjas
% grises son los tramos que la atmósfera no deja pasar.
%
% ⚠️ Licencia gráfica en el extremo azul del visible: el MAGENTA no es un color
% espectral —es extra-espectral, mezcla de rojo y azul—, y el borde real por debajo de
% ~380 nm es el violeta. Se usa magenta porque cierra el círculo de tono y porque es lo
% que muestra la figura del curso; si algún día se quiere el borde correcto, la parada
% que hay que cambiar es la primera.
%
% Por qué el arcoíris sale EXACTO y no aproximado: a saturación plena, girar el tono
% recorre las aristas del cubo RGB, y cada arista es un segmento recto en RGB — que es
% precisamente lo que interpolan los tres backends entre dos paradas. Con las seis
% paradas en los vértices y repartidas por igual (60° de tono cada una), la rampa
% coincide con la de un modelo HSL punto por punto, sin muestrear nada.

display_size 16 10
world_window 0 100 0 62.5
font_size 9

% ── Encabezado y eje de longitud de onda ────────────────────────────
text("Mayor energía", align="left")  { 12 60 }
text("Menor energía", align="right") { 92 60 }
text("Longitud de onda (nm)", align="right") { 58 56.5 }
polyline(marker_end="arrow") { 60 57  66 57 }

text("$10^2$", align="center", valign="bottom") { 20 53 }
text("$10^4$", align="center", valign="bottom") { 44 53 }
text("$10^6$", align="center", valign="bottom") { 68 53 }
text("$10^8$", align="center", valign="bottom") { 90 53 }
polyline { 20 52  20 53 ; 44 52  44 53 ; 68 52  68 53 ; 90 52  90 53 }

% ── La banda principal ──────────────────────────────────────────────
% Ultravioleta: color plano.
rectangle(fill="#B07FC7", color="black") { 12 38  20 52 }
text("Ultra/nvioleta", align="center", valign="middle", color="white", font_size=7) { 16 45 }

% El visible: un barrido de TONO de magenta a rojo, en un solo rectángulo.
% Las seis paradas son los vértices del cubo RGB, que es lo que hace que esto sea
% exacto y no una aproximación: a saturación plena, girar el tono recorre las
% aristas del cubo, y cada arista es un segmento RECTO en RGB — justo lo que
% interpola el degradado. Con 60° de tono entre parada y parada y las paradas
% repartidas por igual, el resultado es el arcoíris de un modelo HSL.
%           magenta    azul      cian      verde    amarillo    rojo
%             300°      240°      180°      120°       60°        0°
rectangle(gradient=["#FF00FF", "#0000FF", "#00FFFF", "#00FF00", "#FFFF00", "#FF0000"]) { 20 38  26 52 }

% Infrarrojo: TRES paradas, repartidas por igual a lo largo del eje.
rectangle(gradient=["#F5A623", "#D93622", "#1A0A06"], color="black") { 26 38  62 52 }
text("Infrarrojo", align="center", valign="middle", color="white") { 44 48 }
polyline(marker_start="arrow", marker_start_orient="reverse", marker_end="arrow", line_width=1.2) { 29 42  59 42 }

% Microondas: dos paradas y el mismo eje horizontal.
rectangle(gradient=[gray(0.78), gray(0.30)], color="black") { 62 38  92 52 }
text("Microondas", align="center", valign="middle", color="white") { 77 48 }
polyline(marker_start="arrow", marker_start_orient="reverse", marker_end="arrow", line_width=1.2) { 65 42  89 42 }

% ── Eje de frecuencia, debajo ───────────────────────────────────────
text("$10^{16}$", align="center", valign="top") { 12 37 }
text("$10^{14}$", align="center", valign="top") { 36 37 }
text("$10^{12}$", align="center", valign="top") { 60 37 }
text("$10^{10}$", align="center", valign="top") { 82 37 }
polyline { 12 38  12 37 ; 36 38  36 37 ; 60 38  60 37 ; 82 38  82 37 }

text("Frecuencia (s$^{-1}$)", align="left") { 72 32 }
polyline(marker_end="arrow") { 70 32  64 32 }

% ── El recuadro ampliado del infrarrojo ─────────────────────────────
polyline { 26 38  18 22 ; 62 38  86 22 }

text("Longitud de onda ($\mu$m)", align="center") { 52 25.5 }
polyline(marker_end="arrow") { 42 26  30 26 }
polyline(marker_end="arrow") { 62 26  74 26 }

rectangle(fill="#F5A623", color="black") { 18 10  28 22 }
text("IRC", align="center", valign="bottom", color="white") { 23 16.5 }
text("cercano", align="center", valign="top", color="white", font_size=6) { 23 15.5 }

rectangle(fill="#ED7722", color="black") { 28 10  46 22 }
text("IROC", align="center", valign="bottom", color="white") { 37 16.5 }
text("onda corta", align="center", valign="top", color="white", font_size=6) { 37 15.5 }

rectangle(fill=gray(0.45), color="black") { 46 10  51 22 }

rectangle(fill="#E8722E", color="black") { 51 10  66 22 }
text("IROM", align="center", valign="bottom", color="white") { 58.5 16.5 }
text("onda media", align="center", valign="top", color="white", font_size=6) { 58.5 15.5 }

rectangle(fill=gray(0.45), color="black") { 66 10  71 22 }

% La última banda se apaga hacia el infrarrojo lejano: degradado de dos paradas.
rectangle(gradient=["#C0392B", "#1A0A06"], color="black") { 71 10  86 22 }
text("IROL", align="center", valign="bottom", color="white") { 78.5 16.5 }
text("onda larga", align="center", valign="top", color="white", font_size=6) { 78.5 15.5 }

text("0.75", align="center", valign="top") { 18 9 }
text("1",    align="center", valign="top") { 28 9 }
text("2.5",  align="center", valign="top") { 46 9 }
text("3",    align="center", valign="top") { 51 9 }
text("5",    align="center", valign="top") { 66 9 }
text("8",    align="center", valign="top") { 71 9 }
text("12",   align="center", valign="top") { 86 9 }
