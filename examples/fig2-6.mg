% Fig. 2.5 de IMQ — Diagrama de difracción de electrones.
% Traducción V3 de examples/v1/fig2-6.mg. Render objetivo:
% examples/v1/reference/fig2-6.svg.
%
% Valida dos piezas de diseño que ninguna otra figura usaba:
%  · rotate= en la invocación: el Detector se coloca girado 37° (escala uniforme,
%    así que rotación y escala conmutan → sin ambigüedad de orden);
%  · place sobre ARCO: los ARCST de V1 (5 números posicionales) quedan claros con
%    from/to nombrados —igual que `arc`— y both_sides explícito (V1: escala < 0).
% ⚠ Sólo verificación de render (parser V3): rayos con ticks y cotas por calibrar.

display_size 12 4
font_size 8
include "arrow.mg"

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

% Detector girado 37° y a escala (V1 SCST .1 .1 + RTST 37)
Detector(at=(7, -0.2), scale=0.2, rotate=37)
dot(1.0) { 7 -0.2 }
% Pila ajustada a su caja (V1 PWST)
fit(Pila, stretch=true) { 2 -0.4  2.6 0 }

% cota
line_width 0.1
polyline { 8 1.25  8.9 1.25  7.19 -0.0552 }

% Rayos catódicos (V1 TICKS punteados) — ⚠ params por calibrar
{
line_width 0.2
dash "dashed"
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

% Flechas de haz: sobre arco (both_sides) y sobre línea. Antes ARCST/LNST con
% 5 números; ahora place con from/to nombrados (como `arc`, §4.5) y both_sides.
color "blue"
place(Arrowr, scale=0.04, both_sides=true, r=2.39, from=197.35, to=237.35) { 8.9 1.25 }
color "black"
place(Arrowr, scale=0.04) { 8.9 1.25  7.5 0.1816 }

% Arco del ángulo φ (V1 CR .7 38 180)
arc(0.7, from=180, to=218) { 8.9 1.25 }

% ── Etiquetas ────────────────────────────────────────────────────────────────
text("cathode")  { 0.1 2.70 }
text("anode")    { 4 2.70 }
text("crystal")  { 9.3 1.75 }
text("detector") { 7.8 -0.50 }
text("$V$")      { 2.20 -1.0 }
text("+")        { 2.55 -0.75 }
text("$\phi$")   { 8 0.9 }
