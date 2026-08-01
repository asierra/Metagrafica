% Ley de la irradiancia — el ángulo sólido que dA subtiende desde P
%
% Un parche ΔP sobre una superficie, su normal, y el cono que va de P al elemento de
% área dA a distancia r. Lo que esta figura enseña es que una SILUETA se puede
% calcular: las dos generatrices del cono son las tangentes desde P al borde de dA, y
% salen en forma cerrada sin que el lenguaje tenga que saber de conos. Y que un
% ÁNGULO del espacio se dibuja en su propio plano —el que contiene a la normal y al
% eje—, donde el barrido del arco ES el ángulo y la elipse que se ve la calcula la
% proyección.
%
% NOTAS ————————————————————————————————————————————————————————————————
%
% PROCEDENCIA: Lira, fig. II-7. RECONSTRUIDA, no digitalizada: proporciones y cámara
% se eligieron para que la página se lea bien, no midiendo el escaneo.
%
% MEJORA DELIBERADA SOBRE EL ORIGINAL: allí φ se marca con una flecha doble PLANA, en
% el papel, y eso miente un poco — el ángulo entre dos direcciones proyectadas no es
% la proyección del ángulo del espacio. Aquí el arco vive en el plano de n^ y el eje,
% así que se escribe `from=0 to=fid` sin convertir nada y mide de verdad.
%
% CÓMO SALE LA SILUETA, que es la pieza que vale. La proyección restringida al plano
% del disco es una AFINIDAD, y la tangencia es invariante afín. Así que en vez de
% resolver «tangente a una elipse desde un punto» en la página, se retro-proyecta el
% ápice al marco del disco —un 2x2— y allí el borde vuelve a ser un CÍRCULO, donde la
% tangencia es de secundaria: los ángulos son atan2(b,a) ± acos(r/D). Con u = ra*e1 y
% v = ra*e2 el círculo tiene radio 1 y queda acos(1/D).
% 📌 Los puntos de tangencia se devuelven al ESPACIO y las generatrices se dibujan con
% `xyz()` en sus dos extremos, porque son rectas de verdad del cono: la
% retro-proyección solo sirve para encontrar el ángulo, no para dibujar.
% ⚠️ Pide D > 1 (el ápice fuera del disco proyectado). Si no, se está mirando el cono
% por dentro y no hay silueta que trazar; `acos` avisaría abortando.
%
% VERIFICADOR INCORPORADO: dΩ es la sección del cono a la fracción tt, o sea de radio
% tt*ra. Tiene que salir TANGENTE a las dos generatrices por construcción. Si la
% derivación se rompe, deja de estarlo y se ve.
%
% COBERTURA EXCLUSIVA: es el único ejemplo con una SILUETA de revolución calculada, y
% el único que dibuja un ángulo del espacio en su plano en vez de anotarlo en el
% papel. También el único usuario de `acos` en el corpus.
%
% ⚠️ El sombrero de n^ va DIBUJADO con una polilínea: no hay `\hat` ni acentos
% combinantes en el modo matemático (U+0302 no tiene glifo y el compilador lo descarta
% con aviso). Es una carencia del lenguaje, no una decisión de la figura.

% Encuadre medido sobre el render, no acotado a ojo: la tinta cae en
% x −2.12..3.74, y −1.24..6.09.
display_size 9.5 11.7
world_window -2.37 3.99 -1.49 6.34

view3d(azimuth=-18, elevation=22)

% --- la escena ---
fid = 28                           % ángulo phi entre la normal y el eje del cono
psd = 24                           % giro del eje alrededor de la normal
RR  = 7.4                          % distancia P → dA
ra  = 1.24                         % radio de dA (ra/RR = 0.17, como el original)
tt  = 0.55                         % dΩ es la sección del cono a esta fracción

fi = rad(fid)   ps = rad(psd)

