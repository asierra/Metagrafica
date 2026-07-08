% Fig. 7.1 de IMQ — Método WKB.
% Traducción V3 de examples/v1/fig6-1.mg. Render objetivo:
% examples/v1/reference/fig6-1.svg.
%
% La curva WKB (V1: PWPT cos2pi + bzsinepaths) es ahora un `sine` con fase de
% coseno (§4.13): un periodo de coseno ajustado al ancho. Sin include ni fitrect.
% ⚠ amplitudes/posiciones por calibrar contra el oráculo (parser V3 pendiente).

display_size 12 3.4
font_size 8

world_window 0 12 -0.7 2.7

% Pozos de potencial (barreras), tramados a 45°  (V1 FPATRN 2)
line_width 0.1
polygon(hatch=45) { 2.5 0  3.5 0  3.5 0.5  3.15 0.5  3.15 0.8  3.5 0.8  3.5 1.7  2.5 1.7 }
polygon(hatch=45) { 8.5 0  9.5 0  9.5 0.5  9.15 0.5  9.15 0.8  9.5 0.8  9.5 1.7  8.5 1.7 }

% Ejes y fronteras de región (subtrayectos con ;)
polyline { 0.2 0 11.2 0 ;  0.2 1.7 11.2 1.7 ;  3 -0.4 3 2 ;  9 -0.4 9 2 }
% Puntos de retorno (líneas punteadas)
polyline(dash="dashed") { 2.5 0 2.5 2 ;  3.5 0 3.5 1.7 ;  8.5 0 8.5 1.7 ;  9.5 0 9.5 2 }

% Función de onda WKB: un periodo de coseno (phase=90) sobre la base y=1.4
line_width 0.8
sine(half_cycles=2, phase=90, amplitude=1) { 1 1.4  11 1.4 }

% ── Etiquetas ────────────────────────────────────────────────────────────────
% Regiones I / II / III (V1: pluma XYPP 1.5 1.2 + TLPP 4.5 0)
text("I")   { 1.5 1.2 }
text("II")  { 6 1.2 }
text("III") { 10.5 1.2 }
% ψ por región
text("$\psi_I$")   { 1.4 0.13 }
text("$\psi_{II}$"){ 5.9 0.13 }
text("$\psi_{III}$"){ 10.4 0.13 }
% ψ en los pozos, puntos de retorno x, potencial y energía
text("$\psi_1$") { 3.2 0.6 }
text("$\psi_2$") { 9.2 0.6 }
text("$x_1'$")   { 2.3 -0.3 }
text("$x_2'$")   { 8.3 -0.3 }
text("$x_1''$")  { 3.3 -0.3 }
text("$x_2''$")  { 9.3 -0.3 }
text("$x_1$")    { 2.9 -0.62 }
text("$x_2$")    { 8.9 -0.62 }
text("$V(x)$")   { 11.1 2.3 }
text("$E$")      { 11.3 1.6 }
