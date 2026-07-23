% Modelo de cuerpo negro — la cavidad y su orificio.
%
% La caja y el arco del orificio se unen en UN solo trayecto cerrado con
% `compound`: al cerrar, los extremos se juntan solos, sin una línea que los
% conecte. Relleno tramado con contorno.
%
% NOTAS --------------------------------------------------------------------
% Fig. 1.1 de «Introducción a la mecánica cuántica», L. de la Peña (FCE/UNAM).

display_size 3 3

struct Fg1() {
    % figura radiante (hairline). Las dos flechas de los extremos son marcadores del
    % propio polyline: marker_start/marker_end orientados a la pendiente local.
    % marker_start_shift corre la de arranque adentro, sobre su segmento.
    line_width 0.1
    polyline(marker_start="arrow", marker_start_shift=0.9, marker_end="arrow",
             marker_size=2.5) { -1.11111 -0.046   0.7071 0.7071   0 -1   -0.7071 0.7071
               1 0   -0.7071 -0.7071   0 0.8 }


    % caja con orificio: polilínea + arco en UN path cerrado (compound), tramado.
    line_width 0.4
    compound(hatch=135, hatch_gap=2, color="black") {
        polyline { -1 0.1736  -1 1.015  1.015 1.015  1.015 -1.015  -1 -1.015  -1 -0.1736 }
        arc(1, from=190, to=530) { 0 0 }   % barrido de 340° desde 190°
    }
}

Fg1(at=(0.52, 0.5), scale=0.46)