% Eje del cono: la normal es y^, y el eje se inclina fi grados en el acimut ps.
dx = sin(fi)*cos(ps)   dy = cos(fi)   dz = sin(fi)*sin(ps)

% Marco del disco: e1 = n^ x d^ (horizontal), e2 = d^ x e1. Ambos unitarios.
h1 = sqrt(dz*dz + dx*dx)
e1x = dz/h1            e1y = 0                e1z = -dx/h1
e2x = dy*e1z - dz*e1y  e2y = dz*e1x - dx*e1z  e2z = dx*e1y - dy*e1x

% Centro de dA
cx = RR*dx   cy = RR*dy   cz = RR*dz

% --- la silueta del cono ---
% El ápice se retro-proyecta al marco del disco, donde el borde es un círculo y la
% tangencia sale en forma cerrada (la derivación, en las NOTAS).
pO = xyz(cx, cy, cz)
pU = xyz(cx + ra*e1x, cy + ra*e1y, cz + ra*e1z)
pV = xyz(cx + ra*e2x, cy + ra*e2y, cz + ra*e2z)
pA = xyz(0, 0, 0)

ux = pU[0] - pO[0]   uy = pU[1] - pO[1]
vx = pV[0] - pO[0]   vy = pV[1] - pO[1]
det = ux*vy - uy*vx
qx = pA[0] - pO[0]   qy = pA[1] - pO[1]
aa = ( qx*vy - qy*vx) / det
bb = (-qx*uy + qy*ux) / det
DD = sqrt(aa*aa + bb*bb)

t0 = atan2(bb, aa)
dt = acos(1/DD)

% Los dos puntos de tangencia, ya en el espacio: las generatrices son rectas de
% verdad del cono, así que se dibujan con xyz() en sus dos extremos.
g1 = t0 + dt
g2 = t0 - dt
t1x = cx + ra*(cos(g1)*e1x + sin(g1)*e2x)
t1y = cy + ra*(cos(g1)*e1y + sin(g1)*e2y)
t1z = cz + ra*(cos(g1)*e1z + sin(g1)*e2z)
t2x = cx + ra*(cos(g2)*e1x + sin(g2)*e2x)
t2y = cy + ra*(cos(g2)*e1y + sin(g2)*e2y)
t2z = cz + ra*(cos(g2)*e1z + sin(g2)*e2z)

% --- el parche ΔP y la superficie, abajo ---
% ΔP es un plano de la escena: el perpendicular a la normal que pasa por P.
color "black"
line_width 0.6
{ plane3d(at=[-0.52, 0, -0.52], u=[1.04, 0, 0], v=[0, 0, 1.04])
  polygon(color="black", fill="white") { 0 0  1 0  1 1  0 1 } }

% La superficie es contorno del PAPEL: no tiene geometría que derivar.
line_width 0.7
smooth(closed=true) { -2.10 -0.21  -1.74 0.45  -0.90 0.33  -0.12 0.78
                       0.90 0.63   1.56 0.09   1.92 -0.57   0.96 -1.05
                      -0.12 -0.87  -0.78 -1.23  -1.56 -0.96 }

% --- el cono: dA, sus dos generatrices, y dΩ ---
line_width 0.7
polyline { xyz(0,0,0)   xyz(t1x, t1y, t1z) }
polyline { xyz(0,0,0)   xyz(t2x, t2y, t2z) }
{ plane3d(at=[cx, cy, cz], u=[ra*e1x, ra*e1y, ra*e1z], v=[ra*e2x, ra*e2y, ra*e2z])
  circle(1) { 0 0 } }

% dΩ es la sección del cono a la fracción tt: mismo eje, radio tt*ra. Tiene que salir
% TANGENTE a las dos generatrices por construcción — si no lo está, la silueta es
% incorrecta, así que la figura trae su propio verificador.
line_width 0.5
{ plane3d(at=[tt*cx, tt*cy, tt*cz],
          u=[tt*ra*e1x, tt*ra*e1y, tt*ra*e1z], v=[tt*ra*e2x, tt*ra*e2y, tt*ra*e2z])
  circle(1, dash="dotted") { 0 0 } }

