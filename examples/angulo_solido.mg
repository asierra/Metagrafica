% Proyección del ángulo sólido — esfera reticulada y casquete A = pi r^2
%
% Una esfera de alambre en axonometría ortográfica, con el casquete circular que
% subtiende un ángulo sólido desde su centro. Todo el dibujo son CÍRCULOS DEL
% ESPACIO —cada meridiano, cada paralelo y el borde del casquete—, y aquí no se
% calcula ni un semieje: `view3d` fija la cámara, `plane3d` pone las coordenadas
% en un plano de la escena, y un `circle` dibujado ahí sale como la elipse exacta
% de su proyección. Lo que no pertenece a ningún plano —las tres cotas punteadas—
% va con `xyz()`. Cambiar `azd` o `eld` mueve la figura entera junta.
%
% NOTAS ————————————————————————————————————————————————————————————————
%
% PROCEDENCIA: Lira, fig. II-4 («proyección del ángulo sólido»). RECONSTRUIDA, no
% digitalizada: el espaciado de la retícula, la cámara y el tamaño del casquete se
% eligieron para que la página se lea como la publicada, no midiendo el escaneo. Lo
% exacto es la geometría: el casquete es un disco PERPENDICULAR a rho, las dos cotas
% desde o van a puntos opuestos de su borde, y r va del centro del casquete al borde.
% Va dibujado grande —r ~ rho— como en el original, que es esquemático: a escala real
% el ángulo sólido sería diminuto.
%
% OCULTAMIENTO DE LA MITAD TRASERA, en forma cerrada. Un punto está del lado visible
% si P.w > 0, con w la dirección hacia el observador,
%     w = (-sin(th)*cos(ph),  sin(ph),  cos(th)*cos(ph))
% y sobre un círculo P(t) = C + u*cos t + v*sin t eso se vuelve
%     A + B*cos t + D*sin t > 0        con A = C.w, B = u.w, D = v.w
% o sea  cos(t - psi) > -A/M,  con  M = sqrt(B²+D²)  y  psi = atan2(D, B).
% El arco visible es t en (psi - tc, psi + tc) con tc = acos(-A/M). Se dibuja con un
% `arc(from=, to=)` DENTRO del mismo `plane3d`, porque ahí el from/to sigue siendo
% ángulo DEL PLANO, no de la página.
%
% 📌 Los cortes caen sobre el LIMBO por construcción: el borde de visibilidad es el
% plano P.w = 0, y su intersección con la esfera ES el limbo. No hay nada que ajustar.
% Los dos casos salen sin trabajo: un meridiano pasa por el centro, así que A = 0 y
% tc = 90° exactos (media elipse, y su mitad visible contiene el polo norte, por lo
% que los cuatro convergen arriba); en un paralelo A/M = tan(bet)*tan(ph), y uno con
% bet < -(90-ph) queda entero detrás, que el clamp cubre con tc = 0 sin necesitar un
% `if`. Un arco de barrido cero es un resultado legítimo y los tres backends lo
% toleran (lo fija test/errors/arco_barrido_cero.mg).
%
% LA FASE DE LA RETÍCULA, que es lo único derivado de dos cosas a la vez. Para que un
% meridiano parta el casquete por la mitad como en el original NO basta ponerlo sobre
% el plano de un meridiano (gam = lam): ese plano corta el disco por su centro en el
% ESPACIO, pero la circunferencia dibujada va a radio 1 mientras el centro del disco
% está a rho, así que el arco pasa POR ENCIMA, descentrado ~0.29. Lo que hace falta es
% que el arco cruce el centro PROYECTADO, y eso es un PUNTO de la esfera, no un plano:
% el rayo visual por el centro del casquete la corta en dos puntos y el DELANTERO
% —el que no se oculta— fija la longitud,
%     |C + s*w| = 1   =>   s = -(C.w) + sqrt((C.w)² + 1 - rho²)
% Se gasta el parámetro de la RETÍCULA y no el del casquete: la fase de los meridianos
% es libre —nada en la física elige la longitud cero— y la posición del casquete es el
% asunto de la figura. Y como sale de la cámara, la propiedad sobrevive a cambiarla:
% verificado a 35°/38°, el meridiano sigue partiendo el casquete.
%
% LO QUE **NO** SE OCULTA, y es MODELADO, no geometría: el casquete se dibuja ENTERO
% aunque parte de su borde quede detrás. Es un disco suelto dentro de una esfera de
% alambre, no una marca sobre su superficie, y recortarlo mutilaría el contorno del
% ángulo sólido, que es lo que la figura tiene que enseñar. El eje polar y el limbo,
% por lo mismo, van en coordenadas de PÁGINA: el limbo es la silueta (un círculo de
% radio R sea cual sea la cámara) y el eje es línea de construcción, cuyo largo es
% decisión de encuadre. Sí se cuidó que el casquete caiga en el hemisferio CERCANO
% (uu < 0): con la mitad trasera oculta, uno lejano se leería como un error.
% El original de Lira no oculta nada —es alambre transparente—: ocultar es una mejora
% deliberada sobre la fuente.
%
% VERIFICADOR: `python3 tools/arcparity.py <eps> <svg> <pdf>` debe pasar. Las elipses
% de aquí tienen semidiámetros conjugados genuinamente OBLICUOS (u.v != 0), que es el
% caso en que EPS traza con matriz, SVG resuelve un SVD 2x2 y PDF transforma puntos de
% control: tres caminos distintos. Si uno se desvía, esta figura lo caza; el golden no,
% porque bendeciría los tres.
%
% COBERTURA EXCLUSIVA: es el primer ejemplo del corpus con `view3d`, `plane3d` y
% `xyz()`. Si sale, esas tres se quedan sin prueba.

