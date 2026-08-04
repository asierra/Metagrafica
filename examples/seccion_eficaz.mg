% Significado geométrico de la sección eficaz
%
% El haz llega por el eje z con parámetro de impacto b, dispersa en el blanco, y un
% detector cilíndrico recoge lo que sale en la dirección (θ, φ). La sección eficaz se
% dibuja como lo que ES: el área A por la que hay que pasar para salir dispersado,
% con el canal del haz muriendo justo sobre ella. Las dos siluetas de revolución —el
% cono de dΩ y el cilindro— salen de la escena con `include "../lib/pseudo3d.mg"`, y
% los ejes rotulados x/y/z son CONTENIDO de la figura, no el marco de la cámara.
%
% NOTAS ————————————————————————————————————————————————————————————————
%
% PROCEDENCIA: Fig. 20.5 de IMQ 3a ed. («Significado geométrico de la seccion
% eficaz»); la edición de Cambridge publica la misma con los rótulos en inglés
% —`target` en vez de `blanco`—, y por eso aquí van en español.
% RECONSTRUIDA, NO digitalizada. Del original se tomó la COMPOSICIÓN, no las
% coordenadas: el mapeo de ejes (x arriba, z a la derecha, y al frente), el haz
% ASIMÉTRICO —más rayos por encima del eje que por debajo, y detenidos antes de que
% empiece el eje y, que es como el original evita que se crucen—, el canal estrecho
% que sí llega hasta el blanco, el blanco sin relleno, y dΩ como disco propio a media
% distancia en vez de sobre la boca del detector.
% ⚠️ Por eso NO lleva número de figura: el número promete fidelidad, y los ángulos
% de aquí (θ=38°, φ=35°) se eligieron para que la página se lea, no midiendo el
% original.
%
% ⚠️ DOS MARCOS QUE NO SON EL MISMO, y es el error que esta figura existe para no
% cometer. La ESCENA de MetaGráfica es x a la derecha, y arriba, z = profundidad
% (§13). La FÍSICA de la dispersión quiere el haz sobre z^ y el plano transversal
% en x^-y^. Se escriben los tres versores de la física EN coordenadas de la escena
% —una vez, arriba— y todo lo demás se deriva de ellos. Cambiar la orientación de
% la figura es cambiar esas tres líneas; ninguna otra cuenta las conoce.
%
% POR QUÉ θ Y φ SE DIBUJAN EN SU PROPIO PLANO, y no con una flecha doble sobre el
% papel: el ángulo entre dos direcciones proyectadas NO es la proyección del ángulo
% del espacio. El arco de θ vive en el plano de z^ y n^, y el de φ en el plano
% transversal x^-y^; en los dos, el barrido del `arc` ES el ángulo, escrito en
% grados y sin convertir nada. Es la lección de `irradiancia`, aplicada dos veces.
% 📌 Tiene un costo medido y aceptado: el original dibuja arcos PLANOS, así que sus
% ángulos se leen más abiertos (43.7° y 39.9° en la página) que los de aquí (33° y
% 24° proyectados). Si hicieran falta más abiertos se suben θ y φ o se abre la
% cámara; lo que no se hace es aplanar el arco, que es mentir con precisión.
%
% VERIFICADOR INCORPORADO: la base del cono de dΩ y la cara delantera del cilindro
% son EL MISMO círculo —mismo centro, mismo radio, mismo plano—, así que el cono va
% con `base=false` y lo dibuja el cilindro. Lo que eso deja comprobable es el borde:
% `lib/pseudo3d.mg` calcula las dos siluetas por caminos DISTINTOS —el cono
% retro-proyecta el ápice y resuelve un `acos`; el cilindro retro-proyecta el otro
% centro y gira 90°— y solo comparten el marco de la base. Si ese marco se rompiera,
% las generatrices del cono dejarían de morir TANGENTES sobre el círculo que dibuja
% el cilindro, y se vería. Hoy mueren donde deben.
%
% ⚠️ POR QUÉ LAS VARIABLES LLEVAN PREFIJO (`tqx`, `dcx`, `trx`) Y NO EL NOMBRE OBVIO.
% Las asignaciones del CUERPO de una struct escriben en el ámbito de quien la llama.
% Los parámetros sí están aislados; las asignaciones internas no. `cono` y `cilindro`
% asignan `qx`/`qy` para su retro-proyección, así que un `qx` del llamador se pierde
% en la invocación, sin aviso y sin error: la figura simplemente sale mal. Aquí costó
% un rato — el rótulo de φ caía a 58° cuando la cuenta daba 102°, y θ salía bien
% porque no depende de `q`. Anotado en PENDIENTES.md; mientras siga así, un `.mg` que
% incluya una biblioteca no puede usar nombres cortos con confianza.
%
% COBERTURA EXCLUSIVA: único usuario de `lib/pseudo3d.mg` en el corpus, y por tanto
% de `cono` y `cilindro`. Esa biblioteca era la última de `lib/` sin ningún ejemplo
% que la incluyera: nada la compilaba, así que se podía pudrir en silencio.
% ⚠️ `irradiancia` NO la usa — lleva la silueta del cono escrita inline a propósito,
% para que se pueda VER la derivación. Ésta usa la biblioteca, que es el otro lado.

