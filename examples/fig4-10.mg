% Fig. 3.3 de IMQ — Niveles del pozo infinito.
% Traducción V3 de examples/v1/fig4-10.mg. Render objetivo:
% examples/v1/reference/fig4-10.svg.
%
% Rediseño limpio (§22.6): los 3 paneles comparten los 4 niveles Eₙ ∝ n² (`for`,
% física a la vista). φₙ=sin(nπx) y ρₙ=|φₙ|² usan la primitiva `sine` (§4.13).
%
% Las ETIQUETAS viven en las coordenadas de MUNDO de su panel (0..10 × 0..19): su
% y ES el nivel n², y se mueven/reescalan con el `fit` del panel — sin calibración
% aparte en la ventana externa. Van a la derecha del pozo (x≈10.5), por eso los
% paneles se separan (hueco de 4) para que no encimen con la columna vecina.
%
% ⚠ Cuerpos escritos UNA SENTENCIA POR LÍNEA (esquiva el bug del parser: una
%   sentencia de estado + otra sentencia en el mismo renglón se traga la 2ª).

display_size 12 6
font_size 8

% ── Piezas compartidas por los 3 paneles (marco 0..10 × 0..19) ───────────────
struct Pozo()    { polyline { 0 19  0 0  10 0  10 19 } }             % paredes del pozo (U)
struct Niveles() { for n = 1 to 4 { polyline { 0 n^2  10 n^2 } } }   % Eₙ ∝ n²

% ── Panel a) energías: niveles sólidos dentro del pozo ───────────────────────
struct PanelE() {
    world_window 0 10 0 19
    line_width 1.0
    Pozo()
    line_width 0.2
    Niveles()
    %font "italic"
    align "right"
    text("$E_1$")   { -0.2 1 }
    text("$E_2$")  { -0.2 4 }
    text("$E_3$")  { -0.2 9 }
    text("$E_4$") { -0.2 16 }
    align "left"
    text("$E_1$")   { 10.5 1 }
    text("4$E_1$")  { 10.5 4 }
    text("9$E_1$")  { 10.5 9 }
    text("16$E_1$") { 10.5 16 }
}

% ── Panel b) funciones de onda φₙ = sin(nπx): n jorobas sobre cada nivel ──────
struct PanelPhi() {
    world_window 0 10 0 19
    line_width 1.0
    Pozo()
    {
        line_width 0.1
        dash "dashed"
        Niveles()
    }
    line_width 0.4
    for n = 1 to 4 { sine(half_cycles=n, amplitude=1.2) { 0 n^2  10 n^2 } }
    text("$\varphi_1$") { 10.5 1 }
    text("$\varphi_2$") { 10.5 4 }
    text("$\varphi_3$") { 10.5 9 }
    text("$\varphi_4$") { 10.5 16 }
}

% ── Panel c) densidades ρₙ = |φₙ|² = sin²(nπx): n jorobas positivas ───────────
struct PanelRho() {
    world_window 0 10 0 19
    line_width 1.0
    Pozo()
    {
        line_width 0.1
        dash "dashed"
        Niveles()
    }
    line_width 0.4
    for n = 1 to 4 { sine(half_cycles=n, amplitude=1.2, squared=true) { 0 n^2  10 n^2 } }
    font "italic"
    text("$\rho_1$") { 10.5 1 }
    text("$\rho_2$") { 10.5 4 }
    text("$\rho_3$") { 10.5 9 }
    text("$\rho_4$") { 10.5 16 }
}

% ── Composición: 3 columnas separadas (hueco de 4 para las etiquetas) ─────────
world_window 0 38 -3 20
fit(PanelE,   stretch=true) { 0  0  10 19 }
fit(PanelPhi, stretch=true) { 14 0  24 19 }
fit(PanelRho, stretch=true) { 28 0  38 19 }

% ── Leyendas de panel: fuera de las structs (dependen del acomodo externo), en
%    coordenadas de la ventana global. x = centro de cada rect de fit ──────────
font "roman"
text("a)") { 4.5  -1.5 }
text("b)") { 18.5 -1.5 }
text("c)") { 32.5 -1.5 }
