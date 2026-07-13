% =====================================================================
%  pseudo3d.mg — Biblioteca de simulación pseudo-3D (proyección oblicua)
%  para MetaGráfica V3.  Ver plan_pseudo3d.md.
%
%  Todo se construye COMPONIENDO primitivas 2D con vértices calculados de
%  los parámetros — cero motor nuevo.  La proyección oblicua/caballera se
%  hornea en la geometría (paralelogramos), no en la pila de transformadas,
%  para que cada pieza sea colocable con at=/scale=/rotate= como cualquier
%  struct y componga de forma predecible.
%
%  Convención: el "escorzo" en profundidad va hacia arriba-derecha con un
%  ángulo `a` (grados) y un factor `f` (1 = caballera, 0.5 = gabinete).
%
%  ⚠ Sin z-buffer: el orden de PINTADO es el orden de escritura.  Dibuja de
%    atrás hacia adelante (primero lo lejano).  Ver plan_pseudo3d.md.
% =====================================================================

% --- plano(w, h): cara rectangular w×h vista oblicuamente --------------
% El borde superior se desplaza a la derecha k·h respecto del inferior
% (cizalla horizontal).  k = tan del ángulo de escorzo × factor.  Un
% círculo dibujado sobre un plano se vuelve elipse por su propia cizalla.
struct plano(w, h, k=0.4, relleno=gray(0.8), contorno="black", lw=0.6) {
    line_width lw
    polygon(fill=relleno, color=contorno) {
        0          0
        w          0
        (w + k*h)  h
        (k*h)      h
        0          0
    }
}

% --- prisma(w, h, d): caja w×h×d en proyección oblicua -----------------
% Tres caras visibles (frente + techo + costado derecho) sombreadas de
% claro a oscuro.  Profundidad proyectada a `a` grados con factor `f`.
% Se dibuja de atrás (techo/costado) hacia el frente.
struct prisma(w, h, d, a=35, f=0.6,
              frente=gray(0.85), techo=gray(0.7), lado=gray(0.55),
              contorno="black", lw=0.6) {
    dx = d * f * cos(a * pi / 180)
    dy = d * f * sin(a * pi / 180)
    line_width lw
    % techo (borde superior del frente hacia atrás)
    % (ojo: un identificador desnudo seguido de '(' se lee como llamada a
    %  función; por eso los vértices con var+paréntesis van parentizados)
    polygon(fill=techo, color=contorno) {
        0        h
        w        h
        (w+dx)   (h+dy)
        (dx)     (h+dy)
        0        h
    }
    % costado derecho (borde derecho del frente hacia atrás)
    polygon(fill=lado, color=contorno) {
        w        0
        (w+dx)   dy
        (w+dx)   (h+dy)
        w        h
        w        0
    }
    % frente (cara real, sin escorzo) — al final, es lo más cercano
    polygon(fill=frente, color=contorno) {
        0  0   w  0   w  h   0  h   0  0
    }
}
