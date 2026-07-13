% Fig. 2.7 — panel b (montaje pseudo-3D): difracción de electrones por una
% lámina policristalina.  Haz incidente → láminas → cristal → pantalla con el
% anillo de difracción.  Construido sobre lib/pseudo3d.mg: láminas = polígonos
% oblicuos (casi verticales) con trama; cristal = prisma delgado; pantalla =
% plano; anillo = ellipse (la elipse la da el motor).  El panel a (Bragg 2D)
% está en fig2-7v3.mg.  Ver plan_pseudo3d.md.

display_size 12 7
font_size 9
include "arrow.mg"
include "../../lib/pseudo3d.mg"

world_window 0 12 0 7

% --- eje óptico no desviado (línea de trazos, al fondo) ----------------
polyline(dash="dashed") { 1.0 3.5  10.1 3.5 }

% --- pantalla (plano oblicuo, al fondo a la derecha) -------------------
plano(w=1.4, h=4.2, k=0.3, relleno="white", at=(8.7, 1.3))

% --- anillo de difracción sobre la pantalla (elipse) ------------------
line_width 0.5
ellipse(0.6, 1.3) { 10.06 3.5 }
polyline { 10.06 3.5  10.06 4.8 }      % radio O→P (marca R)
polyline { 10.06 3.5  10.6 3.72 }      % radio al borde derecho

% --- láminas policristalinas (paralelogramos CASI verticales, trama) --
polygon(hatch=45, hatch_gap=3, color="black") {
    3.60 1.30   3.74 1.30   3.93 6.10   3.79 6.10   3.60 1.30
}
polygon(hatch=45, hatch_gap=3, color="black") {
    5.00 1.30   5.14 1.30   5.33 6.10   5.19 6.10   5.00 1.30
}

% --- cristal (laja delgada inclinada = prisma pequeño) ----------------
prisma(w=0.5, h=0.65, d=0.4, a=35, f=0.6, rotate=12, at=(4.0, 3.0))

% --- haz incidente (sube desde abajo-izq al eje, ángulo θ) ------------
line_width 1
polyline(marker_end="Arrowr", marker_size=3) { 0.7 2.9  3.6 3.5 }

% --- rayo difractado hasta P sobre la pantalla (ángulo 2θ) ------------
polyline { 4.5 3.5  10.06 4.8 }

% --- marcas de ángulo --------------------------------------------------
line_width 0.5
arc(1.1, from=0, to=12) { 1.0 2.95 }     % θ (haz vs eje)
arc(1.5, from=0, to=15) { 5.8 3.5 }      % 2θ (rayo vs eje)

% --- etiquetas ---------------------------------------------------------
align "center"
text("polycrystaline sheet") { 4.35 6.45 }
text("screen") { 9.9 0.7 }
font "italic"
text("$\theta$")  { 1.9 3.15 }
text("$2\theta$") { 6.9 3.85 }
text("L") { 7.3 3.2 }
text("O") { 9.65 3.3 }
text("R") { 10.2 4.15 }
text("P") { 10.2 4.95 }
