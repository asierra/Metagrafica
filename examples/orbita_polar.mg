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
  % El giro tiene que ser ALREDEDOR DEL CENTRO DE LA TIERRA, no del origen del
  % mundo: `rotate` gira el plano entero, así que con el centro en {0 earth_y} el
  % propio centro se desplazaba y las dos órbitas quedaban descentradas 2.6 mm
  % hacia lados opuestos. Se lleva el origen al centro con `translate` y las
  % elipses se colocan ya en {0 0}. Como las sentencias componen en el orden
  % escrito (§11.1), esto es T·R: primero rota, luego traslada.
  translate 0 earth_y
  rotate 15
  % marker_at pone los marcadores en ÁNGULOS del arco (el mismo parámetro que
  % from/to), no solo en los extremos, y cada uno se orienta a la tangente. 0 y 180
  % son los extremos del eje menor: los puntos más anchos, donde la flecha de
  % sentido se lee mejor.
  % Problema: usar las funciones de intersección o algebra booleana de paths para no trazar la
  % parte oculta de la órbita.
  ellipse(axis_x, axis_y, marker="arrow", marker_at=[0, 180], marker_size=4) { 0 0 }
  % El satélite se coloca SOBRE la órbita por su ángulo, alineado a la tangente: ya
  % no hay que calcular la posición a mano. Va dentro del bloque para que le toque
  % la misma matriz que a la elipse, así que basta el mismo centro { 0 0 }.
  place(Satellite, rx=axis_x, ry=axis_y, at=[30], scale=0.8) { 0 0 }
  rotate -30
  ellipse(axis_x, axis_y, marker="arrow", marker_at=[0, 180], marker_size=4) { 0 0 }
}
text("800 Km", align="right") { -3.5 5.5 }
