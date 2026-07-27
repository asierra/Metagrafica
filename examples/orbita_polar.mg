display_size 12 16
world_window -6 6 -8 8

include "../lib/mapa_p30_n55.mg"
include "../lib/satellite.mg"

% axis
polyline(dash="dashed", line_width=0.1) { 0 -7  0 6.75 }
text("N", align="center", valign="midle") { 0 7 }
text("S", align="center", valign="midle") { 0 -7.5 }
arrow_rx = 1.25
arrow_ry = 0.15
text("Rotación de la Tierra", size=8, align="right") { -arrow_rx -6.5 }
arc(rx=arrow_rx, ry=arrow_ry, line_width=2, color="gray", from=90, to=270,
    marker_end="arrow", marker_size=3) { (0.5*arrow_rx) -6.5 }

% planet
earth_radius = 5
earth_y = .5
%circle(earth_radius, fill="steelblue") { 0 earth_y }
Mapa(scale=earth_radius, at=(0, earth_y), grid=false)


% orbits
{
  axis_x = 3.4
  axis_y = 6.2
  rotate 15
  % Problema: colocar flechas sobre nodos de la curva en la misma dirección
  % Problema: usar las funciones de intersección o algebra booleana de paths para no trazar la
  % parte oculta de la órbita.
  ellipse(axis_x, axis_y) { 0 earth_y }
  rotate -30
  ellipse(axis_x, axis_y) { 0 earth_y }
}
text("800 Km", align="right") { -3.5 5.5 }

% Problema: usar una de las nuevas funciones path para obtener un punto sobre la órbita y
% colocar ahí el satélite y así no tener el usuario que calcular su posición.
% Problema: el arco de la antena no rota bien, tal vez mismo problema que tenía la elipse
Satellite(scale=0.8,rotate=10,at=(3.5, 5.5))
