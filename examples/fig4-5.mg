% ───────────────────────────────────────────────────────────────────────────────
% Fig. 3.1 de IMQ, 3a. ed. (sept. 1998, dic. 2002)
% Potenciales y estructura del espectro: tres pozos con sus puntos de retorno.
%
% Port V3 de examples/fig4-5v1.mg. Dos cambios de fondo frente a una traducción
% literal (que solo cambiaría la sintaxis):
%
%  1. Las tres curvas son ANALÍTICAS, no datos. El V1 las traía digitalizadas a 69
%     puntos sobre la caja unitaria de una struct; las fórmulas se recuperaron de
%     esos mismos puntos (ajustan a ~1e-6) y aquí se calculan. Cero coordenada
%     digitalizada.
%  2. Cada panel es un plot{} en unidades FÍSICAS, con su origen real: la caja
%     unitaria de V1 había corrido el origen de b) y c) a un r arbitrario distinto
%     de cero. Los niveles de energía y los puntos de retorno se DERIVAN de E, así
%     que mover una E reacomoda sola su línea, sus marcas y sus rótulos.
%
% Ejercita: varios plot{} en un documento (rejilla de paneles) y `base=` en los
% ejes — el eje V centrado de a), el eje x sobre V=0 de c).
%
% Nota: el V1 traía `DOT 5 011 .3` (punto decimal perdido) en el panel c, así que
% la marca sobre x'_1 se iba a x=454, fuera de página, y falta en el original
% impreso. Aquí sale en su sitio.
% ───────────────────────────────────────────────────────────────────────────────

display_size 12 4.5
font_size 8
world_window 0 120 2 43

n = 60                                     % segmentos por curva

% ── a) Oscilador armónico: V = x² ──────────────────────────────────────────────
% El eje V va CENTRADO (base=0) porque ahí está el origen físico del oscilador.
% E fija los puntos de retorno clásicos en ±√E.
Ea = 0.4
a1 = -sqrt(Ea)
a2 = sqrt(Ea)

plot(x=(-0.95, 0.95), y=(-0.1, 0.85), box=(5.5, 10, 25.5, 37)) {
    line_width 0.6
    for i = 0 to n-1 {                     % la curva sube hasta el techo: V=0.85 → x=±0.922
        p = -0.92 + i*(1.84/n)
        q = -0.92 + (i+1)*(1.84/n)
        polyline { p  p*p   q  q*q }
    }
    line_width 0
    polyline { a1 Ea  a2 Ea }              % nivel de energía
    dash "dashed"
    polyline { a1 Ea  a1 -0.1 }            % retornos, hasta el eje x
    polyline { a2 Ea  a2 -0.1 }
    dash "solid"
    dot(2.5) { a1 Ea  a2 Ea }

    text("/iE") { 0.81 (Ea-0.05) }         % resta en coord → paréntesis (§4)
    text("$x_1$", align="center") { a1 -0.18 }
    text("$x_2$", align="center") { a2 -0.18 }

    xaxis(ticks="none", labels=false, line_width=0.2)
    yaxis(base=0, ticks="none", labels=false, line_width=0.2)
}

% ── b) Barrera coulombiana repulsiva: V = 1/r ─────────────────────────────────
% Ambos ejes caen en el borde de la caja → sin base= (el caso por default).
% La curva se muestrea uniforme en V (= 1/r), no en r: así los puntos se juntan en
% la rodilla, donde 1/r es casi vertical, y se ralean en la cola plana. 60 bastan.
Eb = 0.4
b1 = 1/Eb                                  % retorno: V(r)=E → r=1/E
btop = 0.95                                % techo del panel (donde arranca la curva)
bend = 1/36                                % cola: r=36

