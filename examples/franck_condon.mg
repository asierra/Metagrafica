% Principio de Franck-Condon, PARAMÉTRICO: dos potenciales de Morse con sus
% niveles vibracionales y funciones de onda. A diferencia de fig16-9.mg (port
% fiel de la figura publicada, con la curva digitalizada y los rectángulos de
% las ondas calibrados a mano), aquí NADA está medido: se dan los cinco
% parámetros de cada estado electrónico y el resto es forma cerrada.
%
%   V(r) = D(1 - exp(-a(r-re)))²                    la curva
%   E_v  = we(v+½) - we·xe(v+½)²                    los niveles
%   r±   = re - ln(1 ∓ sqrt(E/D))/a                 los puntos de retorno
%   vmax = 1/(2·xe) - ½                             cuántos niveles ligados hay
%
% Los puntos de retorno son lo que fija el ancho de cada onda: la parte
% oscilante va entre r- y r+, y las colas planas asoman a la región prohibida.
% Con f = 0.5/(v+1) el rect del fit aloja la onda completa (ancho v+2, colas de
% media unidad) dejando la oscilación exactamente entre los retornos.
%
% Necesita exp/ln (§5.2): un Morse no es escribible sin ellas.

display_size 9 11
font_size 9
world_window 0 9 0 11

% ---- estado fundamental ---------------------------------------------------
% we (frecuencia vibracional) y xe (anarmonicidad) son lo que se mide en el
% espectro; la profundidad del pozo NO es independiente de ellas: la relación
% de Morse D = we/(4·xe) la fija. Darlas por separado produce niveles por
% encima de la disociación (y un ln de argumento negativo, que aborta).
a1  = 1.8
re1 = 1.15
we1 = 0.56
xe1 = 0.028
D1  = we1/(4*xe1)

% ---- estado excitado (Te = origen electrónico) ----------------------------
Te  = 5.8
a2  = 1.9
re2 = 1.48
we2 = 0.40
xe2 = 0.0385
D2  = we2/(4*xe2)

ytop = 8.9
nseg = 90                                  % segmentos por curva

% ---- funciones de onda ----------------------------------------------------
% El modo v lleva v+2 medios ciclos alternos — que dan v+1 lóbulos, o sea los v
% nodos del estado— entre dos medias unidades planas, el decaimiento en la
% región clásicamente prohibida. Ancho total v+3; la parte oscilante, v+2.
path plana = { 0 0  .1667 0  .3333 0  .5 0 }

% Una onda: base gris tenue + relleno gris con contorno negro. El ancho de la
% ventana local sale del propio path (path_width), así que la misma struct
% sirve para los siete.
struct Nivel(&onda, w = path_width(&onda)) {
    world_window 0 w -2 2
    line_width 0.1   color "lightgray"   polyline { 0 0  w 0 }
    fill "lightgray"   outlinefill   color "black"   line_width 0.4
    bezier(&onda)
}

