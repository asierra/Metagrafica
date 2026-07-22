% Parábola (calculada) vs arco de círculo (lo que se dibuja a mano).
% El círculo es el MÁS JUSTO: mismo lanzamiento, misma llegada, y misma
% dirección inicial (tangente horizontal) que la parábola.
display_size 13 8
world_window 0 11 1.5 9

x0 = 1    y0 = 8    x1 = 10
k  = 0.0684
y1 = y0 - k*(x1-x0)*(x1-x0)

% parábola, por puntos (calculada de y = y0 - k(x-x0)²)
path par = { (x0) (y0) }
for i = 1 to 18 {
  x = x0 + i*(x1-x0)/18
  path par += { (x) (y0 - k*(x-x0)*(x-x0)) }
}

% círculo tangente a la horizontal en el lanzamiento, por los dos extremos:
% centro justo debajo del lanzamiento, R = (dx²+dy²)/(-2dy)
dx = x1 - x0    dy = y1 - y0
R  = 0 - (dx*dx + dy*dy)/(2*dy)
cx = x0    cy = y0 - R
aend = atan2(y1 - cy, x1 - cx) * 180 / pi

% el arco (gris, punteado) y la parábola (negra) encima
line_width 1.0   color "gray"   dash "dashed"
arc(R, from=90, to=aend) { (cx) (cy) }
dash "solid"   line_width 1.4   color "black"
polyline(&par)

% extremos compartidos
dot(2.6, color="blue") { (x0) (y0)  (x1) (y1) }