plot(x=(0.8, 36), y=(0, btop), box=(39.8, 10, 68, 38.5)) {
    line_width 0.6
    for i = 0 to n-1 {
        u = btop - i*((btop-bend)/n)
        v = btop - (i+1)*((btop-bend)/n)
        polyline { 1/u  u   1/v  v }       % sin paréntesis: pondrían `u (1/v)` = llamada
    }
    line_width 0
    polyline { b1 Eb  36 Eb }              % nivel de energía
    dash "dashed"
    polyline { b1 Eb  b1 0 }               % retorno, hasta el eje x
    dash "solid"
    dot(2.5) { b1 Eb }

    text("/iE") { 37.3 Eb }
    text("$x_1$", align="center") { b1 -0.075 }

    xaxis(ticks="none", labels=false, line_width=0.2)
    yaxis(ticks="none", labels=false, line_width=0.2)
}

% ── c) Potencial efectivo: V = 1/(2r²) − 1/r ──────────────────────────────────
% Centrífugo + Coulomb atractivo. Mínimo en r=1, V=−1/2. El eje x va sobre V=0
% (base=0), INTERIOR al rango −0.5…0.45: es lo que `base=` hace posible.
% Muestreo uniforme en s = 1/r, donde V = s²/2 − s es una PARÁBOLA — 60 segmentos
% dan una curva suave incluso en la pared repulsiva.
Ec1 =  0.3                                 % E > 0: un retorno (dispersión)
Ec2 = -0.3                                 % E < 0: dos retornos (estado ligado)
c0 = 0.4415                                % V(r) = Ec1
c1 = 0.6125                                % V(r) = Ec2, interno
c2 = 2.7208                                % V(r) = Ec2, externo
ctop = 1/0.42                              % pared: la curva arranca en V=0.45
cend = 1/6.3                               % cola

plot(x=(0.37, 6.3), y=(-0.5, 0.45), box=(80, 10, 108.6, 38.5)) {
    line_width 0.6
    for i = 0 to n-1 {
        u = ctop - i*((ctop-cend)/n)
        v = ctop - (i+1)*((ctop-cend)/n)
        % La resta exige paréntesis (§4), y por eso el 1/u de al lado también los
        % lleva: `1/u (…)` leería `u` como llamada a función.
        polyline { (1/u)  (u*u/2-u)   (1/v)  (v*v/2-v) }
    }
    line_width 0
    polyline { c0 Ec1  6.3 Ec1 }           % niveles de energía
    polyline { c1 Ec2  c2 Ec2 }
    dash "dashed"
    polyline { c0 Ec1  c0 0 }              % retornos, hasta el eje x
    polyline { c1 Ec2  c1 0 }
    polyline { c2 Ec2  c2 0 }
    dash "solid"
    dot(2.5) { c0 Ec1  c1 Ec2  c2 Ec2 }

    text("/iE") { 3.46 (Ec1+0.03) }
    text(">0")  { 3.85 (Ec1+0.03) }
    text("/iE") { 3.46 (Ec2-0.07) }
    text("<0")  { 3.85 (Ec2-0.07) }
    % Zona apretada: los rótulos van a la derecha de su línea, como en el libro.
    % x'_1 se corre además lo justo para librar su marca (que en el original falta).
    text("$x'_1$") { (c0+0.35) 0.2 }
    text("$x_1$")  { c1 0.033 }
    text("$x_2$", align="center") { c2 0.033 }

    xaxis(base=0, ticks="none", labels=false, line_width=0.2)
    yaxis(ticks="none", labels=false, line_width=0.2)
}

% ── Mobiliario de página: coords de la ventana ────────────────────────────────
% Los rótulos de ordenada van a una misma altura en los tres paneles (decisión de
% página, no de datos) y al extremo del eje, como en el libro; los de abscisa, al
% final de su eje. Van aquí, y no como axis(title=), porque `title=` los centra a
% lo largo del eje — ver `title_at=` en los pendientes de plan_plot.md.
line_width 0.2
font "roman"
text("/iV(x)") { 1 35 }
text("/iV(x)") { 34.5 35 }
text("/iV(x)") { 74.5 35 }
text("/ix") { 26 9.3 }
text("/ix") { 69 9.3 }
text("/ix") { 109 24.3 }

dot(2.5) { 5 40.5 }
text("classical turning point") { 7 40 }
text("a)") { 13 3 }
text("b)") { 51 3 }
text("c)") { 94 3 }