plot(x=(0.66, 2.95), y=(-0.4, ytop), box=(1.5, 1.0, 8.6, 10.4)) {

    % ---- las dos curvas ---------------------------------------------------
    % Arrancan donde la pared repulsiva alcanza el techo de la figura: también
    % es un punto de retorno, el de energía ytop.
    line_width 0.7
    rini = re1 - ln(1 + sqrt((ytop-0.1)/D1))/a1
    rfin = 2.95
    for i = 0 to nseg-1 {
        p = rini + i*((rfin-rini)/nseg)
        q = rini + (i+1)*((rfin-rini)/nseg)
        polyline { (p)  (D1*(1-exp(-a1*(p-re1)))*(1-exp(-a1*(p-re1))))
                   (q)  (D1*(1-exp(-a1*(q-re1)))*(1-exp(-a1*(q-re1)))) }
    }
    rini = re2 - ln(1 + sqrt((ytop-Te-0.1)/D2))/a2
    for i = 0 to nseg-1 {
        p = rini + i*((rfin-rini)/nseg)
        q = rini + (i+1)*((rfin-rini)/nseg)
        polyline { (p)  (Te + D2*(1-exp(-a2*(p-re2)))*(1-exp(-a2*(p-re2))))
                   (q)  (Te + D2*(1-exp(-a2*(q-re2)))*(1-exp(-a2*(q-re2)))) }
    }

    % ---- niveles no resueltos, hacia la disociación -----------------------
    % Se apiñan solos: E_v es cuadrática en v y su derivada se anula en vmax.
    line_width 0.15
    for v = 7 to 12 {
        E = we1*(v+0.5) - we1*xe1*(v+0.5)*(v+0.5)
        s = sqrt(E/D1)
        polyline { (re1 - ln(1+s)/a1)  (E)   (re1 - ln(1-s)/a1)  (E) }
    }
    for v = 7 to 8 {
        E = we2*(v+0.5) - we2*xe2*(v+0.5)*(v+0.5)
        s = sqrt(E/D2)
        polyline { (re2 - ln(1+s)/a2)  (Te+E)   (re2 - ln(1-s)/a2)  (Te+E) }
    }

    % ---- líneas de disociación --------------------------------------------
    line_width 0.15   dash "dashed"
    polyline { 0.66 (D1)  2.95 (D1) }
    polyline { 0.66 (Te+D2)  2.95 (Te+D2) }
    dash "solid"

    % ---- las ondas vibracionales ------------------------------------------
    % Un solo lazo para los dos pozos: la onda se construye INLINE con v (la
    % invocación de struct acepta una expresión de path), así que no hacen
    % falta siete paths declarados ni siete bloques repetidos.
    %
    % El rect del fit es lo único que hay que armar: en x, los puntos de
    % retorno ensanchados por f = 0.5/(v+2) — la fracción que aloja las colas
    % dejando la oscilación exactamente entre r- y r+—; en y, la energía del
    % nivel más una altura que sale del espaciamiento al siguiente nivel, y que
    % por tanto se cierra sola conforme los niveles se apiñan.
    for v = 0 to 6 {
        E  = we1*(v+0.5) - we1*xe1*(v+0.5)*(v+0.5)
        Es = we1*(v+1.5) - we1*xe1*(v+1.5)*(v+1.5)
        s  = sqrt(E/D1)
        rm = re1 - ln(1+s)/a1
        rp = re1 - ln(1-s)/a1
        g  = 0.5*(rp-rm)/(v+2)
        h  = 0.60*(Es-E)
        fit(Nivel(concat(&plana,
                         sine(half_cycles=v+2, phase=270, amplitude=1) { 0 0  (v+2) 0 },
                         &plana)), stretch=true) { (rm-g) (E-h)  (rp+g) (E+h) }
    }
    for v = 0 to 6 {
        E  = we2*(v+0.5) - we2*xe2*(v+0.5)*(v+0.5)
        Es = we2*(v+1.5) - we2*xe2*(v+1.5)*(v+1.5)
        s  = sqrt(E/D2)
        rm = re2 - ln(1+s)/a2
        rp = re2 - ln(1-s)/a2
        g  = 0.5*(rp-rm)/(v+2)
        h  = 0.60*(Es-E)
        fit(Nivel(concat(&plana,
                         sine(half_cycles=v+2, phase=270, amplitude=1) { 0 0  (v+2) 0 },
                         &plana)), stretch=true) { (rm-g) (Te+E-h)  (rp+g) (Te+E+h) }
    }

    % ---- transición de Franck-Condon --------------------------------------
    % Vertical desde v=0 del fundamental: la molécula no cambia de r durante la
    % transición electrónica. Llega donde la curva excitada corta esa vertical.
    E0 = we1*0.5 - we1*xe1*0.25
    Vfc = Te + D2*(1-exp(-a2*(re1-re2)))*(1-exp(-a2*(re1-re2)))
    line_width 0.5
    polyline(marker_end="arrow") { (re1) (E0)  (re1) (Vfc) }

    xaxis(from=0.66, to=2.95, label="distancia internuclear  $r$", ticks="none",
          tick_labels=false, label_at="center")
    yaxis(from=-0.4, to=ytop, label="energía  $E$", ticks="none",
          tick_labels=false, label_at="center")
}
