% Formas básicas — lámina de referencia de las primitivas geométricas.
%
% Es un catálogo, no una figura: cada primitiva en su forma más simple, y luego
% las mismas con relleno y trama. Las formas se escriben AL RAS, sin envolverlas
% en structs, para que quien viene a buscar «cómo se dibuja una elipse» encuentre
% la línea que la dibuja. Hermana de fill_styles.mg y line_patterns.mg.
%
% NOTAS --------------------------------------------------------------------
% ⚠ NO BORRAR sin mudar antes lo que cubre en exclusiva. Medido sobre el corpus
% (2026-07-22), es el ÚNICO de los 22 ejemplos que ejercita:
%   - ellipse()                    (y con ella el procedimiento /ellipse del EPS)
%   - polyline(closed=true)
%   - arc con barrido NEGATIVO (to=-30)
% y uno de solo dos con circle(), polygon() y translate. Además es el caso que
% reproducen los dos pendientes de fase abiertos (guiones EPS vs PDF; trama SVG
% vs EPS/PDF — ver PENDIENTES.md, deuda técnica).
%
% Convención de coordenadas: las coordenadas SIEMPRE van en el bloque { }, y los
% paréntesis llevan el contenido y los atributos. Por eso el texto se escribe
% text("cadena") { x y }, igual que circle(r) { x y }.

% El canvas mide 11 cm y la ventana 2.2 unidades de ancho, así que 1 unidad de
% mundo = 5 cm.
display_size 11 11

% Tamaño base de texto: 15 pt ≈ 0.53 cm (font_size es absoluto, mismo keyword
% en documento y en bloque)
font_size 15

world_window -1.1 1.1 -1.1 1.1

% --- Marcas de calibración. No son adorno: son las dos varas del documento, y
% por eso van antes que nada. El motor las emite en unidades FÍSICAS y el mundo es cantidad, así
% que compararlas a ojo en el render es la prueba de que la escala está bien.

% Vara de ANCHO: 12 pt de grosor. El trazo mide 0.1 u = 0.5 cm de largo, o sea
% que sale casi cuadrado — si el "cuadrado" se ve rectangular, el grosor miente.
line_width 12
polyline { 0.05 0.1  0.05 0.2 }

% Vara de TEXTO: 0.1 u = 0.5 cm = 14.17 pt, junto a una "M" de 15 pt. Están
% puestas para medirse una contra otra: la regleta debe quedar apenas por debajo
% de la altura nominal del cuerpo. En V3 el grosor nunca depende del aparato
% (no hay "hairline" del dispositivo), así que el trazo fino se pide explícito.
line_width 0.1
polyline { 0.1 0  0.1 0.1 }
text("M") { 0 0 }

% Ejes
polyline { -1 0  1 0 }
polyline { 0 -1  0 0.95 }

% Todo el texto que sigue va centrado. align (horizontal) y valign
% (vertical) son ortogonales; aquí basta el horizontal.
align "center"
text("Primitivos gráficos") { 0 1 }

% El pentágono se dibuja dos veces (contorno y tramado): se construye UNA vez
% como path. `+=` evalúa ya, así que lee la k del lazo; la semilla, en
% cambio, es diferida y por eso va vacía (un literal, sin variables).
pcx = -0.5   % centro
pcy = -0.5
pr  = 0.3    % radio (el vértice k=0 cae en -0.2 -0.5)
path pentagon = { }
for k = 0 to 4 {
    path pentagon += { (pcx + pr*cos(k*72*pi/180))  (pcy + pr*sin(k*72*pi/180)) }
}

{                              % acota font_size/line_width de la sección
    font_size 10
    line_width 0.8

    text("Círculo y arcos") { 0.5 0.75 }
    circle(0.2)               { 0.5 0.5 }   % r posicional
    arc(0.25, from=0,  to=90) { 0.5 0.5 }   % r posicional, ángulos nombrados
    arc(0.3,  from=30, to=45) { 0.5 0.5 }   % barrido de 15° desde 30°
    arc(0.3,  from=0,  to=-30){ 0.5 0.5 }

    text("Elipse") { -0.5 0.75 }
    ellipse(0.2, 0.1) { -0.5 0.5 }          % rx, ry posicionales (x, y)

    % Caracteres acentuados: el motor recodifica el vector de las fuentes
    % estándar cuando el texto los contiene.
    text("Rectángulo") { 0.5 -0.2 }
    rectangle { 0.5 -0.25  0.75 -0.6 }

    text("Polígono") { -0.5 -0.2 }
    % Una polilínea se cierra con closed=true. Abajo el MISMO path se
    % dibuja con polygon(), que cierra por construcción: el par documenta la
    % diferencia entre las dos primitivas, que es lo único que las distingue.
    polyline(&pentagon, closed=true)

    {                          % los mismos, ahora tramados y desplazados
        translate -0.2 -0.15
        % La trama es relleno: sus líneas toman el color de RELLENO (fill, cyan),
        % mientras que color (verde) es el del contorno, que se dibuja al pedir
        % outlinefill. La sentencia es "hatch <ángulo> [paso]".
        color "green"      % color del contorno (con outlinefill)
        fill "cyan"        % color de las líneas de la trama
        outlinefill        % contornea la región
        hatch 45 4
        polygon(&pentagon)                      % cierra por construcción
        circle(0.2)       { 0.5 0.5 }
        ellipse(0.2, 0.1) { -0.5 0.5 }
        rectangle { 0.5 -0.25  0.75 -0.6 }      % mismas esquinas que arriba
    }
}
