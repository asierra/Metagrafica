% Órbita circular — gravitación y fuerza centrípeta
%
% Un satélite en órbita circular alrededor de un planeta: la atracción
% gravitacional que lo curva hacia el centro y su velocidad tangente, con las
% dos fórmulas. Primera figura del corpus que COMPONE con una biblioteca —
% `include` del icono reutilizable `lib/satellite.mg`, colocado con `scale`/`at`—;
% dibuja las dos flechas como subtrayectos disjuntos de UNA polilínea (`;`, misma
% punta), va en aspecto 16:9 y rotula con texto matemático `$…$`.
%
% NOTAS --------------------------------------------------------------------
% FIGURA ORIGINAL (didáctica): no reproduce una publicación, así que lleva nombre
% de física, como franck_condon/turning_points.
%
% ⚠️ FUERA DEL GOLDEN A PROPÓSITO: todavía NO está en la lista de `test/run.sh`.
% Sus fórmulas FINGEN la fracción con `/n` (salto de línea) porque `\frac` aún no
% existe; cuando entre —ver `plan_frac.md`, junto con el espaciado math tipo TeX—
% se reescriben `F = \frac{G m_1 m_2}{r^2}` y `F_c = \frac{m v^2}{r}`, y ENTONCES
% la figura entra a la red golden. Es la figura que MOTIVA `\frac` y estrena `include`.
%
% ESCALA DEL ICONO: `Satellite` (lib/satellite.mg) no declara `world_window`, así
% que se coloca 1:1 —sus coordenadas son unidades de mundo— y `scale=0.8` es un
% MULTIPLICADOR de su tamaño natural (~1.8 u de ancho), no un tamaño absoluto; el
% aspecto se preserva (declarar un `world_window` lo volvería absoluto pero lo
% estiraría, porque la normalización struct→caja es anisotrópica).

display_size 16 9

world_window 0 16  0 9

include "../lib/satellite.mg"

% orbit
circle(4, dash="dashed", line_width=0) { 9 4.5 }

% planet
circle(1, fill="midnightblue", color="black") { 9 4.5 }

% satellite
%rectangle { 12.5 4  13.5 5 }
%fit(Satellite) { 13 4.5  13.5 5 }
Satellite(scale=0.8,at=(13, 4.6))

polyline(marker_end="arrow", marker_size=3) { 12.24 4.5  10.5 4.5 ; 13 5.075 13 7 }

text("$F = G m_1 m_2 /n r^2$", size=12, align="center") { 3 7 }
text("$F_c = m v^2 /n r$", size=12, align="center") { 3 5 }

text("Órbita", size=8, align="right") { 5 3 }
text("Velocidad", align="center", size=8) { 13 7.25 }
text("Fuerza/ngravitacional/n de atracción", align="center", size=8) { 11.2 4 }
