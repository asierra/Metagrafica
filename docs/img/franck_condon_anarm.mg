% VARIANTE PARA docs/calcular_en_vez_de_medir.md — no es del corpus.
%
% Es examples/franck_condon.mg con UN SOLO parámetro cambiado (la anarmonicidad xe1: 0.028 -> 0.045),
% para el par de renders que muestra cómo se reacomoda la figura entera. Vive aquí
% y no en examples/ porque no ejercita nada nuevo del lenguaje: existe para el
% documento. Pero SÍ está versionado y la compuerta `imgfail` de test/run.sh lo
% compila y compara, así que su render no puede quedarse rancio ni volverse
% irreproducible. Si el ejemplo original cambia, hay que rehacer esta variante.
%
% Principio de Franck-Condon, PARAMÉTRICO: dos potenciales de Morse con sus
% niveles vibracionales y funciones de onda. Aquí NADA está medido: se dan los
% parámetros de Morse (a, re, we, xe) de cada estado más Te, y el resto es forma
% cerrada.
%
%   V(r) = D(1 - exp(-a(r-re)))²                    la curva
%   E_v  = we(v+½) - we·xe(v+½)²                    los niveles
%   r±   = re - ln(1 ∓ sqrt(E/D))/a                 los puntos de retorno
%   vmax = 1/(2·xe) - ½                             cuántos niveles ligados hay
%
% Los puntos de retorno fijan el ancho de cada onda: la oscilación va entre r- y
% r+, y las colas asoman a la región prohibida con penetración asimétrica (efecto
% túnel; ver el lazo). Las ψ son CON SIGNO (cruzan la línea) con envolvente
% semiclásica WKB, armadas pieza a pieza con `path +=` (§9); los dos pozos
% comparten un solo lazo con sus parámetros en listas (§5.1).
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
xe1 = 0.045
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
rfin = 3.10                                % donde terminan las dos curvas

% Los números de nivel van a la DERECHA de cada onda. Su x sale de la forma
% cerrada —el retorno externo de la energía E+elab— para que la separación al
% trazo sea constante y no dependa de la pendiente de la pared. Acotado a
% [0.05, 0.20] del retorno (los extremos fallan solos: cerca del fondo cae dentro
% de la cola, cerca de la disociación la curva se aplana y el número se va lejos).
elab = 0.30

% ---- funciones de onda ----------------------------------------------------
% Cada ψ: v+1 lóbulos (v nodos) que CRUZAN la línea, entre dos colas planas (el
% decaimiento en la región prohibida). Las colas son piezas `sine(amplitude=0)`
% de ancho PARAMÉTRICO (línea recta), una por pared, para la penetración distinta.

% Una onda: base gris + relleno con contorno negro. El ancho de la ventana local
% es un parámetro con default calculado, `w = path_width(&onda)` (§8.2), así una
% sola struct con parámetros sirve para todos los niveles.
struct Nivel(&onda, w = path_width(&onda)) {
    world_window 0 w -2 2
    line_width 0.1   color "lightgray"   polyline { 0 0  w 0 }
    fill "orange"   outlinefill   color "black"   line_width 0.4
    bezier(&onda)
}