% Encuadre medido sobre el render, no acotado a ojo: la tinta cae en
% x −5.82..5.66, y −1.70..3.30, y la ventana le deja 0.26 de margen por los cuatro lados.
display_size 13 5.98
world_window -6.08 5.92 -1.96 3.56

include "../lib/pseudo3d.mg"

view3d(azimuth=-32, elevation=20)

% --- parámetros de la escena ---
thd = 38          % θ: ángulo de dispersión, desde el haz
phd = 35          % φ: acimut del plano de dispersión
RR  = 6.6         % blanco → cara del detector
rd  = 0.95        % radio del detector
Ld  = 3.4         % largo del detector
rt  = 0.75        % radio del blanco

th = rad(thd)   ph = rad(phd)

% --- el marco de la FÍSICA, escrito en coordenadas de la ESCENA ---
% ⚠️ Aquí es donde los dos marcos se reconcilian, y es el único sitio. Es un triedro
% derecho: x^ × y^ = z^.
ezx = 1   ezy = 0   ezz = 0        % z^ = haz incidente → a la derecha
exx = 0   exy = 1   exz = 0        % x^                 → hacia arriba
eyx = 0   eyy = 0   eyz = 1        % y^                 → hacia el observador

% n^: la dirección de observación, (θ, φ) en el marco de la física
nx = cos(th)*ezx + sin(th)*cos(ph)*exx + sin(th)*sin(ph)*eyx
ny = cos(th)*ezy + sin(th)*cos(ph)*exy + sin(th)*sin(ph)*eyy
nz = cos(th)*ezz + sin(th)*cos(ph)*exz + sin(th)*sin(ph)*eyz

% q^: la componente transversal de n^, ya unitaria (es cos φ·x^ + sin φ·y^).
% Sirve para dos cosas: el segundo eje del plano donde vive el arco de θ, y la
% dirección de la cateta del triángulo que hace visible a φ.
% ⚠️ Los nombres llevan prefijo a propósito — ver las NOTAS sobre el ámbito.
tqx = cos(ph)*exx + sin(ph)*eyx
tqy = cos(ph)*exy + sin(ph)*eyy
tqz = cos(ph)*exz + sin(ph)*eyz

% Centro de la cara del detector
dcx = RR*nx   dcy = RR*ny   dcz = RR*nz

% El triángulo que hace visible a φ se construye a un radio CORTO, no hasta el
% detector: es andamio, y llevado hasta RR cruzaba el cono de lado a lado.
L0 = 3.4
trx = L0*sin(th)*tqx   try = L0*sin(th)*tqy   trz = L0*sin(th)*tqz
tcx = L0*nx   tcy = L0*ny   tcz = L0*nz

% --- el haz incidente, detrás de todo ---
% Cinco rayos paralelos, y entre ellos el CANAL estrecho de sección A: cualquier
% partícula que entre por ese tubo sale dispersada, y eso —que haya un área tal que
% lo que pasa por ella se dispersa— es lo que significa que la sección eficaz sea un
% ÁREA. El canal llega hasta el blanco; los rayos se detienen antes para no taparlo.
rA = 0.18                            % radio del área efectiva = semiancho del canal
% ⚠️ GROSORES, y el original los ordena a propósito: el haz es lo MÁS FINO (`LWIDTH 0`
% en el V1), los ejes van en medio (`LWIDTH 1`) y el detector es lo más grueso
% (`LWIDTH 3`, dentro de su struct). Con todo al mismo grosor el haz compite con el
% cuerpo del detector y la figura se aplana.
color "black"
line_width 0.4
for k = -1 to 3 {
    off = 0.5*k
    polyline(marker_end="arrow", marker_size=3.5) {
        xyz(-5.85, off*exy, off*exz)
        xyz(-2.00, off*exy, off*exz) }
}
line_width 0.3
polyline { xyz(-5.85,  rA*exy,  rA*exz)   xyz(0,  rA*exy,  rA*exz) }
polyline { xyz(-5.85, -rA*exy, -rA*exz)   xyz(0, -rA*exy, -rA*exz) }

