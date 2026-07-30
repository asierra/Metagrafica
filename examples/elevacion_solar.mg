% Ángulos de elevación solar — la geometría de la iluminación en percepción remota
%
% Corte por el meridiano del observador al mediodía solar: el globo, el plano tangente
% (el horizonte local), la vertical del lugar con el satélite en el cenit y los tres
% rayos del solsticio de junio, los equinoccios y el solsticio de diciembre. Los tres
% ángulos de elevación no están puestos a ojo: salen de la latitud y de la oblicuidad de
% la eclíptica —los dos números declarados al principio— y los rótulos imprimen el mismo
% valor que gobernó el trazo. Cambia `lat` y se recalcula la figura entera.
%
% NOTAS ---------------------------------------------------------------------
% ORIGEN: reconstrucción de la Fig. 7.4 de Lillesand, Kiefer & Chipman, «Remote Sensing
% and Image Interpretation». No es una copia. La lámina original es ESQUEMÁTICA: medidos
% sobre el escaneo, sus tres rayos se separan unos 10° entre sí, cuando la diferencia
% entre un solsticio y el equinoccio es la oblicuidad, 23.44°; y su punto de tangencia
% cae a ~24° de latitud, donde el Sol de verano pasa AL NORTE del cenit. Aquí la
% construcción es física, así que el dibujo no puede desmentir a los rótulos.
%
% POR QUÉ EL RAYO DEL EQUINOCCIO SALE HORIZONTAL: el Sol está infinitamente lejos, de
% modo que su rayo forma con el plano ecuatorial exactamente la declinación δ, sea cual
% sea la latitud. En este corte δ es el ángulo del rayo con la horizontal (+23.44°, 0,
% −23.44°) y la elevación sobre el horizonte local es h = 90° − lat + δ. Los 47° que
% separan al Sol de verano del de invierno son dos veces la oblicuidad, y de ahí sale
% todo el ciclo estacional de la iluminación.
%
% EL LIMBO ES UN MERIDIANO: `lib/fulldisk_map.mg` es una vista ortográfica desde el
% ecuador, así que el borde del disco es el meridiano a 90° de la vista, visto de canto,
% y un punto del limbo a ángulo φ está EXACTAMENTE a latitud φ. Por eso el punto de
% observación se posa sobre el limbo sin corrección de ninguna clase, y por eso el
% ecuador del mapa cae sobre el plano ecuatorial dibujado.
%
% EL SOL ES UN ICONO DE BIBLIOTECA (`lib/sun.mg`), no un disco con rótulo: se lee sin
% que haya que escribirle «Sol» encima. Cuesta margen a la derecha —el anillo de rayos
% llega a 1.9 radios— y por eso la columna de soles se corrió de x=14.5 a 13.9: con el
% icono en 14.5, «Primavera/Otoño» se salía del lienzo. Los soles resbalan por su propio
% rayo, así que mover la columna NO cambia ningún ángulo; las tres elevaciones siguen
% imprimiendo 68.4/45.0/21.6. Todo lo que se separa del sol (el corte del rayo entrante,
% el margen de los rótulos) se mide desde `sun_out`, el radio libre de rayos.
%
% LATITUDES TROPICALES: con lat < 23.44 el rayo de verano queda por encima del cenit
% (h > 90°). La figura sigue siendo correcta, pero se lee peor, porque los arcos se
% montan sobre la línea del cenit. 45° es la latitud que mejor separa los tres rayos.

display_size 20 14
world_window -7 20  -7 12

include "../lib/fulldisk_map.mg"
include "../lib/sun.mg"
include "../lib/satellite.mg"

% --- los dos números de los que sale todo ---
lat = 45            % latitud del punto de observación, grados N
obl = 23.44         % oblicuidad de la eclíptica = declinación en los solsticios

R     = 5           % radio del globo
sun_x = 13.9        % los tres soles se alinean en esta columna
sun_r = 0.8         % radio del DISCO del sol; sus rayos llegan a 1.9 radios
sun_out = 2.15*sun_r  % de aquí para afuera ya no hay rayos: el radio libre

d2r = pi/180
Px  = R*cos(lat*d2r)          % punto de observación, sobre el limbo
Py  = R*sin(lat*d2r)
hor = lat - 90                % horizonte local del lado del Sol (el opuesto, hor+180)

