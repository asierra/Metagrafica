% Órbita circular — gravitación y fuerza centrípeta
%
% Un satélite en órbita circular alrededor de un planeta: la fuerza gravitacional que lo
% curva hacia el centro (roja) y su velocidad tangente (verde), con las dos fórmulas
% compuestas matemáticamente. Estrena tres cosas del lenguaje: `\frac` para las
% fracciones, `include` de una biblioteca —el icono reutilizable `lib/satellite.mg`,
% colocado con `scale`/`at`— y flechas cuya punta HEREDA el color de su línea. Va en
% aspecto 16:9 y rotula con texto matemático `$…$`.
%
% NOTAS --------------------------------------------------------------------
% FIGURA ORIGINAL (didáctica): no reproduce una publicación, así que lleva nombre
% de física, como franck_condon/turning_points.
%
% ESCALA DEL ICONO: `Satellite` (lib/satellite.mg) no declara `world_window`, así
% que se coloca 1:1 —sus coordenadas son unidades de mundo— y `scale=0.8` es un
% MULTIPLICADOR de su tamaño natural (~1.8 u de ancho), no un tamaño absoluto; el
% aspecto se preserva (declarar un `world_window` lo volvería absoluto pero lo
% estiraría, porque la normalización struct→caja es anisotrópica).

display_size 16 9

world_window 0 16  0 9

include "../lib/satellite.mg"

text("/bFuerza de atracción gravitacional") { 1 8 }
text("$F = G \frac{m_1 m_2}{r^2}$", size=12) { 1 7 }

text("/bFuerza centrípeta") { 1 5 }
text("$F_c = \frac{m v^2}{r}$", size=12) { 1 4 }

% orbit
circle(4, dash="dashed", line_width=0) { 10 4.5 }

% planet
circle(1, fill="midnightblue", color="black") { 10 4.5 }

% satellite
%fit(Satellite) { 14 4.5  14.5 5 }
Satellite(scale=0.8,at=(14, 4.6))

polyline(marker_end="arrow", marker_size=3, color="red") { 13.24 4.5  11.5 4.5 }
polyline(marker_end="arrow", marker_size=3, color="green") { 14 5.075 14 7 }

text("Órbita", size=8, align="right") { 6 3 }
text("Velocidad", align="center", size=8) { 14 7.25 }
text("Atracción", align="center", size=8) { 12.5 4 }
