% Difracción de electrones — el experimento de Davisson-Germer.
%
% Es el ejemplo de ILUSTRACIÓN del README. El detector es una struct colocada
% girada 37°; el haz, la cota y el giro del detector son marcadores que se
% ORIENTAN SOLOS a la tangente de su línea o de su arco y se anclan al vértice,
% sin un solo ángulo calculado a mano.
%
% NOTAS --------------------------------------------------------------------
% Fig. 2.5 de Cetto & de la Peña, «Quantum Mechanics: A Physical Approach»
% (Cambridge University Press, 2025) — https://doi.org/10.1017/9781009679633.
% El nombre del archivo sigue esa numeración. Coincide por casualidad con la
% Fig. 2.5 del libro en español: el archivo se llamaba fig2-6 porque esa era su
% numeración en un borrador de Cambridge, y al eliminarse una figura anterior del
% capítulo pasó a ser la 2.5.
%
% El texto matemático usa Latin Modern Math, así que la φ no tiene el trazo ni la
% inclinación de las ediciones impresas. Es un cambio deliberado del proyecto.
%
% Si lo cambias, regenera su SVG:
%     bin/mg examples/fig2-5.mg docs/img/fig2-5.svg

display_size 12 4
font_size 8

% ── Pila (batería) ───────────────────────────────────────────────────────────
struct Pila() {
    world_window -0.125 0.875 0.5 1.5
    color "red"
    line_width 0.2
    polyline { 0 0 0 1 ;  0.5 0 0.5 1 ;  -0.125 0.5 0 0.5 ;  0.75 0.5 0.875 0.5 }
    line_width 1.6
    polyline { 0.25 0.2 0.25 0.8 ;  0.75 0.2 0.75 0.8 }
}

% ── Detector (soporte en L) ──────────────────────────────────────────────────
struct Detector() {
    world_window 0.5 1.2 0.5 1.2
    line_width 1.0
    polyline { 1 0.3  1 0  0 0  0 1  1 1  1 0.7 }
}

% ── Dispositivo ──────────────────────────────────────────────────────────────
world_window -1 11 -1 3

dot(1.0) { -0.5 1  -0.5 1.5 }                    % terminales

line_width 0.2
polyline { -0.5 1 0.4 1 0.4 1.15 0.55 1.25 0.4 1.35 0.4 1.5 -0.5 1.5 ;
           0.5 0.8 -0.2 0.8 -0.2 -0.4 2 -0.4 ;  2.6 -0.4 4.5 -0.4 4.5 0 }

polyline(dash="dashed") { 4.5 0 4.5 1 ;  4.5 1.5 4.5 2.5 }

line_width 0.4
polyline { 1 0 1 1 ;  1 2.5 1 1.5 ;  8.9 0.5 8.9 2 9.1 2 9.1 0.5 8.9 0.5 ;
           0.55 1.25 0.4 1.35 }

% cátodo
line_width 0.2
polyline { 0.5 0.8 0.7 0.8 ;  0.7 1.7 0.5 1.7 }
line_width 1.6
polyline { 0.7 0.8 0.7 1.7 }

% caja del tubo
line_width 0.4
polyline { 5.7 1.15  5.7 0  0 0  0 2.5  5.7 2.5  5.7 1.35 }

% Detector girado 37° y a escala
Detector(at=(7, -0.2), scale=0.2, rotate=37)

% Pila ajustada a su caja
fit(Pila, stretch=true) { 2 -0.4  2.6 0 }

% cota
line_width 0.1
polyline(marker_end="arrow", marker_end_shift=0.75) { 8 1.25  8.9 1.25  7.19 -0.0552 }

% Rayos catódicos — ⚠ params por calibrar
{
line_width 0.2
dash "shortdashed"
for i = 0 to 9 {
  y = 0.8 + i*0.1
  % w se fija por defecto (8.9) y las ramas lo modifican: una asignación dentro
  % de un if altera el w de afuera (alcance de bloque), así que el caso restante
  % (i==4,5) no necesita un else. ⚠ fronteras y anchos por calibrar.
  w = 8.9
  if i < 3 or i > 6      { w = 1.0 }
  else if i==3 or i==6   { w = 5.7 }
  polyline { 0.76 y w y }
}
}

% Flechas de haz: sobre arco (both_sides) y sobre línea, con from/to nombrados,
% igual que en `arc`.
color "blue"
arc(2.39, from=197.35, to=237.35, marker_size=3, marker_end="arrow", marker_start_orient="reverse", marker_start="arrow") { 8.9 1.25 }
color "black"

% Arco del ángulo φ
arc(0.7, from=180, to=218) { 8.9 1.25 }

% ── Etiquetas ────────────────────────────────────────────────────────────────
text("cathode")  { 0.1 2.70 }
text("anode")    { 4 2.70 }
text("crystal")  { 9.3 1.75 }
text("detector") { 7.8 -0.50 }
text("$V$")      { 2.20 -1.0 }
text("+")        { 2.55 -0.75 }
text("$\varphi$")   { 8 0.9 }