% Plano ecuatorial: la referencia desde la que se mide la declinación. Va primero para
% que el globo lo tape por dentro y solo asome fuera del disco.
polyline(dash="dashed", line_width=0.4, color="gray") { -6.6 0  11.6 0 }
text("plano ecuatorial", size=8, color="gray") { 8.3 -0.62 }

FullDiskMap(scale=R, at=(0, 0))
text("Ecuador", size=8, color="white") { -4.3 0.3 }

% Plano tangente: la perpendicular al radio en el punto de observación, o sea el
% horizonte del lugar. Es la línea desde la que se mide la elevación.
polyline(line_width=0.6) { (Px + 6.5*cos((hor+180)*d2r)) (Py + 6.5*sin((hor+180)*d2r))
                           (Px + 11*cos(hor*d2r))        (Py + 11*sin(hor*d2r)) }
% El rótulo va sobre la línea: se lleva el origen al punto del rótulo, apartado 0.22 en
% la dirección del cenit, y se gira el plano lo mismo que la línea.
rot_x = Px + 5.7*cos((hor+180)*d2r) + 0.22*cos(lat*d2r)
rot_y = Py + 5.7*sin((hor+180)*d2r) + 0.22*sin(lat*d2r)
{ translate rot_x rot_y   rotate hor
  text("Plano tangente", align="center", size=9) { 0 0 } }

% Cenit: la vertical del lugar prolongada. El satélite mira al nadir, así que va sobre
% ella; el rayo de verano se le acerca tanto como lo permite la latitud.
polyline(line_width=0.6) { (Px) (Py)  (Px + 7.7*cos(lat*d2r)) (Py + 7.7*sin(lat*d2r)) }
cen_x = Px + 4.6*cos(lat*d2r) + 0.38*cos(hor*d2r)
cen_y = Py + 4.6*sin(lat*d2r) + 0.38*sin(hor*d2r)
{ translate cen_x cen_y   rotate lat
  text("Cenit", align="center", size=9) { 0 0 } }

sat_x = Px + 8.8*cos(lat*d2r)
sat_y = Py + 8.8*sin(lat*d2r)
% El icono apunta su antena a +y, así que para que mire a la Tierra hay que llevarla a
% la dirección lat+180 (el nadir visto desde allá arriba): el giro es lat+90, y sale del
% mismo número que puso al satélite ahí. A 45° da los 135° que se ven a ojo.
Satellite(scale=1.6, rotate=(lat + 90), at=(sat_x, sat_y))
text("Satélite", size=10) { (sat_x + 1.9) (sat_y - 0.15) }

% Los tres soles. Cada uno se coloca por su declinación —no por una altura medida— y el
% arco va del horizonte al rayo, que es justo lo que significa el ángulo de elevación.
decs   = [obl, 0, -obl]
epoca  = ["Verano", "Primavera/Otoño", "Invierno"]
radio  = [6, 4.8, 3.6]

for k = 0 to 2 {
    d  = decs[k]
    sy = Py + (sun_x - Px)*tan(d*d2r)      % el sol de esta época, en su columna
    h  = 90 - lat + d                       % elevación sobre el horizonte local

    % El rayo se corta en el radio LIBRE, no en el borde del disco: si llegara al
    % disco atravesaría el anillo de rayos del icono.
    polyline(color="darkorange", line_width=0.6) {
        (Px) (Py)  (sun_x - sun_out*cos(d*d2r)) (sy - sun_out*sin(d*d2r)) }

    % El icono dice «sol» solo, así que no lleva rótulo dentro.
    Sun(scale=sun_r, at=(sun_x, sy))

    text("/b" + epoca[k], size=10) { (sun_x + sun_out + 0.35) (sy + 0.28) }
    text("$h$ = " + str(h, 1) + "°", size=8.5, color="gray") { (sun_x + sun_out + 0.35) (sy - 0.62) }

    arc(radio[k], from=hor, to=d, color=gray(0.35), line_width=0.6,
        marker_start="arrow", marker_start_orient="reverse", marker_end="arrow", marker_size=3) { (Px) (Py) }
}

% El punto de observación: donde concurren el horizonte, el cenit y los tres rayos.
dot(2.2) { (Px) (Py) }

text("Ángulos de elevación solar ($h$)", align="center", size=10) { 12.2 -4.7 }
text("Observador a latitud $\varphi$ = " + str(lat) + "° N, al mediodía solar", size=9,
     color="gray") { -6.6 -6.4 }
