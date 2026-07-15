% Fig. 6.4 — Vidas medias alfa nucleares (ley de Geiger-Nuttall):
% log(λ⁻¹) vs. E^{-1/2}, del Po al U. IMQ 3a. ed.
%
% Versión V3 LIMPIA (propuesta), a partir de la traducción literal fig6-4v3.mg.
% Cambios respecto a la literal:
%   1. Los ejes se generan con axis() en vez de una polilínea en L + dos ticks()
%      manuales con marcas/avances calculados a mano.
%   2. Los puntos experimentales son marcadores FÍSICOS dot() (círculos redondos,
%      radio inmune al stretch) en vez de circle() geométrico (que bajo el stretch
%      del fit se deforma en elipse — justo el caso que exigía el prólogo /ellipse).
%   3. La recta de ajuste vive en la misma struct que los datos, fiteada al marco.
%   4. (plan_plot.md Fase 1) El eje y usa axis(scale="log"): from/to en VALORES
%      (1e-15/1e5), marcas y rótulos "10^n" generados solos (antes 5 text() a
%      mano). El eje x usa strip_zero=true para el estilo del libro (".30", no
%      "0.30").
%
% Lo que axis NO cubre aún y queda manual:
%   - Rótulos de isótopos y los títulos matemáticos de los ejes (superíndices).

display_size 9 6.7
font_size 8

% ── Datos: recta de ajuste + puntos, en coords digitalizadas ────────────────
% dot(r) = marcador físico (§4.6): su POSICIÓN sigue el stretch del fit, su RADIO
% (pt) no → puntos redondos en su sitio. fill "black" los rellena (disco).
struct PUNTOS() {
    world_window 15 139 18 132
    line_width 0.8
    polyline { 15 132  139 18 }
    fill "black"
    dot(1.5) { 18 127  36 114  42 109  53 101  44 95  55 92  60 87.5  69 87
               71 78.5  78.5 85  80 71  83.5 71.5  76.5 68  80 68  88 60
               99.5 56.5  106 47  101.5 40  108 38  111 41  102.5 30
               129.5 24.5  131 19  137 18 }
}

% Ventana en unidades de "plot" (0..6 en x, 0..5.5 en y), con margen para rótulos.
world_window -1 7.2 -0.95 6.25

% ── Eje x: E^{-1/2} lineal, .30 … .50 cada .05 ──────────────────────────────
% axis mapea el VALOR (from..to) a lo largo de la línea {p1 p2}; los rótulos y las
% marcas salen solos. (La recta 0..6 en plot ↔ .30..50 en dato.) strip_zero pule
% el cero inicial (".30", no "0.30" — estilo del libro). ticks="in": marcas hacia
% adentro (como el original). extend: la línea rebasa un poco cada extremo.
line_width 0.2
axis(from=0.30, to=0.50, step=0.05, decimals=2, strip_zero=true,
     ticks="in", extend=0.4) { 0 0  6 0 }

% ── Eje y: λ^{-1} logarítmico (5 décadas mayores) ───────────────────────────
% scale="log": from/to en VALORES (no exponentes); step=5 décadas entre marcas
% mayores → caen en 1e-15, 1e-10, 1e-5, 1, 1e5 (rótulos "10^n" generados solos,
% n=0 → "1"). ticks="in": marcas hacia adentro. extend=(1, 0.3): la línea baja
% 1 unidad bajo la 1ª marca hasta el eje x (esquina) y rebasa un poco arriba.
axis(scale="log", from=1e-15, to=1e5, step=5,
     ticks="in", extend=(1, 0.3)) { 0 1  0 5 }

% ── Puntos y recta al rectángulo de datos ───────────────────────────────────
line_width 1
fit(PUNTOS, stretch=true) { 0.6 0.7  5.79 5.4 }

% Títulos de ejes (matemáticos) e isótopos: aún manuales (fuera de Fase 1).
text("${/gl^-1} (/rs)$") { -0.83 5.5 }
text("$/iE^{/r-1/2} (MeV)^{/r-1/2} $") { 2.4 -0.9 }
text("$Po{^292}$") { 1.1 5.1 }
text("$ U{^238}$") { 5.9 0.6 }
