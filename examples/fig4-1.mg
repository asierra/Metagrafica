% Fig 2.1 de IMQ (difracción de electrones) — 4 paneles a) b) c) d).
% Render objetivo: examples/v1/reference/fig4-1.svg.
%
% Reescritura V3 declarativa (sin el truco V1 `rotate 90`): las curvas de
% difracción viven en curvas3.mg como DATOS naturales (x=posición t, y=intensidad,
% §9); aquí se TRANSPONEN (`transpose`, intensidad horizontal) y se colocan con
% `fit` directo en su rectángulo, sin rotar ni `at`. La base común (pantallas +
% haz) se factoriza en `Aparato`. Paneles en el orden del oráculo: aparato+
% detectores ARRIBA (a,b), aparato+curvas ABAJO (c,d).

display_size 11 7.7
font_size 8

include "curvas3.mg"     % curvas de onda TRANSPUESTAS: SenDeriv2Sym / SenCosDeriv2Sym

% ── Aparato: pantallas con rendijas (fiel al V1) ─────────────────────────────
struct Marco() {
    world_window 0 2.2 0 1
    line_width 0.1
    polyline { 0.3 0 0.3 0.45 ; 0.3 0.55 0.3 1 ;               % x=.3, rendija .45–.55
               0.9 0 0.9 1 ;                                   % x=.9 (completa)
               1.6 0 1.6 0.3 ; 1.6 0.4 1.6 0.6 ; 1.6 0.7 1.6 1 ; % x=1.6, dos rendijas
               2.2 0 2.2 1 }                                    % detector x=2.2
}

% ── Haz de flechas de electrones ─────────────────────────────────────────────
% Dos haces VERTICALES de 9 flechas (el arpón built-in `arrow` se orienta a la
% tangente → apunta a la derecha), a la izquierda de cada pantalla. repeat =
% traslación pura hacia arriba (advance), sin rotar. Antes venían de arrow.mg
% (struct Arrowr) con include; hoy es el marcador built-in (tamaño físico en pt).
struct Flechas() {
    world_window 0 2.2 0 1
    line_width 0.2
    for i = 0 to 8 {
        y = .1 + i*0.1
        polyline(marker_end="arrow", marker_size=4.5) { .01 y .28 y }
        polyline(marker_end="arrow", marker_size=4.5) { 1.3 y 1.58 y }
    }
}

% Base común de ambos paneles: pantallas + haz de electrones.
struct Aparato() {
    world_window 0 2.2 0 1
    fit(Marco,   stretch=true) { 0 0  2.2 1 }
    fit(Flechas, stretch=true) { 0 0  2.2 1 }
}

% ── Paneles a) b): aparato + patrones de difracción ──────────────────────────
% `transpose` pone la intensidad horizontal (dato natural = posición horizontal);
% fit directo, sin rotar. Una rendija tras x=0.3–0.9; doble rendija tras x=1.6–2.2.
struct PanelCurvas() {
    world_window 0 2.2 0 1
    fit(Aparato, stretch=true) { 0 0  2.2 1 }
    line_width 0.4
    fit(flip_x(transpose(&SenDeriv2Sym)),    stretch=true) { 0.35 0  0.9 1 }   % patrón de una rendija
    fit(flip_x(transpose(&SenCosDeriv2Sym)), stretch=true) { 1.6  0  2.2 1 }   % patrón de doble rendija
}

% ── Paneles c) d): aparato + detectores ──────────────────────────────────────
struct PanelDetectores() {
    world_window 0 2.2 0 1
    fit(Aparato, stretch=true) { 0 0  2.2 1 }
    line_width 0.4
    polyline { 0.9 0.45 0.5 0.45 0.5 0.55 0.9 0.55 ;      % detector izq.
               2.2 0.3 1.8 0.3 1.8 0.4 2.2 0.4 ;          % detectores der.
               2.2 0.7 1.8 0.7 1.8 0.6 2.2 0.6 }
}

% ── Composición: dos bandas horizontales (curvas abajo, detectores arriba) ────
world_window -0.07 1.03  -0.05 1.05
fit(PanelCurvas,     stretch=true) { 0 0    1 0.45 }   % banda inferior → c) d)
fit(PanelDetectores, stretch=true) { 0 0.55 1 1 }      % banda superior → a) b)

text("a)") { 0.22 0.53 }
text("b)") { 0.81 0.55 }
text("c)") { 0.22 -0.03 }
text("d)") { 0.81 -0.03 }
