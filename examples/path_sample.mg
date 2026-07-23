% Muestreo de un trayecto: sample, point_at y angle_at.
%
% Esta familia LEE la geometría de una curva en un parámetro t entre 0 y 1
% recorrido por LONGITUD DE ARCO, así que t=0.5 es el medio GEOMÉTRICO y no la
% mitad de los segmentos. El flag `curve` fija cómo se interpreta el trayecto,
% que es neutro: en verde los puntos como vértices (toca la envolvente), en rojo
% como controles bézier (toca la curva).
%
% NOTAS --------------------------------------------------------------------
% point_at devuelve un punto [x,y], que se usa de dos formas: colocando una
% struct con at= (flecha AZUL, t=0.5), o metiéndolo DIRECTO en el bloque {} de
% una primitiva (flecha MORADA, un marker suelto en t=0.25) — el bloque acepta un
% punto, no solo pares de escalares. Las dos se orientan con rotate=angle_at(…).
%
% ⚠ COBERTURA EXCLUSIVA: es el único ejemplo que ejercita sample, point_at y
% angle_at.

display_size 15 7.5
world_window -0.4 8.4 -0.9 7.6

% flecha corta como struct (apunta a +x en su marco local; rotate= la orienta)
struct flecha() {
    polygon(fill="blue") { 0 0  -0.5 0.22  -0.35 0  -0.5 -0.22 }
}

% una curva suave (bézier de control) — el objeto que se muestrea
path c = smooth { 0.5 1  2.5 6  4.5 1.5  6.5 6  7.5 2 }
line_width 1.2   color "black"   bezier(&c)

% sample: 13 puntos por arco, en los dos modos — envolvente vs curva
dot(sample(&c, 13),             size=2.2, color="green")   % default: ENVOLVENTE
dot(sample(&c, 13, curve=true), size=2.2, color="red")     % curve=true: CURVA

% point_at + angle_at, DOS formas de usarlos:
%  1) colocando una STRUCT con at=/rotate= (la flecha azul, en t=0.5)
flecha(at=point_at(&c, 0.5, curve=true), rotate=angle_at(&c, 0.5, curve=true), scale=1.4)
%  2) point_at directo en el BLOQUE de una primitiva (marker morado, en t=0.25)
%     — el bloque {} acepta un punto [x,y]. El marcador se orienta con su propio
%     `marker_orient=` (ángulo en grados), aquí el que da angle_at a la tangente.
marker(size=7, shape="arrow", color="purple", marker_orient=angle_at(&c, 0.25, curve=true)) {
    point_at(&c, 0.25, curve=true)
}

% leyenda mínima
dot(size=2.2, color="red")   { 0.1 7.3 }
text("curve=true — punto sobre la curva",   align="left", valign="middle", size=8) { 0.4 7.3 }
dot(size=2.2, color="green") { 0.1 6.7 }
text("default — punto sobre la envolvente", align="left", valign="middle", size=8) { 0.4 6.7 }
flecha(at=(0.1, 6.1), scale=1.4)
text("point_at + angle_at en t=0.5",        align="left", valign="middle", size=8) { 0.4 6.1 }
