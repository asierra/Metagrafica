% Tiro parabólico — una trayectoria muestreada punto a punto.
%
% El origen está en la boca del cañón y la parábola se escribe tal cual,
% y = −k·x². Un `for` calcula cada punto y con `path +=` arma la curva y sus
% proyecciones a los dos ejes; en el mismo lazo se dibuja la malla, que no es
% regular ni logarítmica, sino que cae donde la física pone los puntos. El cañón
% es una struct con la boca en su origen local, así que se coloca en (0, 0), y
% con `scale=` se ajusta su tamaño a la escala de la trayectoria.
%
% NOTAS --------------------------------------------------------------------
% Y ES UNA PARÁBOLA DE VERDAD, no la ilustración de una: `k` es la misma en todos
% los puntos porque la define el código. La figura de referencia que inspiró este
% ejemplo dibujaba un arco que se PARECE a una trayectoria; medido, es un
% semicírculo, que se desvía unos 4 cm de la parábola en el punto medio. No
% es que se parezca menos: es una curva cualitativamente distinta. Ilustrar y
% calcular producen curvas diferentes, y solo la calculada es correcta — la tesis
% de docs/calcular_en_vez_de_medir.md, en pequeño.

display_size 13 7
world_window -2 6.67 -4 0.67

% cañón esquemático: boca en el origen local (0,0), apunta a +x
struct canon() {
    compound(fill="black") {
        arc(0.5, from=90, to=270) { -2 0 }
        polyline { -2 -0.5   -0.25 -0.3   -0.25 0.3  -2 0.5 }
    }
    circle(0.62, fill="#5a3a1c", color="black") { -1.5 -0.6 }     % rueda
    circle(0.28, fill="#2a2a2a") { -1.5 -0.6 }                    % cubo
}

n = 6          % puntos de la trayectoria
k = 1/10       % la parábola: y = −k·x²
piso = -k*n*n  % donde cae el último punto

path tray  = { 0 0 }
path proyx = { 0 0 }     % proyección al eje izquierdo (misma y)
path proyy = { 0 0 }     % proyección al borde superior (misma x)

% malla ajustada a los datos + acumulación de puntos, todo en el mismo lazo
line_width 0.3   color "gray"   dash "dotted"
for i = 1 to n {
    x = i
    y = -k*x*x
    path tray  += { x y }
    path proyx += { 0 y }
    path proyy += { x 0 }
    polyline { x piso  x 0 }       % vertical: piso → borde superior
    polyline { 0 y  n y }          % horizontal: eje y → borde derecho
}

% ejes en L (eje y izquierdo + borde superior)
dash "solid"   line_width 0.5   color "black"
polyline { 0 piso  0 0  n 0 }

% El cañón está dibujado a 1 cm por unidad, pero esta ventana pone 1.5 cm por
% unidad (6 unidades de trayectoria en 9 cm), así que se reduce a 2/3 para que
% guarde su proporción; la plataforma, igual.
canon(at=(0, 0), scale=2/3)
rectangle(fill="lime") { -1.83 piso  -0.17 -0.83 }     % plataforma

% trayectoria y puntos, encima de todo
line_width 1.5   color "black"   dash "dashed"   smooth(&tray)
dash "solid"
dot(&tray,  size=2.6, color="gray")
dot(&proyx, size=2.6, color="red")
dot(&proyy, size=2.6, color="green")
