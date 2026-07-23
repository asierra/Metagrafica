% Comportamiento de ψ según la energía — los puntos de retorno clásicos.
%
% Oscilatoria donde E > V, exponencial donde E < V, y los puntos de retorno como
% frontera entre ambas. Figura PARAMÉTRICA: se dan las asíntotas del potencial,
% su mínimo, los tres retornos y las tres energías, y de ahí salen en forma
% cerrada la curva V(x) y, por WKB, las longitudes de onda, las amplitudes y las
% colas.
%
% NOTAS --------------------------------------------------------------------
% Contraparte de la Fig. 4.5 de «Quantum Mechanics» (Cambridge, 2025), p. 80, que
% ilustra lo mismo con las ondas dibujadas a mano. NO lleva número de figura
% porque NO es un port fiel, y no puede serlo (medido sobre la figura publicada,
% 2026-07-20): las tres ondas del libro no pueden ser la misma partícula. La fase
% ∫k dx por lóbulo varía hasta 3× dentro de una misma onda —cada región se dibujó
% repitiendo un ciclo a escala elegida a ojo— y no existe constante de fase que
% reproduzca a la vez sus tres densidades: con ψ_c de 4 antinodos, los que tiene,
% la misma partícula exige ψ_a≈24 y ψ_b≈13 lóbulos, contra 13 y 8 dibujados. Aquí
% las tres COMPARTEN constante, que es la afirmación fuerte. El número de figura
% es una promesa de fidelidad, y por eso este archivo no lo usa.
%
% CÓMO RE-MEDIR LA FIGURA PUBLICADA. Es VECTORIAL en el PDF de Cambridge (no hay
% imágenes en la página), así que se extrae con
%     mutool draw -F trace -o - Quantum_Mechanics.pdf 94      (pág. 80 del libro)
% Su sistema de coordenadas se deduce de las tres verticales y las ocho
% horizontales, cuyos valores se conocen:
%     x = 0 → 245.89 pt,  x = 1 → 308.25 pt   (62.360 pt/unidad)
%     y = 5.2 → 77.99 pt, y = 4.9 →  85.67 pt (25.600 pt/unidad, hacia abajo)
% De ahí la anisotropía publicada, 62.360/25.600 = 2.4359, que es exactamente el
% aspecto de la ventana (9.5/3.9) y es el `kx` de abajo. Esa figura resultó ser el
% MISMO dibujo que el EPS original de 1998 —mismos conteos de segmentos
% (ψ_a=13, ψ_b=9, ψ_c=5) y mismos extremos dentro de 0.002 unidades—, así que el
% EPS se borró: éste es mejor oráculo, y es público.
%
% Los nodos con que se dibujaba V(x) a mano eran, por si hicieran falta:
%   -1.3 4.9  -1 4.9  -0.6 4.8  -0.35 4.5  0 3.7  0.25 3.3  0.576 3.25
%   0.7 3.35  1 3.7  1.15 3.85  1.36 4.075  1.6 4.225  2 4.35  2.3 4.4
% El V(x) analítico de abajo los reproduce con 0.9 pt de error máximo, por debajo
% del ruido de digitalización, y además clava los retornos en −0.35, 0 y 1.

display_size 8.5 8.5
font_size 8

