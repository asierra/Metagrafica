% El ejemplo del README (sección "Quick start"). Si lo cambias, regenera su SVG:
%
%     bin/mg examples/quickstart.mg docs/img/quickstart.svg
%
% Está en el corpus golden (test/run.sh) a propósito: el ejemplo que la portada del
% proyecto le enseña al mundo no puede romperse en silencio — es exactamente lo que
% le pasó al ejemplo del man, que quedó en sintaxis V1 y hoy ni compila.

display_size 9 5.5
font_size 9
world_window -2 11 -1.5 5.5

% plot mapea unidades de DATOS a una caja física en cm; los ejes heredan los rangos
% x=/y= y se rotulan solos.
plot(x=(0,10), y=(0,100), box=(0,0, 9,4.5), grid=true) {
    line_width 0.8
    polyline { 0 0  1 1  2 4  3 9  4 16  5 25  6 36  7 49  8 64  9 81  10 100 }
    marker(size=4, shape="cross") {
        0.9 10.0
        2.5 15.0
        4.2 30.0
        6.75 60.2
    }  
    legend(at="top-left", margin=10, sample_width=20, gap=5, font_size=8) {
        entry("Theoretical") { polyline { 0 0.5  1 0.5 } }
        entry("Experimental") { marker(3, shape="cross", color="black") { 0.5 0.5 } }
    }
    xaxis(step=2, label="x")
    yaxis(step=25, label="$y = x^2$")
}
