% Ondas — la primitiva `sine`.
%
% Cada `sine` recibe una base de dos puntos, un número de medios ciclos y una
% amplitud, y genera por dentro las curvas bézier: la onda oscila perpendicular a
% su base, sea cual sea su orientación. Aquí se ven la fase 0 y la variante
% cuadrada.
%
% NOTAS --------------------------------------------------------------------
% `phase` (múltiplos de 90°: coseno, etc.) está implementado por cuartos de
% ciclo. `sine` sirve también como expresión del álgebra de trayectos, no solo
% como primitiva que dibuja — ver franck_condon.mg.

display_size 12 10
font_size 9

world_window 0 12 0 10

% líneas base tenues (la onda oscila alrededor de ellas)
line_width 0.1
color "lightgray"
polyline { 1 9  11 9 }   polyline { 1 8  11 8 }   polyline { 1 7  11 7 }   polyline { 1 6  11 6 }
polyline { 1 4.5  11 4.5 }   polyline { 1 3.5  11 3.5 }   polyline { 1 2.5  11 2.5 }

% --- φ = sin(nπx): medios ciclos n = 1..4, jorobas de signo alterno ---
line_width 0.5
color "blue"
sine(half_cycles=1, amplitude=0.4) { 1 9  11 9 }
sine(half_cycles=2, amplitude=0.4) { 1 8  11 8 }
sine(half_cycles=3, amplitude=0.4) { 1 7  11 7 }
sine(half_cycles=4, amplitude=0.4) { 1 6  11 6 }

% --- ρ = |φ|² (squared): jorobas todas positivas ---
color "red"
sine(half_cycles=1, amplitude=0.6, squared=true) { 1 4.5  11 4.5 }
sine(half_cycles=2, amplitude=0.6, squared=true) { 1 3.5  11 3.5 }
sine(half_cycles=3, amplitude=0.6, squared=true) { 1 2.5  11 2.5 }

% --- Base inclinada y vertical: la onda oscila perpendicular a la base ---
color "green"
sine(half_cycles=3, amplitude=0.35) { 0.5 0.4  5 1.7 }   % diagonal
sine(half_cycles=2, amplitude=0.35) { 7 0.3  7 2 }       % vertical

% --- Etiquetas ---
color "black"
text("$\phi = sin(n \pi x),  n = 1..4$") { 1 9.6 }
text("$\rho = |\phi|^2$  (squared=true)") { 1 5.1 }
text("base diagonal / vertical") { 0.3 0.05 }
