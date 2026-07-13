display_size 8 4.6
font_size 8
include "arrow.mg"

% Simulación pseudo-3D por proyección oblicua: la cizalla (shear) convierte los
% cuadros frontales en paralelogramos que "receden". Porta el SHST del V1 (§11.1).
% El shear envuelve SOLO los planos + el corner (bloque acotado); las líneas de
% enlace punteadas y las cotas van sin cizallar, como en el original.

struct Cuadro() {
    world_window 0 2.5 0 1
    line_width 0.6
    polygon(fill=gray(0.8), color="black") { 0 0  0 1  1 1  1 0  0 0 }
}
struct Corner() {
    world_window 0 2.5 0 1
    line_width 0
    dash "dotted"
    polyline { 1 0.5  1 0  0.72 0 }
}
struct Caja() {
    {
        shear 0.4 0
        Cuadro(at=(0, 0))
        Cuadro(at=(1.45, 0))
        Cuadro(at=(1.78, 0))
        Corner(at=(1.45, 0))
    }
    line_width 0
    dash "dotted"
    polyline { 0 0  1.78 0 }
    polyline { 0 0.69  1.78 0.69 }
    polyline { 0.46 0.505  2.24 0.505 }
    polyline { 0.46 1.2  2.24 1.2 }
    dash "solid"
    polyline { 0 -0.02  0 -0.08 }
    polyline { 1.45 -0.02  1.45 -0.08 }
    polyline { 1.78 -0.02  1.78 -0.08 }
    place(Arrowr, scale=0.03) { 0.87 -0.05  1.45 -0.05 }
    place(Arrowr, scale=0.03) { 0.58 -0.05  0 -0.05 }
    place(Arrowr, scale=0.03) { 1.65 -0.05  1.78 -0.05 }
    place(Arrowr, scale=0.03) { 1.57 -0.05  1.45 -0.05 }
    font "italic"
    align "center"
    text("D=A-R") { 0.725 -0.075 }
    text("R") { 1.615 -0.075 }
    text("L") { 2.1 0.22 }
    text("L") { 2.3 0.73 }
}
world_window 0 8 0 4.6
Caja(scale=3.174, at=(0.3, 0.5))
