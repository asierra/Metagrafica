% =====================================================================
%  aircraft.mg — Aeronaves como icono, EN PLANTA y con la nariz a la derecha.
%
%  Las dos van dibujadas a tamaño canónico (largo ≈ 2 unidades de mundo,
%  centradas en el origen), así que `scale=` en la invocación es el factor
%  directo y `at=` las coloca sin sorpresas. Los grosores de línea NO escalan
%  con `scale=`: se quedan en puntos, que es lo que se quiere en un icono.
%
%  Ejemplo:  Jet(scale=0.95, at=(6, 10))
%            LightPlane(scale=0.95, at=(6, 7))
%            Jet(scale=0.8, rotate=90, at=(2, 2))   % subiendo
%
%  ⚠️ EN PLANTA Y NO DE PERFIL, a propósito y con la medición hecha: de perfil,
%    con un fuselaje lo bastante delgado para leerse a este tamaño, el ala y el
%    estabilizador quedan colgando de una línea y se ven como piezas sueltas.
%    La silueta simétrica se reconoce de un vistazo aunque mida centímetro y
%    medio, que es como aparece proyectada en un salón. Si alguna vez las
%    mezclas con un icono de perfil en la misma columna, el cambio de punto de
%    vista se lee como error, no como variedad.
% =====================================================================

% --- Jet: reactor de ala en flecha. Plataforma de GRAN ALTITUD ---------
%
% Ala en flecha y envergadura CORTA respecto al largo (span/largo ≈ 0.65).
struct Jet() {
    % fuselaje, la nariz en punta a la derecha
    polygon(fill="white", color="black", line_width=0.6) {
        1.10 0
        0.55 0.13    -0.95 0.13
        -1.10 0
        -0.95 -0.13  0.55 -0.13 }
    % alas, en flecha hacia atrás
    polygon(fill=gray(0.75), color="black", line_width=0.5) {
        0.30 0.10    -0.10 0.10
        -0.55 0.72   -0.20 0.72 }
    polygon(fill=gray(0.75), color="black", line_width=0.5) {
        0.30 -0.10   -0.10 -0.10
        -0.55 -0.72  -0.20 -0.72 }
    % estabilizadores de cola
    polygon(fill=gray(0.75), color="black", line_width=0.5) {
        -0.72 0.08   -0.95 0.08
        -1.10 0.36   -0.90 0.36 }
    polygon(fill=gray(0.75), color="black", line_width=0.5) {
        -0.72 -0.08  -0.95 -0.08
        -1.10 -0.36  -0.90 -0.36 }
}

% --- LightPlane: avioneta de ala alta y hélice. BAJA ALTITUD ----------
%
% Se distingue del `Jet` por la FORMA, que es lo que hay que pedirle a un par de
% iconos que van uno encima del otro en la misma columna: ala RECTA (no en
% flecha), fuselaje CORTO y hélice en la nariz. Largo y estrecho arriba, corto y
% ancho de alas abajo: se separan de un vistazo aunque midan centímetro y medio.
%
% ⚠️ LAS PROPORCIONES SON LAS DEL CESSNA 172, no las que salen a ojo: 8.28 m de
%   largo por 11.0 m de envergadura, o sea que el ala es MÁS ANCHA QUE LARGO EL
%   AVIÓN (1.33×). Aquí, con el largo normalizado a 1.70, la envergadura sale
%   2.26 y el resto se sigue: cuerda de raíz 1.63 m → 0.33, borde de ataque al
%   30% del largo desde la nariz → x=0.24, estabilizador de 3.4 m → ±0.35.
%   El primer intento le puso envergadura de jet (más corta que el fuselaje) y
%   el resultado no se leía como avioneta sino como reactor mal dibujado. La
%   proporción ES el icono: es lo único que un lector que no sabe de aviones
%   distingue sin pensarlo.
%
% El ala va de una pieza CRUZANDO el fuselaje, que es como se ve un ala alta
% desde arriba; en el jet son dos piezas que nacen del fuselaje.
%
% ⚠️ ORDEN DE PINTADO: las alas van ANTES que el fuselaje, al revés que en el
%   jet. Las del jet son astillas en diagonal y no tapan nada; un ala recta es
%   una banda ancha que cruza el dibujo entero, y pintada encima convierte al
%   avión en una T. Debajo, el fuselaje blanco la parte en dos y vuelve a mandar.
%
% ⚠️ EL ALA LLEVA ESTRECHAMIENTO (cuerda 0.33 en la raíz, 0.20 en la punta) y no
%   es un rectángulo. Un rectángulo de la misma envergadura se lee como una
%   BARRA cruzando el dibujo; basta ese estrechamiento para que la misma banda
%   se lea como ala.
struct LightPlane() {
    % ala alta, recta y con estrechamiento, de una pieza sobre la cabina
    polygon(fill=gray(0.75), color="black", line_width=0.5) {
        0.24 0.07    0.20 1.13
        0.02 1.13    -0.09 0.07
        -0.09 -0.07  0.02 -1.13
        0.20 -1.13   0.24 -0.07 }
    % estabilizador horizontal, recto
    polygon(fill=gray(0.75), color="black", line_width=0.5) {
        -0.60 0.35   -0.60 -0.35
        -0.83 -0.35  -0.83 0.35 }
    % fuselaje de nariz roma: ahí va el motor, no una punta
    polygon(fill="white", color="black", line_width=0.6) {
        0.66 0.11    0.75 0.05
        0.75 -0.05   0.66 -0.11
        -0.72 -0.09  -0.95 -0.045
        -0.95 0.045  -0.72 0.09 }
    % la cabina, justo detrás del motor: el otro rasgo que se ve desde arriba
    polygon(fill=gray(0.72), color="black", line_width=0.35) {
        0.42 0.08    0.12 0.072
        0.12 -0.072  0.42 -0.08 }
    % deriva, vista en planta: una cuña ANGOSTA y DENTRO de la silueta del
    % fuselaje. Más ancha o más oscura deja de leerse como quilla y se lee como
    % punta de flecha saliendo por la cola, que voltea el avión.
    polygon(fill=gray(0.75), color="black", line_width=0.35) {
        -0.52 0      -0.92 0.032
        -0.92 -0.032 }
    % hélice: el disco es vertical, así que desde arriba se ve de canto.
    % Va CLARA y con el cono delante; oscura se leía como un timón y volteaba
    % el avión —parecía apuntar a la izquierda—.
    ellipse(0.03, 0.28, fill=gray(0.85), color="black", line_width=0.4) {
        0.79 0 }
    circle(0.05, fill="black") { 0.79 0 }
}
