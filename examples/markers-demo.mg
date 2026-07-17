% markers-demo.mg — Demostración V3 de los marcadores físicos (§4.6): las 6 formas
% del catálogo, la orientación tangente sobre una curva (shape="arrow" se orienta
% por default) y el color independiente marcador/curva.
%
% Ejecutar desde examples/:  ../bin/mg markers-demo.mg markers-demo.eps
%
% `marker(r, shape=…)` es la primitiva; `dot(r)` es su atajo para el disco y no
% lleva shape. Ojo con los dos `marker` de este archivo, que NO son lo mismo:
% `marker(shape=…)` es la PRIMITIVA (estampa un símbolo por punto), mientras que
% `polyline(marker=…)` es el ATRIBUTO que decora los vértices de una curva. Ahí el
% argumento se llama `marker` porque nombra QUÉ se pone; en la primitiva se llama
% `shape` porque la primitiva ya ES el marcador.
%
% El tamaño es FÍSICO (size, en puntos): inmune a la ventana, solo la posición se
% transforma. arrow se orienta a la tangente local por default; el resto queda fijo
% (sobreescribible con marker_orient="auto"|"fixed").
%
% Relleno como en los círculos (§4.6): a secas = relleno; color= sin fill= = ABIERTO
% (solo contorno); fill= = relleno en ese color. cross/x son siempre contorno
% (geometría no cerrada). No hay forma "disk": el disco es `circle` relleno.

display_size 12 12
world_window 0 12 0 12

align "center"
text("{/bMarcadores estándar y orientación tangente}") { 6 11.4 }

% --- 1. Fila con los 6 marcadores estándar con sus defaults ---
marker(size=8, shape="circle")  { 1.5 10 }
marker(size=8, shape="square")  { 3.5 10 }
marker(size=8, shape="diamond") { 5.5 10 }
marker(size=8, shape="cross")   { 7.5 10 }
marker(size=8, shape="x")       { 9.5 10 }
marker(size=8, shape="arrow")   { 11.5 10 }

% --- 2. Flechas orientadas siguiendo la tangente de una curva ---
color "blue"
polyline(marker="arrow", marker_size=10) { 1 6  3 8  6 8  9 6  11 5 }

% --- 3. Dispersión: curva roja, puntos negros (marcador ≠ color de curva) ---
color "red"
polyline(marker="circle", marker_size=5, marker_color="black", marker_fill="none") { 1 2.5  3 3.7  5 2.9  7 4.1  9 3.3  11 3.9 }

% --- 4. Dispersión sin curva: rombos ABIERTOS (color= sin fill=) en otro color ---
marker(size=7, shape="diamond", color="green") { 1 1  3 1.6  5 0.8  7 1.7  9 1.1  11 1.4 }