plot(x=(0.66, rfin), y=(-0.4, ytop), box=(1.5, 1.0, 8.6, 10.4)) {

    % ---- las dos curvas ---------------------------------------------------
    % Arrancan donde la pared repulsiva alcanza el techo de la figura: también
    % es un punto de retorno, el de energía ytop.
    line_width 0.7
    rini = re1 - ln(1 + sqrt((ytop-0.1)/D1))/a1
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
    polyline { 0.66 (D1)  (rfin) (D1) }
    polyline { 0.66 (Te+D2)  (rfin) (Te+D2) }
    dash "solid"

    % ---- las ondas vibracionales ------------------------------------------
    % UN solo lazo para los dos pozos: sus parámetros van en listas (§5.1)
    % indexadas por `pozo`, así la construcción de la onda aparece UNA vez en vez
    % de duplicada. El offset en energía (0 para el fundamental, Te para el
    % excitado) y la etiqueta del nivel 0 (v'' vs v') también salen de listas.
    aa  = [a1, a2]     rre = [re1, re2]   DD  = [D1, D2]
    wwe = [we1, we2]   xxe = [xe1, xe2]   TT  = [0, Te]
    lab0 = ["$v''=0$", "$v'=0$"]
    for pozo = 0 to 1 {
        a = aa[pozo]   re = rre[pozo]   D = DD[pozo]
        we = wwe[pozo]   xe = xxe[pozo]   toff = TT[pozo]
        for v = 0 to 6 {
            E  = we*(v+0.5) - we*xe*(v+0.5)*(v+0.5)
            Es = we*(v+1.5) - we*xe*(v+1.5)*(v+1.5)
            s  = sqrt(E/D)
            rm = re - ln(1+s)/a
            rp = re - ln(1-s)/a
            h  = 0.60*(Es-E)

            % --- penetración asimétrica en la región prohibida (efecto túnel) -------
            % ψ no se anula en el retorno: decae hacia afuera. La longitud de
            % penetración es la escala de Airy d ∝ |V'(retorno)|^(−⅓) (raíz CÚBICA de
            % la pendiente, NO 1/√(V−E), que diverge en el retorno). El COCIENTE entre
            % las dos paredes sale sin masa ni ħ: |V'(r±)| = 2a√(DE)(1∓s) → el factor
            % común se cancela y queda ((1+s)/(1−s))^⅓. Pared interna empinada = cola
            % corta; externa tendida = cola larga; la razón crece con v sola (s→1).
            % Las colas planas del path llevan ese ancho (media geométrica 0.5, para
            % conservar la tinta); el fit las mapea a voladizos gL/gR en r.
            ratio = exp(ln((1+s)/(1-s))/3)
            tL = 0.5/sqrt(ratio)                   % cola interna (pared empinada)
            tR = 0.5*sqrt(ratio)                   % cola externa (pared tendida)
            sx = (rp-rm)/(v+2)                      % escala local→r del tramo oscilante
            gL = tL*sx   gR = tR*sx                 % voladizos en unidades de r

            % --- la onda: v+1 lóbulos ψ (cruzan la línea) con envolvente WKB ------
            % Cada lóbulo es un medio ciclo entre extremos consecutivos (pendiente
            % cero en la unión → empalme liso); su altura es la amplitud semiclásica
            % |ψ| ∝ (E−V)^(−¼), máxima en los retornos. Al ser ψ CON SIGNO, el traslape
            % de Franck-Condon puede cancelarse — lo que |ψ|² no mostraría.
            % El `for` acumula los medios ciclos con `path +=` (§9), cada uno con SU
            % amplitud (un concat no cubre la longitud variable en v).
            eprev = 0                              % extremo previo (arranca en la cola)
            path w = sine(half_cycles=1, phase=0, amplitude=0) { 0 0  (tL) 0 }
            for k = 1 to v+1 {                     % un pico por lóbulo, en el JOIN
                rr  = rm + (rp-rm)*k/(v+2)          % posición del pico en r
                Vr  = D*(1-exp(-a*(rr-re)))*(1-exp(-a*(rr-re)))
                mag = exp(-0.25*ln(E-Vr))          % (E−V)^(−¼), envolvente WKB
                ek  = mag                          % signo alterno: par hacia abajo
                if mod(k,2) == 0 { ek = -mag }
                amp = (eprev-ek)/2                  % swing eprev→ek; phase 270 = +A→−A
                ph  = 270
                if amp < 0 { amp = -amp   ph = 90 }
                path w += sine(half_cycles=1, phase=ph, amplitude=amp) { 0 0  1 0 }
                eprev = ek
            }
            amp = eprev/2                          % último medio ciclo: de vuelta a 0
            ph  = 270
            if amp < 0 { amp = -amp   ph = 90 }
            path w += sine(half_cycles=1, phase=ph, amplitude=amp) { 0 0  1 0 }
            path w += sine(half_cycles=1, phase=0, amplitude=0) { 0 0  (tR) 0 }

            % El rect del fit: en x, los retornos ensanchados por los voladizos
            % ASIMÉTRICOS (gL interno, gR externo); en y, la energía ± una altura del
            % espaciamiento al nivel siguiente, que se cierra sola al apiñarse.
            fit(Nivel(&w), stretch=true) { (rm-gL) (toff+E-h)  (rp+gR) (toff+E+h) }

            etq = str(v)
            if v == 0 { etq = lab0[pozo] }         % §5.3: default afuera, if lo ajusta
            xl = re - ln(1 - sqrt((E+elab)/D))/a
            if xl < rp+gR+0.05 { xl = rp+gR+0.05 }
            if xl > rp+gR+0.20 { xl = rp+gR+0.20 }
            text(etq, font_size=7, align="left", valign="middle") { (xl) (toff+E) }
        }
    }

    % ---- transición de Franck-Condon --------------------------------------
    % Vertical desde v=0 del fundamental: la molécula no cambia de r durante la
    % transición electrónica. Llega donde la curva excitada corta esa vertical.
    %
    % Lo que la figura tiene que dejar ver es que la transición electrónica
    % CAMBIA el número cuántico vibracional, y eso aquí es un RESULTADO, no una
    % colocación: la vertical sube a 7.776, entre v'=6 (7.749) y v'=7 (7.934) y
    % casi encima del 6 — o sea v''=0 → v'≈6. Sale solo del desplazamiento de los
    % mínimos (re1=1.15 vs re2=1.48); mover re2 mueve el nivel de llegada sin
    % tocar nada más, que es justo la enseñanza. La marca en la punta (abajo) hace
    % legible el 0 → 6 sin cruzar la vista a los números de la derecha.
    E0 = we1*0.5 - we1*xe1*0.25
    Vfc = Te + D2*(1-exp(-a2*(re1-re2)))*(1-exp(-a2*(re1-re2)))
    line_width 0.5
    polyline(marker_end="arrow") { (re1) (E0)  (re1) (Vfc) }

    % Marca del nivel de llegada, junto a la punta (que cae en el retorno interno
    % de v'=6): hace legible "0 → v'≈6" sin cruzar la vista a los números de la
    % derecha. Es "≈6" porque Vfc=7.776 cae entre v'=6 (7.749) y v'=7 (7.934) — el
    % nivel más cercano, el que la transición puebla sobre todo.
    text("$v'\approx6$", font_size=7, align="right", valign="middle") { (re1-0.06) (Vfc) }

    xaxis(from=0.66, to=rfin, label="distancia internuclear  $r$", ticks="none",
    tick_labels=false, label_at="center")
    yaxis(from=-0.4, to=ytop, label="energía  $E$", ticks="none",
    tick_labels=false, label_at="center")
}