% --- el parámetro de impacto ---
% b se acota a la ENTRADA, donde el rayo aún no se ha desviado, y va del eje del haz
% al rayo: es la distancia al eje con la que llega la partícula. La cota se traza
% entre dos puntos de la ESCENA proyectados, pero sus serifas son marcas de PÁGINA
% —una acotación se lee en el papel, no en el espacio—.
imp0 = xyz(-6.30, 0, 0)
imp1 = xyz(-6.30, 1.0*exy, 1.0*exz)
line_width 0.5
polyline { (imp0[0]) (imp0[1])   (imp1[0]) (imp1[1]) }
polyline { (imp0[0]-0.20) (imp0[1])   (imp0[0]+0.20) (imp0[1]) }
polyline { (imp1[0]-0.20) (imp1[1])   (imp1[0]+0.20) (imp1[1]) }

% --- la triada de ejes ---
% Tres segmentos y tres rótulos: la forma mínima de una jaula de ejes, que es toda
% la que esta figura necesita.
line_width 0.6
polyline(marker_end="arrow", marker_size=4) { xyz(0,0,0)   xyz(3.6*ezx, 3.6*ezy, 3.6*ezz) }
polyline(marker_end="arrow", marker_size=4) { xyz(0,0,0)   xyz(3.0*exx, 3.0*exy, 3.0*exz) }
polyline(marker_end="arrow", marker_size=4) { xyz(0,0,0)   xyz(3.0*eyx, 3.0*eyy, 3.0*eyz) }

% --- el eje del haz dispersado, y la construcción que hace visible a φ ---
% ⚠️ El eje de n^ NO es decorativo: sin él el arco de θ muere en el aire entre las dos
% generatrices del cono, y un ángulo que no termina sobre nada se lee como incompleto.
% Los dos arcos nacen sobre un eje dibujado y mueren sobre otro, que es la condición
% para que midan algo a la vista y no solo en la cuenta.
line_width 0.5
polyline(dash="dashed") { xyz(0,0,0)   xyz(dcx + Ld*nx, dcy + Ld*ny, dcz + Ld*nz) }
% El triángulo rectángulo: del origen a la proyección transversal, y de ahí paralelo
% al haz hasta el eje de observación. Discontinuo porque es andamio, pero NEGRO: en
% gris claro el arco de φ parecía morir en la nada.
polyline(dash="dashed") { xyz(0,0,0)   xyz(trx, try, trz) }
polyline(dash="dashed") { xyz(trx, try, trz)   xyz(tcx, tcy, tcz) }

% --- el blanco, y el área efectiva sobre él ---
% Una lámina delgada de canto al haz: un disco del plano transversal x^-y^. El gris
% va SIN RELLENO, como en el original: relleno se tragaba el punto donde convergen los
% rayos —el suceso que la figura cuenta— y escondía la A que va justo encima.
{ plane3d(at=[0,0,0], u=[rt*exx, rt*exy, rt*exz], v=[rt*eyx, rt*eyy, rt*eyz])
  circle(1, color="black") { 0 0 } }
% A es la sección eficaz dibujada como lo que es: el área que el blanco le presenta
% al haz. Va en el mismo plano transversal y con el radio del canal, así que el tubo
% incidente muere exactamente sobre ella — es la misma área vista de canto y de frente.
line_width 0.6
{ plane3d(at=[0,0,0], u=[rA*exx, rA*exy, rA*exz], v=[rA*eyx, rA*eyy, rA*eyz])
  circle(1) { 0 0 } }

% --- el cono de dΩ y el detector ---
% `base=false`: la base del cono es la cara delantera del cilindro, y la dibuja él.
% ⚠️ El cono va FINO como el haz incidente, no grueso: en el original el haz dispersado
% es `LWIDTH 0` igual que el incidente, y solo el BARRIL del detector es `LWIDTH 3`. Los
% dos haces son lo mismo —trayectorias—; el detector es el único objeto sólido.
cono(rd, axis=[dcx, dcy, dcz], pos=[0, 0, 0], base=false, lw=0.4)
cilindro(rd, axis=[Ld*nx, Ld*ny, Ld*nz], pos=[dcx, dcy, dcz], lw=0.9)

% dΩ es la sección del cono a la fracción tt del recorrido: mismo eje, radio tt·rd.
% Va como disco PROPIO y no sobre la boca del detector —que es lo que hace el
% original— porque así el rótulo tiene a qué agarrarse: la boca está medio oculta por
% el tubo y no se deja rotular. ⚠️ Y es el segundo verificador de la figura: por
% construcción tiene que salir TANGENTE a las dos generatrices; si la silueta se
% rompe, deja de estarlo y se ve.
tt = 0.55
hn  = sqrt(nz*nz + nx*nx)
w1x = nz/hn            w1y = 0                w1z = -nx/hn
w2x = ny*w1z - nz*w1y  w2y = nz*w1x - nx*w1z  w2z = nx*w1y - ny*w1x
line_width 0.5
{ plane3d(at=[tt*dcx, tt*dcy, tt*dcz],
          u=[tt*rd*w1x, tt*rd*w1y, tt*rd*w1z], v=[tt*rd*w2x, tt*rd*w2y, tt*rd*w2z])
  circle(1) { 0 0 } }

