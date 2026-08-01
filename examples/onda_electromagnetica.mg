% Onda electromagnética — E y B en planos perpendiculares
%
% Dos ondas en fase, cada una en su plano: el campo eléctrico en el vertical, el
% magnético en el horizontal, propagándose a lo largo de x. Cada onda es un `sine`
% corriente DENTRO de un `plane3d`, rellena media onda por media onda, y sus peines
% y su cota son polilíneas 2-D del mismo plano: no se muestrea ni se proyecta nada a
% mano, y las puntas de los peines se orientan solas. Lo único que no pertenece a
% ningún plano es el eje, que va con `xyz()`.
%
% NOTAS ————————————————————————————————————————————————————————————————
%
% PROCEDENCIA: Lira, fig. II-1 («señal u onda electromagnética»). RECONSTRUIDA, no
% digitalizada: el original es un dibujo a mano sin cámara que despejar, así que la
% suya (acimut −25°, elevación 18°) se eligió para que la página se lea como la
% publicada. Lo que sí es exacto es todo lo demás. El original es line-art; el color
% es una decisión de esta versión.
%
% LAS DOS AMPLITUDES SON DISTINTAS, Y EL FACTOR SALE DE LA CÁMARA. El plano de B es
% el horizontal, que la proyección escorza; con la misma amplitud que E saldría más
% corto y la figura sugeriría que un campo es menor que el otro, que sería decir algo
% falso. Se pide que las dos se vean IGUALES de largas y eso se despeja:
%     |proj(y^)| = cos(el)
%     |proj(z^)| = hypot(sin(az), cos(az)*sin(el))
%     ampb = amp * |proj(y^)| / |proj(z^)|
% Lo comparable en el dibujo es la LONGITUD: E y B tienen unidades distintas y su
% razón física no es dibujable, así que igualar lo que se ve es la lectura honesta.
% Como sale de la cámara, sigue valiendo si se cambia.
%
% B ARRANCA HACIA EL OBSERVADOR (−z), que es lo que pone su primer lóbulo arriba a la
% derecha, como el original. Es convención de signo, no geometría: E y B solo tienen
% que ser perpendiculares.
%
% LA COTA DE LAMBDA VA DENTRO DEL PLANO DE E, no en la página. Así queda paralela al
% eje —que desciende con esta cámara— y sus dos patas miden lo mismo sin una sola
% cuenta: bajo `plane3d` un segmento vertical del plano se proyecta con la misma
% longitud sea cual sea su x (verificado: 26.458 y 26.457 pt).
%
% ORDEN DE PINTADO, que aquí es toda la oclusión que hay: rellenos, luego el eje
% —si va antes lo tapan, y es la línea que organiza la figura—, luego los peines y
% al final los trazos de las ondas.
%
% COBERTURA EXCLUSIVA: es el único ejemplo con un GENERADOR y un RELLENO dentro de un
% `plane3d` (§13 promete que el dibujo 2-D corriente funciona ahí dentro, y sin esta
% figura esa promesa no tiene prueba), y el único con MARCADORES bajo una matriz no
% conforme — que es la familia de bugs «fórmula isótropa en el caso anisótropo», la
% más recurrente del proyecto. Aquí sale limpia: las puntas de E apuntan verticales
% (su dirección es proj(y^)) y las de B arriba-derecha (proj(−z^)).
%
% ⚠️ Los rellenos van con `bezier(&p, fill=)` y NO con `polygon(&p, fill=)`: el
% generador `sine` devuelve puntos de CONTROL de Bézier, y `polygon` los tomaría como
% vértices — el relleno saldría con esquinas rectas, una cometa en vez del lóbulo.

display_size 14 9
world_window -0.7 9.0 -3.5 3.0

azd = -25                          % acimut
eld = 18                           % elevación
view3d(azimuth=azd, elevation=eld)

nl  = 2                            % longitudes de onda
lam = 4                            % longitud de onda
amp = 1.9                          % amplitud de E
nx  = nl * lam
hc  = lam / 2                      % media longitud de onda