% ── el potencial ─────────────────────────────────────────────────────────────
% Pozo asimétrico con asíntotas distintas a cada lado:
%     V(x) = V∞ − (V∞−Vm)·exp(−|(x−xm)/w|^s)
% Los anchos y exponentes NO se ajustan a ojo: se despejan exigiendo que la
% curva pase por los puntos de retorno rotulados. El lado izquierdo tiene dos
% condiciones —V(x0)=Eb y V(x1)=Ec— que fijan exactamente sus dos parámetros;
% el derecho tiene una, V(x2)=Ec, y su exponente qr queda como única forma libre.
Vl = 4.9        % V_-  asíntota izquierda
Vr = 4.4        % V_+  asíntota derecha
xm = 0.576      % posición del mínimo
Vm = 3.25       % profundidad del mínimo
qr = 1.4        % forma de la pared derecha (único parámetro de forma libre)
KAIRY = 0.11    % escala visual de la penetración; la ASIMETRÍA entre paredes
                % no depende de ella (sale de V'), solo la profundidad absoluta,
                % que sí llevaría m y ħ — igual que en franck_condon.mg

Ea = 5.2        % por encima de las dos asíntotas → oscila en todo x
Eb = 4.5        % entre V_+ y V_-  → un retorno, a la izquierda
Ec = 3.7        % por debajo de las dos → dos retornos, estado ligado

x0 = -0.35      % retorno de Eb
x1 = 0          % retornos de Ec
x2 = 1

% Despeje del lado izquierdo (dos ecuaciones, dos incógnitas)
AL = ln((Vl-Eb)/(Vl-Vm))
BL = ln((Vl-Ec)/(Vl-Vm))
pl = ln(AL/BL) / ln((xm-x0)/(xm-x1))
wl = (xm-x0) / ((0-AL)^(1/pl))
% Lado derecho: el ancho sale de V(x2)=Ec
wr = (x2-xm) / ((0-ln((Vr-Ec)/(Vr-Vm)))^(1/qr))

% ── encuadre ─────────────────────────────────────────────────────────────────
% La ventana tiene aspecto 3.9:9.5 y el lienzo es cuadrado. V3 es isométrico
%, así que sin corregir letterboxearía a un buzón angosto. Se ensancha la
% ventana en x por el mismo factor y se dibuja bajo un `scale` anisótropo. Es
% seguro para los rótulos: bajo un transform V3 mueve el ANCLA del texto, no los
% glifos (verificado: el `scalefont` del EPS sale idéntico).
xmin = -1.3   xmax = 2.3
kx = 2.4358974                       % = 9.5/3.9
world_window -3.1666667 6.3333333 -4 5.5
scale kx 1

% ── constante de fase ────────────────────────────────────────────────────────
% Es el ÚNICO parámetro visual libre, y equivale a elegir la masa: fija cuántos
% lóbulos caben. Se despeja de la cuantización de Bohr-Sommerfeld del estado
% ligado —∫k dx = (n+½)π entre sus retornos—, así que las otras dos ondas NO
% tienen libertad: comparten partícula, luego comparten constante.
nodos = 3                            % nodos interiores de ψ_c → nodos+1 antinodos
NQ = 400
hq = (x2-x1)/NQ
Sq = 0
for i = 1 to NQ {
    xq = x1 + (i-0.5)*hq
    Vq = Vr - (Vr-Vm)*exp(0 - (((xq-xm)/wr)^qr))
    if xq < xm { Vq = Vl - (Vl-Vm)*exp(0 - (((xm-xq)/wl)^pl)) }
    dq = Ec - Vq
    if dq > 0 { Sq = Sq + sqrt(dq)*hq }
}
C = (nodos+0.5)*pi/Sq

% ── la curva del potencial ───────────────────────────────────────────────────
% Se acumula como segmentos rectos con `path +=`: cada pieza es RELATIVA,
% y el empalme de concat (inicio de la siguiente = final de la anterior) las
% encadena en la curva absoluta. Es la vía para dibujar una función cuando el
% nº de puntos depende de una variable — un bloque de coordenadas no lleva `for`.
NV = 300
hv = (xmax-xmin)/NV
Vprev = Vl - (Vl-Vm)*exp(0 - (((xm-xmin)/wl)^pl))
Vini = Vprev
path curvaV = { 0 0 }
for i = 1 to NV {
    xv = xmin + i*hv
    Vv = Vr - (Vr-Vm)*exp(0 - (((xv-xm)/wr)^qr))
    if xv < xm { Vv = Vl - (Vl-Vm)*exp(0 - (((xm-xv)/wl)^pl)) }
    path curvaV += { 0 0  (hv) (Vv-Vprev) }
    Vprev = Vv
}

% ── mobiliario: niveles, asíntotas y verticales de los retornos ──────────────
line_width 0
polyline { (xmin) (Ea)  (xmax) (Ea) }
polyline { (xmin) (Vl)  (xmax) (Vl) }
polyline { (x0-0.05) (Eb)  (xmax) (Eb) }
polyline { (x1-0.05) (Ec)  (xmax) (Ec) }
polyline { (xmin) 3.2  (xmax) 3.2 }
% bases de las tres funciones de onda
ba = 2        bb = -0.5     bc = -3
polyline { (xmin) (ba)  (xmax) (ba) }
polyline { (xmin) (bb)  (xmax) (bb) }
polyline { (xmin) (bc)  (xmax) (bc) }
% verticales: cada retorno hasta su propio nivel
polyline { (x0) (bb)  (x0) (Eb) }
polyline { (x1) -3.5  (x1) (Ec) }
polyline { (x2) -3.5  (x2) (Ec) }

line_width 0.8
tvx = xmin   tvy = Vini
{ translate tvx tvy   polyline(&curvaV) }

% ── las tres funciones de onda ───────────────────────────────────────────────
% Un solo lazo para las tres (listas), así la construcción WKB aparece UNA
% vez. Cada onda marcha en x acumulando la fase ∫k dx; cada vez que la fase
% avanza π hay un extremo, y entre extremos consecutivos se emite un medio ciclo
% de `sine` con `path +=` — pendiente cero en la unión, luego empalme liso.
% La amplitud de cada extremo es la semiclásica |ψ| ∝ (E−V)^(−¼): grande donde
% la partícula va lenta (cerca de los retornos), pequeña en el fondo del pozo.
EE   = [ Ea, Eb, Ec ]
XA   = [ -1.3, x0, x1 ]              % inicio del tramo oscilante
XB   = [ 2.3, 2.3, x2 ]              % fin
BASE = [ ba, bb, bc ]
% Escala visual de amplitud, por onda. Es lo único que se elige mirando: fija
% que cada onda quepa en su banda sin tocar la vecina. La FORMA de la envolvente
% —dónde crece y cuánto— la manda (E−V)^(−¼), no estos números.
ESC  = [ 0.533, 0.486, 0.590 ]
TL   = [ 0, 1, 1 ]                   % ¿lleva cola exponencial a la izquierda?
TR   = [ 0, 0, 1 ]                   % ¿y a la derecha?
XT0  = [ 0, x0, x1 ]                 % retorno izquierdo (donde nace la cola)
XT1  = [ 0, 0, x2 ]                  % retorno derecho
gx0 = 0  gy0 = 0  gx1 = 0  gy1 = 0

line_width 0.6
for onda = 0 to 2 {
    E = EE[onda]   xa = XA[onda]   xb = XB[onda]
    yb = BASE[onda]   S = ESC[onda]
    N = 1400
    h = (xb-xa)/N
    fase = 0
    kk = 0                           % extremos ya detectados
    xpre = xa
    epre = 0
    path w = { 0 0 }
    for i = 1 to N {
        x = xa + i*h
        xc = x - h/2
        Vc = Vr - (Vr-Vm)*exp(0 - (((xc-xm)/wr)^qr))
        if xc < xm { Vc = Vl - (Vl-Vm)*exp(0 - (((xm-xc)/wl)^pl)) }
        dd = E - Vc
        if dd > 0 { fase = fase + C*sqrt(dd)*h }
        % Los extremos de ψ ~ cos(φ−π/4)/√k caen cada π de fase
        if fase > pi/4 + kk*pi {
            Vx = Vr - (Vr-Vm)*exp(0 - (((x-xm)/wr)^qr))
            if x < xm { Vx = Vl - (Vl-Vm)*exp(0 - (((xm-x)/wl)^pl)) }
            mag = S*((E-Vx)^(0-0.25))
            ecur = mag
            if mod(kk,2) == 1 { ecur = 0 - mag }
            if kk > 0 {
                amp = (ecur-epre)/2
                ph = 270
                if amp < 0 { amp = 0 - amp   ph = 90 }
                path w += sine(half_cycles=1, phase=ph, amplitude=amp) { 0 0  (x-xpre) 0 }
            }
            if kk == 0 { gx0 = x   gy0 = ecur }
            gx1 = x   gy1 = ecur
            xpre = x   epre = ecur   kk = kk+1
        }
    }
    twx = gx0   twy = yb + gy0
    { translate twx twy   bezier(&w) }

    % ── colas en la región prohibida (E < V) ─────────────────────────────────
    % ψ no se anula en el retorno: decae hacia afuera. La longitud de decaimiento
    % NO es 1/√(V−E) —que DIVERGE justo en el retorno, donde V−E=0 y WKB se
    % rompe—: es la escala de Airy, d ∝ |V'(retorno)|^(−⅓), raíz CÚBICA de la
    % pendiente. Por eso la cola es más larga contra la pared tendida que contra
    % la empinada, sin que nadie lo ponga a mano. V' por diferencia centrada.
    ee = 0.004
    for lado = 0 to 1 {
        hay = TL[onda]   xr = XT0[onda]   xg = gx0   yg = gy0   dir = 0-1
        if lado == 1 { hay = TR[onda]   xr = XT1[onda]   xg = gx1   yg = gy1   dir = 1 }
        if hay == 1 {
            xp = xr + ee
            xn = xr - ee
            Vp = Vr - (Vr-Vm)*exp(0 - (((xp-xm)/wr)^qr))
            if xp < xm { Vp = Vl - (Vl-Vm)*exp(0 - (((xm-xp)/wl)^pl)) }
            Vn = Vr - (Vr-Vm)*exp(0 - (((xn-xm)/wr)^qr))
            if xn < xm { Vn = Vl - (Vl-Vm)*exp(0 - (((xm-xn)/wl)^pl)) }
            pend = abs((Vp-Vn)/(2*ee))
            dd = KAIRY*(pend^(0-1/3))
            % 7 muestras de yb + yg·exp(−|x−xg|/dd), del extremo hacia afuera
            L = 3.2*dd
            u1 = xg + dir*L/6      u2 = xg + dir*2*L/6    u3 = xg + dir*3*L/6
            u4 = xg + dir*4*L/6    u5 = xg + dir*5*L/6    u6 = xg + dir*L
            smooth {
                (u6) (yb + yg*exp(0-L/dd))
                (u5) (yb + yg*exp(0-5*L/(6*dd)))
                (u4) (yb + yg*exp(0-4*L/(6*dd)))
                (u3) (yb + yg*exp(0-3*L/(6*dd)))
                (u2) (yb + yg*exp(0-2*L/(6*dd)))
                (u1) (yb + yg*exp(0-L/(6*dd)))
                (xg) (yb + yg)
            }
        }
    }
}

% ── rótulos ──────────────────────────────────────────────────────────────────
% Anclados a los parámetros, no a coordenadas sueltas: si se mueve una energía,
% su rótulo la sigue.
font "Times-Roman"
xlab = xmax + 0.1
text("$E_a$") { (xlab) (Ea-0.1) }
text("$V_-$") { (xlab) (Vl-0.1) }
text("$E_b$") { (xlab) (Eb-0.1) }
text("$V_+$") { (xlab) (Vr-0.3) }
text("$E_c$") { (xlab) (Ec-0.1) }
% Las ψ van al margen, a la altura de su base: con la envolvente WKB las ondas
% llenan su banda de lado a lado y ya no queda hueco interior donde ponerlas.
text("$\psi_a$") { (xlab) (ba-0.1) }
text("$\psi_b$") { (xlab) (bb-0.1) }
text("$\psi_c$") { (xlab) (bc-0.1) }
text("$V(x)$") { -0.09 4 }
ylab = 2.96
text("$x_0$") { (x0+0.06) (ylab) }
text("$x_1$") { (x1+0.07) (ylab) }
text("$x_2$") { (x2+0.05) (ylab) }