% --- los dos ángulos, cada uno en SU plano ---
% θ: el plano de z^ y n^. Su segundo eje es q^, así que el barrido es θ directo.
line_width 0.5
{ plane3d(u=[2.1*ezx, 2.1*ezy, 2.1*ezz], v=[2.1*tqx, 2.1*tqy, 2.1*tqz])
  arc(1, from=0, to=(thd)) { 0 0 } }
% φ: el plano transversal. Sus ejes son x^ e y^, así que el barrido es φ directo.
{ plane3d(u=[1.35*exx, 1.35*exy, 1.35*exz], v=[1.35*eyx, 1.35*eyy, 1.35*eyz])
  arc(1, from=0, to=(phd)) { 0 0 } }

% --- rótulos ---
% Los de los ángulos van en la bisectriz DE LA PÁGINA, no en la del espacio: la
% proyección no conserva la bisección. El arco mide; el rótulo solo señala.
pZ = xyz(ezx, ezy, ezz)
pN = xyz(nx, ny, nz)
pX = xyz(exx, exy, exz)
pQ = xyz(tqx, tqy, tqz)

font_size 12
align "center"
% θ va pegada a SU arco y CORRIDA hacia el eje z, no en la bisectriz: en la bisectriz
% se embarra en el haz dispersado. El original hace lo mismo —su rótulo cae como a un
% cuarto de la cuña, no a la mitad—. Se toma el punto del arco a esa fracción y se sale
% un poco por el radio, igual que los rótulos de los ejes.
fTh = 0.32
pTh = xyz(2.1*(cos(fTh*th)*ezx + sin(fTh*th)*tqx),
          2.1*(cos(fTh*th)*ezy + sin(fTh*th)*tqy),
          2.1*(cos(fTh*th)*ezz + sin(fTh*th)*tqz))
hTh = sqrt(pTh[0]*pTh[0] + pTh[1]*pTh[1])
text("$\theta$") { (pTh[0] + 0.44*pTh[0]/hTh) (pTh[1] + 0.44*pTh[1]/hTh - 0.12) }
amf = (atan2(pX[1], pX[0]) + atan2(pQ[1], pQ[0])) / 2
text("$\phi$") { (1.68*cos(amf)) (1.68*sin(amf)) }

% Los ejes se rotulan un poco más allá de su punta, en la misma dirección.
pEz = xyz(3.6*ezx, 3.6*ezy, 3.6*ezz)
pEx = xyz(3.0*exx, 3.0*exy, 3.0*exz)
pEy = xyz(3.0*eyx, 3.0*eyy, 3.0*eyz)
hz = sqrt(pEz[0]*pEz[0] + pEz[1]*pEz[1])
hx = sqrt(pEx[0]*pEx[0] + pEx[1]*pEx[1])
hy = sqrt(pEy[0]*pEy[0] + pEy[1]*pEy[1])
text("$z$") { (pEz[0] + 0.42*pEz[0]/hz) (pEz[1] + 0.42*pEz[1]/hz - 0.12) }
text("$x$") { (pEx[0] + 0.42*pEx[0]/hx) (pEx[1] + 0.42*pEx[1]/hx - 0.12) }
text("$y$") { (pEy[0] + 0.42*pEy[0]/hy) (pEy[1] + 0.42*pEy[1]/hy - 0.12) }

% dΩ rotula SU disco, apartándose perpendicularmente del eje en la página — arriba a
% la izquierda, del lado libre, como en el original.
pC = xyz(tt*dcx, tt*dcy, tt*dcz)
hC = sqrt(pC[0]*pC[0] + pC[1]*pC[1])
text("$d\Omega$", align="right") { (pC[0] - 0.62*pC[1]/hC - 0.10) (pC[1] + 0.62*pC[0]/hC + 0.10) }

% A y b son símbolos, así que van en modo matemático como θ, φ y dΩ. `a` cuelga de una
% guía curva —el original usa un cuarto de arco— porque rotula el ÁREA del blanco, que
% no tiene un punto propio donde ponerle el nombre.
align "right"
text("$A$") { -0.42 -1.02 }
text("$b$") { (imp0[0] - 0.32) ((imp0[1] + imp1[1])/2 - 0.16) }
line_width 0.4
smooth { 0.55 -1.52   0.30 -1.22   -0.02 -0.82 }
align "left"
text("$a$") { 0.62 -1.70 }

font_size 11
align "left"
text("blanco") { 0.50 -0.70 }
text("haz incidente") { -4.95 2.85 }
pD = xyz(dcx + Ld*nx, dcy + Ld*ny, dcz + Ld*nz)
align "left"
text("detector") { (pD[0] - 0.38) (pD[1] - 1.58) }
