% Diagrama de difracción de electrones (experimento de Davisson-Germer).
%
% PROCEDENCIA — la figura tiene DOS números, uno por libro:
%   · **Figura 2.5** de Cetto & de la Peña, "Quantum Mechanics: A Physical Approach"
%     (Cambridge University Press, 2025) — https://doi.org/10.1017/9781009679633.
%     Es la numeración que sigue el NOMBRE de este archivo.
%   · Fig. 2.5 de IMQ (el libro en español). Coinciden por casualidad: el archivo se
%     llamaba `fig2-6` porque esa era su numeración en un borrador de Cambridge, y al
%     eliminarse una figura anterior del capítulo pasó a ser la 2.5 (renombrado
%     2026-07-16). Su fuente V1 sigue siendo `examples/v1/fig2-6.mg` — el corpus V1
%     está congelado y conserva los nombres viejos.
%
% Es el ejemplo de ILUSTRACIÓN del README (el otro es quickstart.mg, una gráfica).
% Si lo cambias, regenera su SVG:
%
%     bin/mg examples/fig2-5.mg docs/img/fig2-5.svg
%
% Traducción V3 de examples/v1/fig2-6.mg; render de referencia (V1):
% examples/v1/reference/fig2-6.svg. NO es idéntico y no puede serlo: el texto
% matemático usa Latin Modern Math (V1 usaba Times + la fuente `symbol`), así que la
% φ tiene otro trazo e inclinación. Es un cambio deliberado del proyecto.
%
% Valida piezas de diseño que ninguna otra ilustración usaba:
%  · rotate= en la invocación: el Detector se coloca girado 37° (escala uniforme,
%    así que rotación y escala conmutan → sin ambigüedad de orden);
%  · MARCADORES declarativos en vez de flechas colocadas a mano. El giro del
%    detector es un `arc(marker_end/start="arrow", marker_start_orient="reverse")`
%    (flecha curva bidireccional); el haz y la cota son `polyline(marker_end="arrow"
%    …)` con `marker_end_shift` para meter la punta adentro. Los marcadores se
%    ORIENTAN SOLOS a la tangente local y se anclan al vértice: CERO ángulos y
%    posiciones calculados a mano. En V1 esto era SCST/RTST (calcular el ángulo y
%    rotar la punta); en el paso intermedio, `place(Arrowr)` con puntos de locus
%    escogidos a ojo. Ahora es una propiedad del propio trazo y NO incluye arrow.mg.

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

% Detector girado 37° y a escala (V1 SCST .1 .1 + RTST 37)
Detector(at=(7, -0.2), scale=0.2, rotate=37)

% Pila ajustada a su caja (V1 PWST)
fit(Pila, stretch=true) { 2 -0.4  2.6 0 }

% cota
line_width 0.1
polyline(marker_end="arrow", marker_end_shift=0.75) { 8 1.25  8.9 1.25  7.19 -0.0552 }

% Rayos catódicos (V1 TICKS punteados) — ⚠ params por calibrar
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

% Flechas de haz: sobre arco (both_sides) y sobre línea. Antes ARCST/LNST con
% 5 números; ahora place con from/to nombrados (como `arc`, §4.5) y both_sides.
color "blue"
arc(2.39, from=197.35, to=237.35, marker_size=3, marker_end="arrow", marker_start_orient="reverse", marker_start="arrow") { 8.9 1.25 }
color "black"

% Arco del ángulo φ (V1 CR .7 38 180)
arc(0.7, from=180, to=218) { 8.9 1.25 }

% ── Etiquetas ────────────────────────────────────────────────────────────────
text("cathode")  { 0.1 2.70 }
text("anode")    { 4 2.70 }
text("crystal")  { 9.3 1.75 }
text("detector") { 7.8 -0.50 }
text("$V$")      { 2.20 -1.0 }
text("+")        { 2.55 -0.75 }
text("$\varphi$")   { 8 0.9 }
