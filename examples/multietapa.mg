% Observación multietapa: la misma escena vista desde cuatro alturas
%
% Un mismo punto del terreno observado desde el satélite, desde una aeronave a
% gran altitud, desde otra a baja altitud y desde el suelo. Al bajar de nivel,
% cada fuente cubre MENOS superficie con MÁS detalle, y lo que se extrae en un
% nivel se extrapola al de arriba. La columna de plataformas no está colocada a
% ojo: cae sobre el centro del terreno porque se evalúa la misma fórmula
% bilineal que dibuja el paralelogramo, y los niveles se reparten por una
% fórmula, no uno por uno. Las dos aeronaves y las figuras humanas salen de
% `lib/` con `include`.
%
% NOTAS ---------------------------------------------------------------------
% ORIGEN: reconstruccion de la Fig. 1.25 de Lillesand, Kiefer & Chipman,
% «Remote Sensing and Image Interpretation» (7a ed., Wiley 2015), p. 65 del
% PDF, «Multistage remote sensing concept».
%
% POR QUE SE REHIZO Y NO SE RECORTO: el original es un escaneo de un dibujo de
% los anos setenta que Wiley arrastra sin redibujar. Medido sobre el PDF a 200
% dpi, el trazo esta reventado y los rotulos son ilegibles —«Satellite data»
% sale como «Sa: elliec dety», «High altitude data» como «I fijl. elrlt.,de»—.
% No hay --dpi que lo arregle: asi esta en el original. Proyectado en un salon
% no se lee, y era la unica figura del acervo para este concepto.
%
% LA COLUMNA DE PLATAFORMAS CAE SOBRE EL CENTRO DEL TERRENO, y no a un lado:
% `eje_x` se EVALUA con la misma formula bilineal que dibuja el paralelogramo,
% P(u,s) = TL + u*(TR-TL) + s*(BL-TL) en u=s=0.5, de modo que mover un vertice
% del terreno arrastra la columna entera. Los cuatro niveles miran el mismo
% punto: esa es la afirmacion de la figura, y si la columna se despegara del
% centro la figura diria que miran sitios distintos.
%
% LOS NIVELES ESTAN EQUIESPACIADOS por construccion (`y_nivel = y_suelo +
% (k+1)*salto`), no colocados a ojo. El original los reparte de forma
% irregular; aqui la escalera es pareja porque lo unico que la figura afirma
% del eje vertical es el ORDEN, no las altitudes —que van de 700 km a 500 m y
% no caben en ninguna escala comun—. La cota lateral dice lo que si se afirma:
% al bajar, menos area y mas detalle.
%
% LAS DOS AERONAVES SON DE CLASE DISTINTA, y esa es media figura: reactor de
% ala en flecha arriba, avioneta de ala alta y helice abajo. Antes eran la MISMA
% silueta al mismo tamano y solo los rotulos separaban «gran altitud» de «baja
% altitud» —o sea que la figura afirmaba dos plataformas y dibujaba una—. Las
% dos salen de `lib/aircraft.mg` (`Jet` y `LightPlane`), en planta las dos: un
% cambio de punto de vista entre dos peldanos de la misma columna se lee como
% error, no como variedad.
%
% EL TERRENO ES UN PARALELOGRAMO CON REJILLA, perspectiva de caballero barata:
% dos familias de polilineas generadas por la misma formula bilineal, sin motor
% 3-D de por medio. Se prefirio a la malla de puntos del original, que a tamano
% de proyeccion se convierte en ruido.
%
% COBERTURA EXCLUSIVA: es el UNICO usuario de `lib/aircraft.mg` y de
% `lib/people.mg`. Si sale del corpus, esas dos bibliotecas dejan de compilarse
% en cada `check` y nada vuelve a mirarlas.

display_size 12 10.73
world_window -0.6 14.6 -0.3 13.3

include "../lib/satellite.mg"
include "../lib/aircraft.mg"
include "../lib/people.mg"

% ===========================================================================
%  El terreno, y el eje que sale de su centro
% ===========================================================================

TLx = 1.30   TLy = 4.40
TRx = 9.10   TRy = 4.40
BLx = -0.30  BLy = 1.35
BRx = TRx + BLx - TLx
BRy = TRy + BLy - TLy

% El centro del paralelogramo: P(0.5, 0.5) de la formula bilineal.
eje_x = TLx + 0.5*(TRx - TLx) + 0.5*(BLx - TLx)
eje_y = TLy + 0.5*(TRy - TLy) + 0.5*(BLy - TLy)

nu = 8   ns = 5
for i = 0 to nu {
    u = i/nu
    polyline(line_width=0.35, color=gray(0.45)) {
        (TLx + u*(TRx-TLx))                 (TLy + u*(TRy-TLy))
        (TLx + u*(TRx-TLx) + (BLx-TLx))     (TLy + u*(TRy-TLy) + (BLy-TLy)) } }
for j = 0 to ns {
    s = j/ns
    polyline(line_width=0.35, color=gray(0.45)) {
        (TLx + s*(BLx-TLx))                 (TLy + s*(BLy-TLy))
        (TRx + s*(BLx-TLx))                 (TRy + s*(BLy-TLy)) } }
polyline(closed=true, line_width=0.8) { TLx TLy  TRx TRy  BRx BRy  BLx BLy }

% ===========================================================================
%  La escalera de plataformas
% ===========================================================================

salto  = 2.90
y_baja = eje_y + 1*salto
y_alta = eje_y + 2*salto
y_sat  = eje_y + 3*salto

% El eje: la linea de vista comun a los cuatro niveles.
polyline(dash="dashdot", line_width=0.6) { eje_x eje_y  eje_x (y_sat - 0.85) }

Satellite(scale=1.15, rotate=180, at=(eje_x, y_sat))
Jet(scale=0.95, at=(eje_x, y_alta))
LightPlane(scale=0.80, at=(eje_x, y_baja))

% El apoyo de campo, sobre el terreno y junto al eje.
Person(scale=1.0, at=((eje_x + 1.05), (eje_y - 0.30)))
Person(scale=0.85, at=((eje_x + 1.75), (eje_y - 0.45)))

% ===========================================================================
%  Rotulos
% ===========================================================================

rot_x = eje_x + 1.55

text("Datos de satélite", size=10) { rot_x y_sat }
text("Datos aéreos/nde gran altitud", size=10) { rot_x (y_alta + 0.25) }
text("Datos aéreos/nde baja altitud", size=10) { rot_x (y_baja + 0.25) }
text("Observaciones de campo", size=10) { (TRx - 1.9) (BRy - 0.85) }

% ===========================================================================
%  La cota lateral: lo unico que la figura afirma del eje vertical
% ===========================================================================

cot_x = 13.30
polyline(marker_end="arrow", line_width=0.8, marker_size=3.5) {
    cot_x (y_sat - 0.55)   cot_x (eje_y + 0.85) }
text("más área,/nmenos detalle", align="center", size=8.5, color=gray(0.40)) {
    cot_x (y_sat + 0.95) }
text("menos área,/nmás detalle", align="center", size=8.5, color=gray(0.40)) {
    cot_x (eje_y + 0.10) }

% Sin pie: el titulo lo pone la lamina. Un pie aqui competia con el rotulo
% «Observaciones de campo», que va justo encima y tambien se lee como pie.
