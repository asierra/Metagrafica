% Prueba de repeat — traducción V3 de examples/v1/rpstest.mg
%
% ⚠ AÚN NO COMPILABLE (no hay parser V3). Fixture de spec; su render debe igualar
% examples/v1/reference/rpstest.svg. Ejercita §8 (struct + at/scale), §17 (repeat
% con at/advance) y §7 (estado).

struct Cuadro() {
    % polyline, no rectangle: EPS/PDF no rotan rectangle; SVG sí (§4.4/§19)
    polyline { 0 0  1 0  1 1  0 1  0 0 }
}

% 1) Repetición simple: 3 copias avanzando en x. Posición y avance en la propia
%    llamada (at/advance, §17) → repeat autocontenido, sin moveto/step.
%    V1: SCST .3 .3 (tamaño, MTST) + TLPP .3 0 (avance) + XYPP .05 .05 (pos)
repeat(Cuadro, count=3, scale=0.3, at=(0.05, 0.05), advance=(0.3, 0))

align "center"
text("Repeate Structure Test") { 0.5 0.94 }

% 2) 4 copias en diagonal, verdes, con struct deformado (scale anisotrópico)
color "green"
repeat(Cuadro, count=4, scale=(0.2, 0.125), at=(0.05, 0.37), advance=(0.2, 0.135))

% 3) Sin avance: omitir advance → todas las copias en el mismo punto; transform=
%    rotate(60) acumula el giro en cada una. (El antiguo G3 "¿cómo resetear step?"
%    desaparece: cada repeat es autocontenido, no comparte estado de pluma.)
%    V1: XYPP .7 .5 (pos) + IDPP (sin avance) + RTRS 60 (rota acumulando) + RPST 5
color "red"
repeat(Cuadro, count=5, scale=(0.2, 0.125), at=(0.7, 0.5), transform=rotate(60))

% 4) Colocación puntual: NO es un place (no hay locus), es invocación directa con
%    at/scale (§8). V1: identifier placement `cuadro .1 .75`.
Cuadro(scale=(0.2, 0.125), at=(0.1, 0.75))