display_size 10 10.2
world_window -1.08 1.45 -1.28 1.30

% --- la cámara ---
% Los grados se escriben una vez: view3d los toma y la trigonometría de la
% visibilidad los reusa en radianes.
azd = 15                           % acimut
eld = 20                           % elevación
view3d(azimuth=azd, elevation=eld)
th = rad(azd)
ph = rad(eld)
st = sin(th)   ct = cos(th)
sp = sin(ph)   cp = cos(ph)

% --- el eje polar y el limbo, en coordenadas de página ---
line_width 0.3
polyline(dash="dashdot") { 0 -1.2  0 1.22 }
line_width 0.5
circle(1) { 0 0 }

% --- el casquete: dirección de rho por acimut gam y elevación bb ---
% El disco es perpendicular a rho, así que su plano lo generan e1 (horizontal) y
% e2 = rho^ x e1. Se dibuja más abajo, después de la retícula.
rho = 0.62
rr  = 0.56
uu  = rad(-40)               % = th - gam;  uu < 0 lo pone en el hemisferio cercano
bb  = rad(6.5)
gam = th - uu
sb = sin(bb)   cb = cos(bb)
sg = sin(gam)  cg = cos(gam)

dx  = cb*cg    dy  = sb    dz  = cb*sg          % rho^, unitario
p1x = -sg      p1y = 0     p1z = cg             % e1
p2x = -sb*cg   p2y = cb    p2z = -sb*sg         % e2

ccx = rho*dx   ccy = rho*dy   ccz = rho*dz      % centro del casquete

% --- la longitud del meridiano que parte el casquete por la mitad ---
% El rayo visual por el centro del casquete corta la esfera; la raíz delantera da
% la longitud de la que se cuelga toda la retícula.
wx = -st*cp    wy = sp     wz = ct*cp           % w: dirección hacia el observador
cw = ccx*wx + ccy*wy + ccz*wz
ss = -cw + sqrt(cw*cw + 1 - rho*rho)
lam0 = atan2(ccz + ss*wz, ccx + ss*wx)

