% Marcadores — las siete formas del catálogo, y cómo se orientan.
%
% Las formas, la orientación tangente sobre una curva (`arrow` se orienta sola), el
% color independiente entre marcador y curva, y DÓNDE se ponen: en todos los vértices
% o solo al principio, en medio y al final. El tamaño es FÍSICO, en puntos: inmune a
% la ventana, porque solo la posición se transforma.
%
% NOTAS --------------------------------------------------------------------
% ⚠ COBERTURA EXCLUSIVA: es el único ejemplo que ejercita `marker_mid`. Sus dos
% hermanos tienen quien los use (`marker_start` y `marker_end` salen en fig2-5,
% fig4-1, elevacion_solar y gravitacion_orbita); el de en medio estaba documentado
% en la referencia y sin una sola prueba, y su regresión natural es la peor: que la
% punta desaparezca sin que nada proteste.
% Los dos `marker` de este archivo NO son lo mismo: `marker(shape=…)` es la
% PRIMITIVA (estampa un símbolo por punto), mientras que `polyline(marker=…)` es
% el ATRIBUTO que decora los vértices de una curva. Ahí el argumento se llama
% `marker` porque nombra QUÉ se pone; en la primitiva se llama `shape` porque la
% primitiva ya ES el marcador. `dot(r)` es el atajo del disco y no lleva shape.
%
% triangle es el triángulo relleno estático; arrow es el arpón de contorno
% DIRECCIONAL. Son ejes distintos: forma contra papel. Solo arrow se orienta a la
% tangente por default; el resto queda fijo, y las dos cosas se sobreescriben con
% marker_orient="auto"|"fixed".
%
% Relleno: a secas = relleno; color= sin fill= = ABIERTO (solo contorno); fill= =
% relleno en ese color. cross, x y arrow son siempre contorno, por ser geometría
% a trazo. No hay forma "disk": el disco es un circle relleno.

% La banda de abajo (y negativa) es sitio que se le añadió al lienzo para la sección 5.
% Ventana y lienzo crecen LO MISMO, así que la escala isométrica no se mueve y nada de
% lo que ya estaba cambia de tamaño ni de sitio.
display_size 12 13.6
world_window 0 12 -1.6 12

align "center"
text("{/bMarcadores estándar y orientación tangente}") { 6 11.4 }

% --- 1. Fila con los 7 marcadores estándar con sus defaults ---
marker(size=8, shape="circle")   { 1.2 10 }
marker(size=8, shape="square")   { 2.9 10 }
marker(size=8, shape="diamond")  { 4.6 10 }
marker(size=8, shape="cross")    { 6.3 10 }
marker(size=8, shape="x")        { 8.0 10 }
marker(size=8, shape="triangle") { 9.7 10 }
marker(size=8, shape="arrow")    { 11.4 10 }

% --- 1b. Orientación por ÁNGULO FIJO (marker_orient=grados): el triángulo (▶ por
%     default) rota a ▲ 90 / ◀ 180 / ▼ 270; un ángulo sirve para cualquier forma. ---
marker(size=8, shape="triangle", marker_orient=90)  { 3.5 8.6 }
marker(size=8, shape="triangle", marker_orient=180) { 5.5 8.6 }
marker(size=8, shape="triangle", marker_orient=270) { 7.5 8.6 }
marker(size=8, shape="square",   marker_orient=45)  { 9.5 8.6 }

% --- 2. Flechas orientadas siguiendo la tangente de una curva ---
color "blue"
polyline(marker="arrow", marker_size=10) { 1 6  3 8  6 8  9 6  11 5 }

% --- 3. Dispersión: curva roja, puntos negros (marcador ≠ color de curva) ---
color "red"
polyline(marker="circle", marker_size=5, marker_color="black", marker_fill="none") { 1 2.5  3 3.7  5 2.9  7 4.1  9 3.3  11 3.9 }

% --- 4. Dispersión sin curva: rombos ABIERTOS (color= sin fill=) en otro color ---
marker(size=7, shape="diamond", color="green") { 1 1  3 1.6  5 0.8  7 1.7  9 1.1  11 1.4 }

% --- 5. DÓNDE se pone el marcador: marker_start / marker_mid / marker_end ---
% Contrasta con la sección 2: ahí `marker=` decora TODOS los vértices; aquí cada
% posición se nombra aparte. `marker_mid` toma los INTERMEDIOS —los dos de en medio de
% esta polilínea de cuatro puntos—, así que un vértice de más es un marcador de más:
% es la manera de poner una punta a media línea sin partirla en dos.
color "black"
polyline(marker_start="circle", marker_mid="arrow", marker_end="arrow", marker_size=8) {
    1 -0.5  4.5 -1.1  8 -0.5  11 -1.1 }
