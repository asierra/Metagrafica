% Fig. 7.9 de IMQ — Potencial de contacto (dos metales).
% Traducción V3 de examples/v1/fig6-10.mg. Render objetivo:
% examples/v1/reference/fig6-10.svg.
%
% Dos paneles a)/b) vía fit (como fig4-1). Simplificaciones V3:
%  · líderes/niveles punteados (V1 GNPATH+DOT y LPATRN 3) → polyline(dash="dotted");
%  · bloques metálicos: rectangle tramado (ya transformable) + contorno grueso;
%  · flechas de W con place; reglillas de cota con ticks.
% ⚠ Sólo verificación de render (parser V3): posiciones/params de ticks, escalas
%   de flecha, y la etiqueta "energy" rotada (texto bajo transform, §19 abierto).

display_size 11.8 4.7
font_size 8
include "arrow.mg"

% ── Panel a): metales en contacto, sin equilibrar ────────────────────────────
struct Fig1() {
    world_window 0 10 0 7

    % interruptor (contactos + palanca)
    line_width 0.2
    dot(0.75) { 4.25 0.75  5.75 0.75 }
    polyline { 4 0.75 4.25 0.75 5.6875 1.325 ;  5.75 0.75 6 0.75 }

    % cotas (reglillas de dimensión) — ⚠ params por calibrar
    line_width 0.1
    ticks(23, mark=(2, 0), at=(2, 1.95), advance=(0, 0.2))
    ticks(23, mark=(2, 0), at=(6, 1.95), advance=(0, 0.2))
    % columnas de puntos de ocupación electrónica (V1 GNPATH + DOT 3 &dots):
    % un dot físico por nivel, paso 0.2, del fondo (1.95) hasta E_F (centro del pozo)
    for i = 0 to 9 {                    % niveles comunes a ambos pozos
        y = 1.95 + i*0.2
        dot(0.75) { 3 y  7 y }
    }
    for i = 10 to 15 {                  % pozo A sube más → E_{F_A}=4.95 (16 niveles)
        y = 1.95 + i*0.2
        dot(0.75) { 3 y }
    }

    % bloques metálicos: tramado + contorno grueso
    rectangle(hatch=45, hatch_gap=4, color="black") { 2 0.25  4 0.75 }
    rectangle(hatch=45, hatch_gap=4, color="black") { 6 0.25  8 0.75 }
    line_width 0.6
    rectangle { 2 0.25  4 0.75 }
    rectangle { 6 0.25  8 0.75 }

    % perfil de potencial (barreras)
    line_width 0.1
    polyline { 0 7 2 7 2 1.8 ;  4 1.8 4 7 6 7 6 1.8 ;  8 1.8 8 7 10 7 }

    % niveles de Fermi (punteados)
    polyline(dash="dotted") { 0 4.95 1 4.95 ;  1.7 4.95 2 4.95 ;
                              8 3.75 8.19 3.75 ;  9 3.75 10 3.75 }

    % flechas de función de trabajo W (V1 LNST sc<0 = both_sides)
    place(Arrowr, scale=0.05, both_sides=true, gap=0.5) { 0.4 4.95  0.4 7 }   % hueco para W_A
    place(Arrowr, scale=0.05, both_sides=true, gap=0.4) { 9.5 3.75  9.5 7 }   % hueco para W_B

    % etiquetas
    text("metal A") { 2.2 -0.25 }
    text("metal B") { 6.2 -0.25 }
    font "italic"
    text("$E_{F_A}$") { 1 4.8 }
    text("$W_A$")     { 0.05 5.8 }
    text("$W_B$")     { 9.2 5.2 }
    text("$E_{F_B}$") { 8.2 3.6 }
    font "roman"
}

% ── Panel b): metales equilibrados (ΔV = W_B − W_A) ──────────────────────────
struct Fig2() {
    world_window 0 10 0 7

    line_width 0.2
    dot(0.75) { 4.25 0.75  5.75 0.75 }
    polyline { 4 0.75 4.25 0.75 ;  4.25 0.83 5.8 0.83 ;  5.75 0.75 6 0.75 }

    line_width 0.1
    ticks(17, mark=(2, 0), at=(2, 1.95), advance=(0, 0.2))
    ticks(23, mark=(2, 0), at=(6, 1.95), advance=(0, 0.2))
    % columnas de puntos: equilibradas (ambos pozos hasta 3.75, 10 niveles)
    for i = 0 to 9 {
        y = 1.95 + i*0.2
        dot(0.75) { 3 y  7 y }
    }

    polyline(dash="dashed") { 0 7  2 7 }
    rectangle(hatch=45, hatch_gap=4, color="black") { 2 0.25  4 0.75 }
    rectangle(hatch=45, hatch_gap=4, color="black") { 6 0.25  8 0.75 }
    line_width 0.6
    rectangle { 2 0.25  4 0.75 }
    rectangle { 6 0.25  8 0.75 }

    line_width 0.1
    polyline { 0 5.6 2 5.6 2 1.8 ;  4 1.8 4 5.6 6 7 6 1.8 ;  8 1.8 8 7 10 7 }

    polyline(dash="dotted") { 0 3.75 1 3.75 ;  1.7 3.75 2 3.75 ;
                              8 3.75 8.19 3.75 ;  9 3.75 10 3.75 }

    place(Arrowr, scale=0.05, both_sides=true, gap=0.5) { 0.4 3.75  0.4 5.6 }  % hueco para W_A
    place(Arrowr, scale=0.05, both_sides=true, gap=0.4) { 9.5 3.75  9.5 7 }     % hueco para W_B
    place(Arrowr, scale=0.05, both_sides=true, gap=0.7) { 1 5.6  1 7 }          % hueco para Δ V

    text("metal A") { 2.2 -0.25 }
    text("metal B") { 6.2 -0.25 }
    font "italic"
    text("$E_{F_A}$")               { 1 3.6 }
    text("$W_A$")                   { 0.05 4.5 }
    text("$W_B$")                   { 9.2 5.2 }
    text("$E_{F_B}$")               { 8.2 3.6 }
    text("$\Delta V = W_B - W_A$")  { 0 6.12 }
    font "roman"
}

% ── Composición: eje de energía + dos paneles + etiquetas ────────────────────
% La ventana toma el ASPECTO del display (11.8/4.7 ≈ 2.51) para que el meet
% isométrico llene el ancho: x se extiende al doble (23.6 = 9.4·2.51), en vez del
% estirado anisótropo `SCST .45 .8` del V1. Los paneles se colocan a lo ancho.
world_window -0.8 22.8  -1.3 8.1

line_width 0.2
place(Arrowr, scale=0.08) { 0.2 1.4  0.2 7.8 }        % eje vertical de energía

fit(Fig1, stretch=true) { 1    0  11   7 }
fit(Fig2, stretch=true) { 12.4 0  22.4 7 }

text("a)") { 5.6 -0.9 }
text("b)") { 17  -0.9 }

% "energy" rotado 90° (texto bajo transform, §19 — define si giran los glifos)
{ translate -0.1 2   rotate 90   text("energy") { 0 0 } }