% La amplitud de B se deriva de la cámara para que las dos se vean iguales (NOTAS).
th  = rad(azd)   ph = rad(eld)
ampb = amp * cos(ph) / sqrt(sin(th)*sin(th) + cos(th)*cos(th)*sin(ph)*sin(ph))

azul = "#cdcdff"
rosa = "#ffcdcd"

% --- los rellenos, una media onda por polígono ---
% B (horizontal) primero y E (vertical) después: donde se solapan en la página
% mandan los lóbulos verticales.
{
    plane3d(u=[1, 0, 0], v=[0, 0, 1])
    for k = 0 to (2*nl - 1) {
        a = -ampb
        if mod(k, 2) == 1 { a = ampb }
        path b = sine(half_cycles=1, amplitude=(a)) { (k*hc) 0  (k*hc + hc) 0 }
        bezier(&b, fill=rosa)
    }
}
{
    plane3d(u=[1, 0, 0], v=[0, 1, 0])
    for k = 0 to (2*nl - 1) {
        a = amp
        if mod(k, 2) == 1 { a = -amp }
        path e = sine(half_cycles=1, amplitude=(a)) { (k*hc) 0  (k*hc + hc) 0 }
        bezier(&e, fill=azul)
    }
}

% --- el eje de propagación ---
color "black"
line_width 0.4
polyline(marker_end="arrow", marker_size=4) { xyz(-0.4, 0, 0)   xyz(nx+0.9, 0, 0) }

% --- los peines: uno por lóbulo, en su extremo, con punta ---
% Van en el plano de su propia onda, así que son polilíneas 2-D corrientes y la punta
% se orienta sola con la tangente.
line_width 0.3
{
    plane3d(u=[1, 0, 0], v=[0, 0, 1])
    for k = 0 to (2*nl - 1) {
        x = k*hc + hc/2
        polyline(dash="dotted", marker_end="triangle", marker_orient="auto", marker_size=3.5) {
            (x) 0   (x) (-ampb * sin(x * pi / hc))
        }
    }
}
{
    plane3d(u=[1, 0, 0], v=[0, 1, 0])
    for k = 0 to (2*nl - 1) {
        x = k*hc + hc/2
        polyline(dash="dotted", marker_end="triangle", marker_orient="auto", marker_size=3.5) {
            (x) 0   (x) (amp * sin(x * pi / hc))
        }
    }
}

% --- los trazos de las dos ondas, encima de todo ---
line_width 1.1
{
    plane3d(u=[1, 0, 0], v=[0, 0, 1])
    sine(half_cycles=(2*nl), amplitude=(-ampb)) { 0 0  (nx) 0 }
}
{
    plane3d(u=[1, 0, 0], v=[0, 1, 0])
    sine(half_cycles=(2*nl), amplitude=(amp)) { 0 0  (nx) 0 }
}

% --- la cota de lambda, de cresta a cresta del campo eléctrico ---
x1 = lam/4
x2 = lam/4 + lam
yb = amp + 0.75
line_width 0.3
{
    plane3d(u=[1, 0, 0], v=[0, 1, 0])
    polyline { (x1) (amp + 0.12)   (x1) (yb + 0.12) }
    polyline { (x2) (amp + 0.12)   (x2) (yb + 0.12) }
    polyline(marker_start="arrow", marker_end="arrow", marker_size=3.5, marker_start_orient="reverse") {
        (x1) (yb)   (x2) (yb)
    }
}

% --- rótulos ---
% Cada uno se cuelga de un punto de la ESCENA y se aparta de él en la página, así que
% sigue puesto si se cambia la cámara.
pL = xyz((x1+x2)/2, yb, 0)
pE = xyz(x1, amp, 0)
pB = xyz(x1, 0, -ampb)
pF = xyz(nx, 0, 0)

font_size 12
align "center"
text("$\lambda$") { (pL[0]) (pL[1] + 0.22) }

align "right"
text("$E$") { (pE[0] - 0.35) (pE[1] - 0.12) }
text("$B$") { (pB[0] - 0.25) (pB[1] + 0.12) }

align "left"
font_size 10
text("Dirección de/npropagación") { (pF[0] + 0.15) (pF[1] + 0.45) }
