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

% --- cono(r, axis, pos) y cilindro(r, axis, pos): siluetas de revolución --------
%
% Un cono y un cilindro no son caras planas: lo que se dibuja de ellos es su SILUETA,
% y esa hay que calcularla. La receta salió de `examples/irradiancia.mg` y se extrajo
% aquí después de estar probada en los tres backends, no antes.
%
% CÓMO. La proyección restringida al plano de una base es una AFINIDAD, y la tangencia
% es invariante afín. Así que en vez de resolver «tangente a una elipse» en la página,
% se retro-proyecta al marco de la base —un 2x2— donde el borde vuelve a ser un CÍRCULO:
%   - cono: el ápice cae en (a,b) y los puntos de tangencia están en
%     atan2(b,a) ± acos(1/D), con D = hypot(a,b) medido en radios.
%   - cilindro: los dos círculos son iguales, así que las tangentes comunes tocan en
%     atan2(s) ± 90°, con s el desplazamiento entre centros retro-proyectado. Sin acos.
% Los puntos de tangencia vuelven al ESPACIO y las generatrices se trazan con `xyz()`
% en sus dos extremos, porque son rectas de verdad del cuerpo.
%
% ⚠️ `axis` es el vector del ápice (o de la base) al OTRO extremo: lleva dirección y
%   longitud juntas, así que no hace falta un parámetro de altura.
% ⚠️ Van en ALAMBRE, como el resto de esta biblioteca: se trazan los bordes, no se
%   rellena el cuerpo. Rellenarlo pide el arco lejano de la base, que es otra cuenta.

% --- cono ---
% ⚠️ Si el ápice cae DENTRO de la base proyectada (D <= 1) se está mirando el cono por
%   dentro y no hay silueta que trazar: se dibuja solo la base. Sin esa guarda `acos`
%   abortaría, que es correcto pero no es lo que una figura quiere aquí.
struct cono(r, axis=[0,1,0], pos=[0,0,0], base=true, contorno="black", lw=0.6) {
    axl = sqrt(axis[0]*axis[0] + axis[1]*axis[1] + axis[2]*axis[2])
    dx = axis[0]/axl   dy = axis[1]/axl   dz = axis[2]/axl
    cx = pos[0] + axis[0]   cy = pos[1] + axis[1]   cz = pos[2] + axis[2]

    % marco de la base: e1 ⊥ eje (horizontal si se puede), e2 = eje x e1
    hh = sqrt(dz*dz + dx*dx)
    e1x = 1   e1y = 0   e1z = 0
    if hh > 0.001 { e1x = dz/hh   e1z = -dx/hh }
    e2x = dy*e1z - dz*e1y   e2y = dz*e1x - dx*e1z   e2z = dx*e1y - dy*e1x

    line_width lw
    color contorno
    if base {
        { plane3d(at=[cx, cy, cz], u=[r*e1x, r*e1y, r*e1z], v=[r*e2x, r*e2y, r*e2z])
          circle(1) { 0 0 } }
    }

    pO = xyz(cx, cy, cz)
    pU = xyz(cx + r*e1x, cy + r*e1y, cz + r*e1z)
    pV = xyz(cx + r*e2x, cy + r*e2y, cz + r*e2z)
    pA = xyz(pos[0], pos[1], pos[2])
    ux = pU[0] - pO[0]   uy = pU[1] - pO[1]
    vx = pV[0] - pO[0]   vy = pV[1] - pO[1]
    dd = ux*vy - uy*vx
    qx = pA[0] - pO[0]   qy = pA[1] - pO[1]
    aa = ( qx*vy - qy*vx) / dd
    bb = (-qx*uy + qy*ux) / dd
    DD = sqrt(aa*aa + bb*bb)

    if DD > 1 {
        t0 = atan2(bb, aa)
        dt = acos(1/DD)
        g1 = t0 + dt   g2 = t0 - dt
        polyline { xyz(pos[0], pos[1], pos[2])
                   xyz(cx + r*(cos(g1)*e1x + sin(g1)*e2x),
                       cy + r*(cos(g1)*e1y + sin(g1)*e2y),
                       cz + r*(cos(g1)*e1z + sin(g1)*e2z)) }
        polyline { xyz(pos[0], pos[1], pos[2])
                   xyz(cx + r*(cos(g2)*e1x + sin(g2)*e2x),
                       cy + r*(cos(g2)*e1y + sin(g2)*e2y),
                       cz + r*(cos(g2)*e1z + sin(g2)*e2z)) }
    }
}

% --- cilindro ---
% `pos` es el centro de una base y `axis` lleva a la otra. Se trazan los dos círculos
% enteros (alambre) y las dos tangentes comunes.
struct cilindro(r, axis=[0,1,0], pos=[0,0,0], contorno="black", lw=0.6) {
    axl = sqrt(axis[0]*axis[0] + axis[1]*axis[1] + axis[2]*axis[2])
    dx = axis[0]/axl   dy = axis[1]/axl   dz = axis[2]/axl
    c2x = pos[0] + axis[0]   c2y = pos[1] + axis[1]   c2z = pos[2] + axis[2]

    hh = sqrt(dz*dz + dx*dx)
    e1x = 1   e1y = 0   e1z = 0
    if hh > 0.001 { e1x = dz/hh   e1z = -dx/hh }
    e2x = dy*e1z - dz*e1y   e2y = dz*e1x - dx*e1z   e2z = dx*e1y - dy*e1x

    line_width lw
    color contorno
    { plane3d(at=[pos[0], pos[1], pos[2]], u=[r*e1x, r*e1y, r*e1z], v=[r*e2x, r*e2y, r*e2z])
      circle(1) { 0 0 } }
    { plane3d(at=[c2x, c2y, c2z], u=[r*e1x, r*e1y, r*e1z], v=[r*e2x, r*e2y, r*e2z])
      circle(1) { 0 0 } }

    pO = xyz(pos[0], pos[1], pos[2])
    pU = xyz(pos[0] + r*e1x, pos[1] + r*e1y, pos[2] + r*e1z)
    pV = xyz(pos[0] + r*e2x, pos[1] + r*e2y, pos[2] + r*e2z)
    pB = xyz(c2x, c2y, c2z)
    ux = pU[0] - pO[0]   uy = pU[1] - pO[1]
    vx = pV[0] - pO[0]   vy = pV[1] - pO[1]
    dd = ux*vy - uy*vx
    qx = pB[0] - pO[0]   qy = pB[1] - pO[1]
    aa = ( qx*vy - qy*vx) / dd
    bb = (-qx*uy + qy*ux) / dd

    % Las tangentes comunes de dos círculos IGUALES son perpendiculares a la línea de
    % centros: no hay nada que resolver, solo girar 90°.
    for k = 0 to 1 {
        g = atan2(bb, aa) + rad(90 - 180*k)
        polyline { xyz(pos[0] + r*(cos(g)*e1x + sin(g)*e2x),
                       pos[1] + r*(cos(g)*e1y + sin(g)*e2y),
                       pos[2] + r*(cos(g)*e1z + sin(g)*e2z))
                   xyz(c2x + r*(cos(g)*e1x + sin(g)*e2x),
                       c2y + r*(cos(g)*e1y + sin(g)*e2y),
                       c2z + r*(cos(g)*e1z + sin(g)*e2z)) }
    }
}
