% ─────────────────────────────────────────────────────────────────────────────
% Tiro parabólico — una trayectoria muestreada punto a punto.
%
% Ilustra tres cosas del lenguaje a la vez:
%   · `path +=` construye la curva punto a punto en un `for` (§9), y las mismas
%     coordenadas alimentan las proyecciones a cada eje.
%   · la malla NO es un `grid` regular ni una escala log: en x el paso es
%     constante, pero en y los puntos caen donde la física los pone (Δy CRECE).
%     Una escala log ni siquiera existiría —la trayectoria llega a y<0—. Las
%     líneas guía se dibujan en el mismo lazo, ajustadas a los datos.
%   · el cañón es una `struct` con la BOCA en su origen local (0,0): se ensambla
%     solo y se coloca con `at=(x0,y0)`, la misma variable que fija el origen de
%     la trayectoria. Mover `y0` mueve cañón y curva JUNTOS.
% ─────────────────────────────────────────────────────────────────────────────
display_size 15 11
world_window -2 11 -0.5 9.5

% cañón esquemático: boca en el origen local (0,0), apunta a +x
struct canon() {
    polygon(fill="#7a4a20", color="black") { -2.0 -0.45  -0.7 -0.45  -1.55 -2.4 }   % cureña
    circle(0.62, fill="#5a3a1c", color="black") { -1.4 -2.4 }                        % rueda
    circle(0.28, fill="#2a2a2a") { -1.4 -2.4 }                                       % cubo
    rectangle(fill="#3c3c3c", color="black") { -2.0 -0.45  0.05 0.45 }              % tubo
    rectangle(fill="#1c1c1c") { 0.05 -0.30  0.35 0.30 }                             % anillo de boca
}

x0 = 1        % boca del cañón = origen de la trayectoria
y0 = 8
n  = 6

path tray  = { (x0) (y0) }
path proyx = { (x0) (y0) }     % proyección al eje IZQUIERDO (misma y)
path proyy = { (x0) (y0) }     % proyección al borde SUPERIOR (misma x)

% malla ajustada a los datos + acumulación de puntos, todo en el mismo lazo
line_width 0.3   color "gray"   dash "dotted"
for i = 1 to n {
    x = x0 + i
    y = y0 - i*i/6.5
    path tray  += { (x) (y) }
    path proyx += { (x0) (y) }
    path proyy += { (x) (y0) }
    polyline { (x) 0  (x) (y0) }        % vertical: piso → borde superior
    polyline { (x0) (y)  (10) (y) }     % horizontal: eje y → borde derecho
}

% ejes en L (eje y izquierdo + borde superior)
dash "solid"   line_width 0.5   color "black"
polyline { (x0) 0  (x0) (y0)  (10) (y0) }

% cañón, con la boca en (x0,y0)
canon(at=(x0, y0))

% trayectoria y puntos, encima de todo
line_width 1.5   color "green"   dash "dashed"   smooth(&tray)
dash "solid"
dot(&tray,  size=2.6, color="green")
dot(&proyx, size=2.6, color="orange")
dot(&proyy, size=2.6, color="blue")