% --- la normal y el radio ---
line_width 0.7
polyline(marker_end="arrow", marker_size=4) { xyz(0,0,0)   xyz(0, 3.5, 0) }
polyline(marker_end="arrow", marker_size=4) { xyz(0,0,0)   xyz(cx, cy, cz) }

% --- el ángulo phi, en SU plano ---
% El marco es u = n^ y v = la componente del eje perpendicular a n^, así que el
% barrido del arco es phi directamente, en grados y sin convertir nada.
wx = dx/sin(fi)   wz = dz/sin(fi)          % d^ sin su componente a lo largo de n^
rf = 1.75                                  % radio de la marca
line_width 0.5
{ plane3d(u=[0, rf, 0], v=[rf*wx, 0, rf*wz])
  arc(1, from=0, to=(fid),
      marker_start="arrow", marker_end="arrow", marker_size=3.5, marker_start_orient="reverse") { 0 0 } }

% --- rótulos ---
pC  = xyz(cx, cy, cz)
pD  = xyz(tt*cx, tt*cy, tt*cz)
pNt = xyz(0, 3.5, 0)

font_size 12
align "center"
text("$dA$") { (pC[0]) (pC[1] + 0.2) }
text("$d\Omega$", align="right") { (pD[0] - 0.05) (pD[1] + 0.05) }
% El sombrero se dibuja (ver NOTAS), relativo al ancla del rótulo.
nlx = pNt[0] - 0.32   nly = pNt[1] - 1.05
text("$n$") { (nlx) (nly) }
line_width 0.5
polyline { (nlx - 0.24) (nly + 0.34)   (nlx - 0.13) (nly + 0.47)   (nlx - 0.02) (nly + 0.34) }
align "center"
% `r` se aparta PERPENDICULARMENTE de su propia cota, en la página, así que sigue
% puesta si se mueve la cámara.
pR = xyz(0.40*cx, 0.40*cy, 0.40*cz)
hR = sqrt(pC[0]*pC[0] + pC[1]*pC[1])
text("$r$") { (pR[0] + 0.34*pC[1]/hR) (pR[1] - 0.34*pC[0]/hR) }
% El rótulo va en la bisectriz DE LA PÁGINA, no en la del espacio: la proyección no
% conserva la bisección, y puesto en la real caía sobre la flecha de n^. El arco mide;
% el rótulo solo señala.
align "center"
pNd = xyz(0, 1, 0)
% Y se bisecta contra la GENERATRIZ más cercana a n^, no contra el eje: el hueco libre
% lo bordea el cono. Medido desde P, en la página: n^ a 90°, generatriz a 75.7°, eje a
% 63.5° — el cono se come casi toda la cuña.
pG1 = xyz(t1x, t1y, t1z)
pG2 = xyz(t2x, t2y, t2z)
an  = atan2(pNd[1], pNd[0])
ag  = atan2(pG1[1], pG1[0])
if abs(atan2(pG2[1], pG2[0]) - an) < abs(ag - an) { ag = atan2(pG2[1], pG2[0]) }
am  = (an + ag) / 2
rl  = 1.95                                 % más cerca del arco que marca el ángulo
text("$\phi$") { (rl*cos(am)) (rl*sin(am)) }

% P se despega del parche por su propia esquina: se toma la esquina cercana ya
% proyectada y se sigue un poco más en esa dirección.
pQ = xyz(0.52, 0, 0.52)
hQ = sqrt(pQ[0]*pQ[0] + pQ[1]*pQ[1])
text("$P$") { (pQ[0] + 0.42*pQ[0]/hQ) (pQ[1] + 0.42*pQ[1]/hQ - 0.12) }
align "right"
text("$\Delta P$") { -0.72 -0.62 }