% --- meridianos: el plano contiene el eje polar, girado la longitud lam ---
line_width 0.3
for i = -1 to 2 {
    lam = lam0 + rad(i * 45)
    psi = deg(atan2(sp, cp * sin(lam - th)))
    v0  = psi - 90
    v1  = psi + 90
    {
        plane3d(u=[cos(lam), 0, sin(lam)], v=[0, 1, 0])
        arc(1, from=(v0), to=(v1)) { 0 0 }
    }
}

% --- paralelos: plano horizontal a la altura sin(bet), radio cos(bet) ---
for j = 0 to 4 {
    bet = rad((j - 2) * 30)
    rp  = cos(bet)
    qq  = -sin(bet) * sp / (rp * cp)
    qc  = (abs(qq + 1) - abs(qq - 1)) / 2      % clamp a [-1,1]
    tc  = deg(acos(qc))
    psi = deg(atan2(ct, -st))
    v0  = psi - tc
    v1  = psi + tc
    {
        plane3d(at=[0, sin(bet), 0], u=[rp, 0, 0], v=[0, 0, rp])
        arc(1, from=(v0), to=(v1)) { 0 0 }
    }
}

% --- el casquete A = pi r^2 ---
line_width 0.5
{
    plane3d(at=[ccx, ccy, ccz], u=[rr*p1x, rr*p1y, rr*p1z], v=[rr*p2x, rr*p2y, rr*p2z])
    circle(1) { 0 0 }
}

% --- las cotas: no pertenecen a ningún plano, así que van con xyz() ---
ps = rad(168)                            % el borde al que apunta r
tpx = ccx + rr*p2x   tpy = ccy + rr*p2y   tpz = ccz + rr*p2z    % borde "arriba"
bpx = ccx - rr*p2x   bpy = ccy - rr*p2y   bpz = ccz - rr*p2z    % borde "abajo"
epx = ccx + rr*(cos(ps)*p1x + sin(ps)*p2x)
epy = ccy + rr*(cos(ps)*p1y + sin(ps)*p2y)
epz = ccz + rr*(cos(ps)*p1z + sin(ps)*p2z)

line_width 0.3
polyline(dash="dotted") { xyz(0,0,0)   xyz(tpx,tpy,tpz) }        % rho, hasta un borde
polyline(dash="dotted") { xyz(0,0,0)   xyz(bpx,bpy,bpz) }        % ...y hasta el opuesto
polyline(dash="dotted") { xyz(ccx,ccy,ccz)   xyz(epx,epy,epz) }  % r, del centro al borde

% --- rótulos ---
% Cada uno se aparta perpendicularmente de su propia cota, en la página, así que
% sigue puesto si se cambia la cámara: ni una coordenada de rótulo medida a ojo.
pT = xyz(tpx,tpy,tpz)
pC = xyz(ccx,ccy,ccz)
pE = xyz(epx,epy,epz)

hn  = sqrt(pT[0]*pT[0] + pT[1]*pT[1])
rlx = 0.50*pT[0] + 0.16*pT[1]/hn
rly = 0.50*pT[1] - 0.16*pT[0]/hn

dxr = pE[0] - pC[0]
dyr = pE[1] - pC[1]
hr  = sqrt(dxr*dxr + dyr*dyr)
elx = pC[0] + 0.62*dxr + 0.13*dyr/hr
ely = pC[1] + 0.62*dyr - 0.13*dxr/hr

font_size 11
align "right"
text("o") { -0.05 -0.05 }

align "center"
text("$\rho$") { (rlx) (rly) }
text("$r$")    { (elx) (ely) }

% La flecha de A = pi r^2 entra desde fuera, por arriba a la derecha.
align "left"
text("$A = \pi r^2$") { 0.80 1.13 }
line_width 0.4
polyline(marker_end="arrow", marker_size=3.5) { 0.89 1.06   0.70 0.30 }
