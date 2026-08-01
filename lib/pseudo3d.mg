% =====================================================================
%  pseudo3d.mg — Piezas sólidas para simulación pseudo-3D en MetaGráfica V3.
%  Ver plan_pseudo3d.md.
%
%  ⚠️ REESCRITA EL 2026-07-31 SOBRE `plane3d`, y en el camino se RETIRÓ `plano`.
%
%  La versión anterior horneaba la proyección dentro de cada forma: `plano(w,h,k)`
%  con su propia cizalla y `prisma(w,h,d,a,f)` con su propio ángulo de escorzo.
%  Funcionaba, y su defecto se puede MEDIR: en `fig2-7b-v3` la pantalla (k=0.3)
%  recedía a 73.3° y el cristal (a=35) a 35.0°. Treinta y ocho grados de diferencia
%  entre dos piezas del mismo dibujo, que coincidían solo porque se habían colocado
%  a mano hasta verse bien. Eso es lo que arregla tener una cámara: las piezas dejan
%  de tener cada una su propia idea de la profundidad.
%
%  `plano` SE FUE porque ya existe y se llama `plane3d`:
%      { plane3d(at=[x,y,z], u=[…], v=[…])
%        rectangle(fill=gray(0.8), color="black") { 0 0  1 1 } }
%  Es más corto que invocarlo, no hereda una cizalla ajena, y encima un `circle`
%  dibujado ahí dentro sale como la elipse EXACTA de su proyección. Una struct que
%  solo envuelve una primitiva y le quita generalidad no se gana el sitio.
%
%  ⚠️ Sin z-buffer: el orden de PINTADO es el orden de escritura. Dibuja de atrás
%    hacia adelante (primero lo lejano).
%
%  ⚠️ NINGUNA COMPUERTA MIRA ESTE ARCHIVO: `test/run.sh` compila `examples/`, y hoy
%    ningún ejemplo del corpus incluye esta biblioteca (su único cliente,
%    `fig2-7b-v3`, vive en `local/`). Si algo de aquí se rompe, se nota al usarlo.
% =====================================================================

% --- prisma(w, h, d): caja de w×h×d puesta en la ESCENA ----------------
% Tres caras visibles (techo, costado derecho y frente), cada una un plano del
% espacio, sombreadas de claro a oscuro y pintadas de atrás hacia adelante.
%
% `pos` es la esquina inferior-izquierda-trasera, en coordenadas de la ESCENA, y la
% caja crece hacia +x (ancho), +y (alto) y +z (hacia el observador). Se llama `pos`
% y no `at` porque `at=` es palabra de COLOCACIÓN de una struct (§8): sigue
% disponible, y sirve para correr la pieza en la página DESPUÉS de proyectarla.
%
% ⚠️ Qué cara queda detrás depende de la cámara. Este orden —techo, costado, frente—
%   es el correcto para la vista de siempre (observador arriba y a la derecha, que es
%   lo que dan `view3d` con elevación positiva o la oblicua con `angle` entre 0 y 90).
%   Con la cámara del otro lado hay que reordenar: es MODELADO, no algo que la
%   biblioteca pueda adivinar sin un z-buffer.
struct prisma(w, h, d, pos=[0,0,0],
              frente=gray(0.85), techo=gray(0.7), lado=gray(0.55),
              contorno="black", lw=0.6) {
    px = pos[0]   py = pos[1]   pz = pos[2]
    line_width lw
    % techo (y = py+h), extendido hacia atrás en profundidad
    { plane3d(at=[px, py+h, pz], u=[w, 0, 0], v=[0, 0, d])
      polygon(fill=techo, color=contorno) { 0 0  1 0  1 1  0 1 } }
    % costado derecho (x = px+w)
    { plane3d(at=[px+w, py, pz], u=[0, 0, d], v=[0, h, 0])
      polygon(fill=lado, color=contorno) { 0 0  1 0  1 1  0 1 } }
    % frente (z = pz+d): la cara más cercana, al final
    { plane3d(at=[px, py, pz+d], u=[w, 0, 0], v=[0, h, 0])
      polygon(fill=frente, color=contorno) { 0 0  1 0  1 1  0 1 } }
}

% --- lamina(h, d): placa delgada perpendicular al eje x ----------------
% Una sola cara: la que se ve de una lámina puesta de canto al haz, que crece en
% alto (+y) y en profundidad (+z). Va aparte de `prisma` porque una lámina no tiene
% grosor que dibujar, y porque su uso normal es con TRAMA en vez de relleno liso.
struct lamina(h, d, pos=[0,0,0], angulo=45, paso=3, contorno="black", lw=0.6) {
    px = pos[0]   py = pos[1]   pz = pos[2]
    line_width lw
    { plane3d(at=[px, py, pz], u=[0, 0, d], v=[0, h, 0])
      polygon(hatch=angulo, hatch_gap=paso, color=contorno) { 0 0  1 0  1 1  0 1 } }
}
