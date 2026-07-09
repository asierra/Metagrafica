% Formas básicas — traducción V3 de examples/v1/primitives.mg
%
% ⚠ AÚN NO COMPILABLE: el parser de la gramática V3 (§1–§18) no existe todavía.
% Este archivo es un fixture de la spec y el objetivo de traducción del par
% examples/v1/primitives.mg → examples/v3/primitives.mg. Su render debe igualar
% la referencia examples/v1/reference/primitives.svg (oráculo de migración).
%
% Estilo V3: estado por sentencias con ámbito de bloque (§7); transformaciones
% como sentencias (§11.1); contorno de relleno = presencia de color= (§4).
% Convención de coordenadas: las coordenadas siempre van en el bloque { }; los
% paréntesis llevan el contenido y los atributos. Por eso el texto es
% text("cadena") { x y }, igual que circle(r) { x y } (§4.8).

% Con estas dimensiones, 1 equivale a 0.5 cm (el canvas mide 11 cm)
display_size 11 11

% Tamaño base de texto: 15 pt ≈ 0.5 cm (font_size es absoluto, mismo keyword
% en documento y en bloque, §7.3)
font_size 15

world_window -1.1 1.1 -1.1 1.1

% Ancho de línea en pt (V1 LWIDTH 60 → 60 × 0.2 = 12 pt)
line_width 12
polyline { 0.05 0.1  0.05 0.2 }

% Línea hairline: en V3 no hay grosor dependiente del aparato, se da explícito
line_width 0.1
polyline { 0.1 0  0.1 0.1 }
text("M") { 0 0 }

% Ejes
polyline { -1 0  1 0 }
polyline { 0 -1  0 0.95 }

% Todo el texto que sigue va centrado (V1 TALIGN 1). align (horizontal) y valign
% (vertical) son ortogonales; aquí basta el horizontal (§4.8).
align "center"

text("Primitivos gráficos") { 0 1 }

{                              % acota font_size/line_width de la sección
    font_size 10
    line_width 0.8

    text("Círculo y arcos") { 0.5 0.75 }
    circle(0.2)               { 0.5 0.5 }   % r posicional
    arc(0.25, from=0,  to=90) { 0.5 0.5 }   % r posicional, ángulos nombrados
    arc(0.3,  from=30, to=45) { 0.5 0.5 }   % V1 CR 0.3 15 30: a0=30, da=15
    arc(0.3,  from=0,  to=-30){ 0.5 0.5 }

    text("Elipse") { -0.5 0.75 }
    ellipse(0.2, 0.1) { -0.5 0.5 }          % rx, ry posicionales (x, y)

    % Caracteres acentuados: el motor recodifica el vector de las fuentes
    % estándar cuando el texto los contiene (§18).
    text("Rectángulo") { 0.5 -0.2 }
    rectangle { 0.5 -0.25  0.75 -0.6 }

    text("Polígono") { -0.5 -0.2 }
    polygon { -0.2 -0.5  -0.4073 -0.2147  -0.7427 -0.3237
              -0.7427 -0.6763  -0.4073 -0.785 }

    {                          % los mismos, ahora tramados y desplazados
        translate -0.2 -0.15
        % V1: FCOLOR -cyan + FPATRN -2. En V3 la trama son trazos: color fija el
        % color de sus líneas (y el contorno, si se quiere). FPATRN 2 = hatch 45°,
        % gap 4 — mapeo 1:1 (ver fill_styles.mg). Forma de sentencia: ángulo
        % (obligatorio) y paso (opcional) en una línea, "hatch <ángulo> [paso]".
        color "green"
        hatch 45 4
        polygon { -0.2 -0.5  -0.4073 -0.2147  -0.7427 -0.3237
                  -0.7427 -0.6763  -0.4073 -0.785 }
        circle(0.2)       { 0.5 0.5 }
        ellipse(0.2, 0.1) { -0.5 0.5 }
        rectangle { 0.5 -0.6  0.75 -0.25 }
    }
}
