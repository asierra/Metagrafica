% ─────────────────────────────────────────────────────────────────────────────
% Muestreo de un path por longitud de arco: sample / point_at / angle_at (§9).
%
% La familia LEE geometría de una curva en un parámetro t∈[0,1] recorrido por
% LONGITUD DE ARCO — así t=0.5 es el medio GEOMÉTRICO, no la mitad de los
% segmentos. El flag `curve` fija la interpretación del path (que es neutro):
%   curve=false (default) — puntos como VÉRTICES → toca la ENVOLVENTE (verde)
%   curve=true            — controles bézier, evalúa la cúbica → toca la CURVA (rojo)
%
% point_at devuelve un punto [x,y]: se usa para COLOCAR una struct con at= (no en
% el bloque {} de una primitiva). Aquí una flecha-struct se orienta a la tangente
% en t=0.5 con at=point_at(...) + rotate=angle_at(...).
%
% Único ejemplo del corpus que ejercita sample/point_at/angle_at.
% ─────────────────────────────────────────────────────────────────────────────
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

% point_at + angle_at: la flecha en el medio geométrico, orientada a la tangente
flecha(at=point_at(&c, 0.5, curve=true), rotate=angle_at(&c, 0.5, curve=true), scale=1.4)

% leyenda mínima
dot(size=2.2, color="red")   { 0.1 7.3 }
text("curve=true — punto sobre la curva",   align="left", valign="middle", size=8) { 0.4 7.3 }
dot(size=2.2, color="green") { 0.1 6.7 }
text("default — punto sobre la envolvente", align="left", valign="middle", size=8) { 0.4 6.7 }
flecha(at=(0.1, 6.1), scale=1.4)
text("point_at + angle_at en t=0.5",        align="left", valign="middle", size=8) { 0.4 6.1 }
