% Fig. 1.1 de IMQ — Modelo de cuerpo negro.
% Traducción V3 de examples/v1/fig2-1.mg. Render objetivo:
% examples/v1/reference/fig2-1.svg.
%
% Estrena `compound` (§9.4): la polilínea de la caja + el arco del orificio se
% unen en UN path cerrado (V1 OPPT…CLPT) — al cerrar, los extremos se juntan
% solos, sin línea explícita que los conecte. Relleno tramado con contorno.

display_size 3 3
include "arrow.mg"                 % marcadores de flecha: Arrowr

struct Fg1() {
    % figura radiante (hairline)
    line_width 0.1
    polyline { -1.11111 -0.046   0.7071 0.7071   0 -1   -0.7071 0.7071
               1 0   -0.7071 -0.7071   0 0.8 }

    % flechas orientadas con la pendiente de la línea (§10.1); el marcador cae en
    % el 2º punto. Antes había que calcular el ángulo y rotar a mano (SCST/RTST).
    place(Arrowr, scale=0.04) { -1.11111 -0.046   -0.93 0.03 }
    place(Arrowr, scale=0.04) { -0.7071 -0.7071    0 0.8 }

    % caja con orificio: polilínea + arco en UN path cerrado (compound), tramado.
    % V1: FPATRN -8 → hatch 135°, paso 2, con contorno (negativo = contornear).
    line_width 0.4
    compound(hatch=135, hatch_gap=2, color="black") {
        polyline { -1 0.1736  -1 1.015  1.015 1.015  1.015 -1.015  -1 -1.015  -1 -0.1736 }
        arc(1, from=190, to=530) { 0 0 }   % V1 CR 1 340 190: barrido 340° desde 190°
    }
}

Fg1(at=(0.52, 0.5), scale=0.46)
