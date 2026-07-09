% Fig. 3.3 de IMQ — Niveles del pozo infinito.
% Traducción V3 de examples/v1/fig4-10.mg. Render objetivo:
% examples/v1/reference/fig4-10.svg.
%
% Rediseño limpio (§22.6): los 3 paneles comparten los 4 niveles Eₙ ∝ n², que
% ahora son un `for` con la física a la vista (antes: `0 1;0 4;0 9;0 16` literal
% ×3). Las funciones de onda φₙ=sin(nπx) y densidades ρₙ=|φₙ|² usan la primitiva
% `sine` (§4.13) —una línea cada una— en vez del baile V1 de IDPT/TLPT/SCPT/RPPT/BZ
% + bzsinepaths. NO se necesita `include`: `sine` genera las bezier por dentro.
%
% ⚠ Sólo verificación de render (bloqueado por el parser V3): amplitudes de `sine`,
%   rects de columna y posiciones de etiqueta se calibran contra el oráculo. El
%   layout de 3 columnas es el diseño limpio; el V1 escalaba a mano (SCST) —si se
%   quiere el encuadre exacto del V1 se ajustan los rects de `fit`.

display_size 12 6
font_size 8

% ── Piezas compartidas por los 3 paneles (marco 0..10 × 0..19) ───────────────
struct Pozo()    { polyline { 0 19  0 0  10 0  10 19 } }      % paredes del pozo (U)
struct Niveles() { for n = 1 to 4 { polyline { 0 n^2  10 n^2 } } }  % Eₙ ∝ n²

% ── Panel a) energías: niveles sólidos dentro del pozo ───────────────────────
struct PanelE() {
    world_window 0 10 0 19
    line_width 1.0   Pozo()
    line_width 0.2   Niveles()
}

% ── Panel b) funciones de onda φₙ = sin(nπx): n jorobas sobre cada nivel ──────
struct PanelPhi() {
    world_window 0 10 0 19
    line_width 1.0   Pozo()
    { line_width 0.1   dash "dashed"   Niveles() }            % niveles como base punteada
    line_width 0.4
    for n = 1 to 4 { sine(half_cycles=n, amplitude=1.2) { 0 n^2  10 n^2 } }
}

% ── Panel c) densidades ρₙ = |φₙ|² = sin²(nπx): n jorobas positivas ───────────
struct PanelRho() {
    world_window 0 10 0 19
    line_width 1.0   Pozo()
    { line_width 0.1   dash "dashed"   Niveles() }
    line_width 0.4
    for n = 1 to 4 { sine(half_cycles=n, amplitude=1.2, squared=true) { 0 n^2  10 n^2 } }
}

% ── Composición: 3 columnas lado a lado (⚠ rects por calibrar) ───────────────
world_window 0 32 -3 19
fit(PanelE,   stretch=true) { 0  0  10 19 }
fit(PanelPhi, stretch=true) { 11 0  21 19 }
fit(PanelRho, stretch=true) { 22 0  32 19 }

% ── Etiquetas (explícitas; posiciones por calibrar) ──────────────────────────
% Energías Eₙ = n²·E₁ y funciones/densidades, en cada nivel n² de cada columna.
font "italic"
text("$E_1$")    { 3 1 }    text("$\varphi_1$") { 14 1 }    text("$\rho_1$") { 25 1 }
text("4$E_1$")   { 3 4 }    text("$\varphi_2$") { 14 4 }    text("$\rho_2$") { 25 4 }
text("9$E_1$")   { 3 9 }    text("$\varphi_3$") { 14 9 }    text("$\rho_3$") { 25 9 }
text("16$E_1$")  { 3 16 }   text("$\varphi_4$") { 14 16 }   text("$\rho_4$") { 25 16 }

font "roman"
text("a)") { 5 -2 }
text("b)") { 16 -2 }
text("c)") { 27 -2 }
