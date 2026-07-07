% Fig 2.1 de IMQ (difracción de electrones) — 4 paneles a) b) c) d).
% Traducción V3 (ESQUELETO DE ESTUDIO) de examples/v1/fig4-1.mg. Render objetivo:
% examples/v1/reference/fig4-1.svg.
%
% Propósito: composición de paneles en V3 (structs con ventana propia + fit
% anidado, §8/§10.2/§16). Curvas (curvas3.mg) y flechas (arrow.mg) ya traducidas
% e incluidas. Regla clave: V1 PWST SIEMPRE deformaba → V3 fit(stretch=true) (§10.2).
%
% ⚠ Pendiente SOLO de verificar el render contra el oráculo (todo bloqueado por
%   el parser V3; este archivo es sintaxis V3). No quedan decisiones de diseño:
%   1) que fit(stretch=true) reproduzca el aspecto del V1 (PWST);
%   2) rotación/posición exactas de Fg1/Fg2 y el rect del haz de Flechas.
% (Las flechas van world-scaled a propósito: reproducen el V1, no usan el
%  marcador físico —eso es para datos nuevos, fig2-3.)

display_size 11 7.7
font_size 8

include "arrow.mg"       % marcadores de flecha: Arrowr…
include "curvas3.mg"     % curvas de onda: SenDeriv2Sym / SenCosDeriv2Sym

% ── Aparato: pantallas con rendijas (fiel al V1) ─────────────────────────────
% V1 MARCO: WW 0 2.2 0 1, LWIDTH 0 → 0.1. Los `;` del PL V1 se conservan en V3
% como subtrayectos disjuntos (§4): un solo polyline, mismo estilo.
struct Marco() {
    world_window 0 2.2 0 1
    line_width 0.1
    polyline { 0.3 0 0.3 0.45 ; 0.3 0.55 0.3 1 ;               % x=.3, rendija .45–.55
               0.9 0 0.9 1 ;                                   % x=.9 (completa)
               1.6 0 1.6 0.3 ; 1.6 0.4 1.6 0.6 ; 1.6 0.7 1.6 1 ; % x=1.6, dos rendijas
               2.2 0 2.2 1 }                                    % detector x=2.2
}

% ── Haz de flechas de electrones ─────────────────────────────────────────────
% V1: MKST arrowr + LNST .02 0 9 : .01 .1 .3 .1  → 9 flechas iguales a lo largo
% de la línea (.01,.1)→(.3,.1), escala .02.
struct Flechas() {
    world_window 0 2.2 0 1
    line_width 0.2
    % place por count=9 (§10.1). El tamaño (scale=0.02) es cantidad de mundo, así
    % que se deforma con el fit del panel — pero eso REPRODUCE el V1 (LNST .02 bajo
    % PWST), que es lo que muestra el oráculo. El marcador FÍSICO (dot/marker) es
    % para datos nuevos (fig2-3), NO para reproducir esta colocación decorativa V1.
    place(Arrowr, scale=0.02, count=9) { 0.01 0.1  0.3 0.1 }
}

% Curva simétrica rotada 90° y colocada, lista para fit en el panel (V1 FG1/FG2).
% ⚠ rotación/posición exactas por verificar contra el oráculo.
struct Fg1() { rotate 90   SenDeriv2Sym(at=(1, 0)) }
struct Fg2() { rotate 90   SenCosDeriv2Sym(at=(1, 0)) }

% ── Paneles superiores a) b): aparato + curvas de difracción ─────────────────
struct FigAB() {
    world_window 0 2.2 0 1
    fit(Marco, stretch=true) { 0 0  2.2 1 }
    line_width 0.4
    fit(Fg1, stretch=true) { 0.35 0  0.9 1 }    % patrón de una rendija
    fit(Fg2, stretch=true) { 1.6  0  2.2 1 }    % patrón de doble rendija
    fit(Flechas, stretch=true) { 0 0  1.3 1 }   % ⚠ rect del haz por confirmar
}

% ── Paneles inferiores c) d): aparato + detectores ───────────────────────────
struct FigCD() {
    world_window 0 2.2 0 1
    fit(Marco, stretch=true) { 0 0  2.2 1 }
    line_width 0.4
    polyline { 0.9 0.45 0.5 0.45 0.5 0.55 0.9 0.55 ;      % detector izq.
               2.2 0.3 1.8 0.3 1.8 0.4 2.2 0.4 ;          % detectores der.
               2.2 0.7 1.8 0.7 1.8 0.6 2.2 0.6 }
    fit(Flechas, stretch=true) { 0 0  1.3 1 }
}

% ── Composición: dos bandas horizontales + etiquetas ─────────────────────────
world_window -0.07 1.03  -0.05 1.05
fit(FigCD, stretch=true) { 0 0    1 0.45 }    % banda inferior → c) d)
fit(FigAB, stretch=true) { 0 0.55 1 1 }       % banda superior → a) b)

text("a)") { 0.22 0.53 }
text("b)") { 0.81 0.55 }
text("c)") { 0.22 -0.03 }
text("d)") { 0.81 -0.03 }
