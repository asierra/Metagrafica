% Tiro parabólico — una trayectoria muestreada punto a punto.
%
% Tres cosas a la vez: `path +=` construye la curva dentro de un `for`, y las
% mismas coordenadas alimentan las proyecciones a cada eje; la malla no es
% regular ni logarítmica, sino que cae donde la física pone los puntos; y el
% cañón es una struct con la BOCA en su origen local, colocada con la misma
% variable que fija el origen de la trayectoria, así que mover y0 mueve el cañón
% y la curva JUNTOS.
%
% NOTAS --------------------------------------------------------------------
% Y ES UNA PARÁBOLA DE VERDAD, no la ilustración de una. La trayectoria sale de
% y = y0 − k·(x−x0)², así que k = (y0−y)/(x−x0)² es CONSTANTE en los siete puntos
% (0.06838) — verificado. La figura de referencia que inspiró este ejemplo
% dibujaba un arco que se PARECE a una trayectoria; medido, es un semicírculo,
% que se desvía unas 4 unidades de la parábola en el punto medio. No es que se
% parezca menos: es una curva cualitativamente distinta. Ilustrar y calcular
% producen curvas diferentes, y solo la calculada es correcta — la tesis de
% docs/calcular_en_vez_de_medir.md, en pequeño.

display_size 13 7
world_window -2 11 2 9

% cañón esquemático: boca en el origen local (0,0), apunta a +x
struct canon() {
    compound(fill="black") {
        arc(0.5, from=90, to=270) { -2 0 }
        polyline { -2 -0.5   -0.25 -0.3   -0.25 0.3  -2 0.5 }
     }    
    circle(0.62, fill="#5a3a1c", color="black") { -1.5 -0.6 }                        % rueda
    circle(0.28, fill="#2a2a2a") { -1.5 -0.6 }                                       % cubo
}
x0 = 1        % boca del cañón = origen de la trayectoria
y0 = 8
y1 = 2.4615
n  = 6

path tray  = { (x0) (y0) }
path proyx = { (x0) (y0) }     % proyección al eje IZQUIERDO (misma y)
path proyy = { (x0) (y0) }     % proyección al borde SUPERIOR (misma x)

% malla ajustada a los datos + acumulación de puntos, todo en el mismo lazo
line_width 0.3   color "gray"   dash "dotted"
for i = 1 to n {
    x = x0 + i*1.5
    y = y0 - i*i/6.5
    path tray  += { (x) (y) }
    path proyx += { (x0) (y) }
    path proyy += { (x) (y0) }
    polyline { (x) (y1)  (x) (y0) }        % vertical: piso → borde superior
    polyline { (x0) (y)  (10) (y) }     % horizontal: eje y → borde derecho
}

% ejes en L (eje y izquierdo + borde superior)
dash "solid"   line_width 0.5   color "black"
polyline { (x0) (y1) (x0) (y0)  (10) (y0) }

% cañón, con la boca en (x0,y0)
canon(at=(x0, y0))

% plataforma
rectangle(fill="green") { -1.75 (y1) 0.75 6.75  }
 
% trayectoria y puntos, encima de todo
line_width 1.5   color "black"   dash "dashed"   smooth(&tray)
dash "solid"
dot(&tray,  size=2.6, color="lime")
dot(&proyx, size=2.6, color="orange")
dot(&proyy, size=2.6, color="blue")
