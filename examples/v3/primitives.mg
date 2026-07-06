% Formas básicas — traducción V3 de examples/v1/primitives.mg
%
% ⚠ AÚN NO COMPILABLE: el parser de la gramática V3 (§1–§18) no existe todavía.
% Este archivo es un fixture de la spec y el objetivo de traducción del par
% examples/v1/primitives.mg → examples/v3/primitives.mg. Su render debe igualar
% la referencia examples/v1/reference/primitives.svg (oráculo de migración).
%
% Estilo V3: estado por sentencias con ámbito de bloque (§7); transformaciones
% como sentencias (§11.1); contorno de relleno = presencia de color= (§4).

% Con estas dimensiones, 1 equivale a 0.5 cm (el canvas mide 11 cm)
display_size 11 11

% Tamaño base de texto: 15 pt ≈ 0.5 cm   ⚠ (keyword de config aún por fijar, §19)
font_size 15

world_window -1.1 1.1 -1.1 1.1

% Checar ancho de línea  (V1 LWIDTH 60 → 60 × 0.2 = 12 pt)
width 12
polyline { 0.05 0.1  0.05 0.2 }

% Checar tamaño de letra  (V1 LWIDTH 0 → hairline)
width 0.1
polyline { 0.1 0  0.1 0.1 }
text(0, 0) { "M" }

% Ejes
polyline { -1 0  1 0 }
polyline { 0 -1  0 0.95 }

% Todo el texto que sigue va centrado (V1 TALIGN 1)
align "center"
text(0, 1) { "Primitivos gráficos" }

{                              % acota size/width de la sección
    size 10
    width 0.8

    text(0.5, 0.75) { "Círculo y arcos" }
    circle(r=0.2)               { 0.5 0.5 }
    arc(r=0.25, from=0,  to=90) { 0.5 0.5 }
    arc(r=0.3,  from=30, to=45) { 0.5 0.5 }   % V1 CR 0.3 15 30: a0=30, da=15
    arc(r=0.3,  from=0,  to=-30){ 0.5 0.5 }

    text(-0.5, 0.75) { "Elipse" }
    ellipse(rx=0.2, ry=0.1) { -0.5 0.5 }

    text(0.5, -0.2) { "Rectángulo" }
    rectangle { 0.5 -0.25  0.75 -0.6 }

    text(-0.5, -0.2) { "Polígono" }
    polygon { -0.2 -0.5  -0.4073 -0.2147  -0.7427 -0.3237
              -0.7427 -0.6763  -0.4073 -0.785 }

    {                          % los mismos, rellenos y desplazados
        translate -0.2 -0.15
        color "green"
        fill  "cyan"
        % V1: FCOLOR -cyan + FPATRN -2 (trama con contorno).
        % ⚠ FPATRN índice → hatch ángulo no es 1:1: revisión manual (§20).
        polygon { -0.2 -0.5  -0.4073 -0.2147  -0.7427 -0.3237
                  -0.7427 -0.6763  -0.4073 -0.785 }
        circle(r=0.2)           { 0.5 0.5 }
        ellipse(rx=0.2, ry=0.1) { -0.5 0.5 }
        rectangle { 0.5 -0.6  0.75 -0.25 }
    }
}
