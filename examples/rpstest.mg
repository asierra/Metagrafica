% Repetición de estructuras — las formas de colocar una struct muchas veces.
%
% Un cuadro repetido con `repeat`: avanzando en x, en diagonal y deformado, y
% girando sobre el mismo punto, donde `transform=` acumula el giro en cada copia.
% La última no es un repeat, sino colocación puntual con at= y scale=.
%
% NOTAS --------------------------------------------------------------------
% Cada repeat es autocontenido —posición y avance van en la propia llamada— y no
% comparte estado de pluma con los demás, así que no hay nada que reiniciar entre
% uno y otro.

struct Cuadro() {
    % rectangle ya es transformable (rota/deforma como SVG) → uso directo
    rectangle { 0 0  1 1 }
}

% 1) Repetición simple: 3 copias avanzando en x. Posición y avance en la propia
%    llamada (at/advance) → repeat autocontenido, sin moveto/step.
repeat(Cuadro, count=3, scale=0.3, at=(0.05, 0.05), advance=(0.3, 0))

align "center"
text("Repeate Structure Test") { 0.5 0.94 }

% 2) 4 copias en diagonal, verdes, con struct deformado (scale anisotrópico)
color "green"
repeat(Cuadro, count=4, scale=(0.2, 0.125), at=(0.05, 0.37), advance=(0.2, 0.135))

% 3) Sin avance: omitir advance → todas las copias en el mismo punto; transform=
%    rotate(60) acumula el giro en cada una.
color "red"
repeat(Cuadro, count=5, scale=(0.2, 0.125), at=(0.7, 0.5), transform=rotate(60))

% 4) Colocación puntual: NO es un place (no hay locus), es invocación directa con
%    at/scale.
Cuadro(scale=(0.2, 0.125), at=(0.1, 0.75))
