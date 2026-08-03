% =====================================================================
%  people.mg — La figura humana como pictograma, en dos lecturas.
%
%  Ambas van centradas en el origen y a tamaño canónico (alto ≈ 1 unidad de
%  mundo), así que `scale=` es el factor directo y `at=` las coloca igual.
%  Los grosores de línea NO escalan: se quedan en puntos.
%
%  Ejemplo:  Person(scale=1.0, at=(7.4, 2.6))
%            PersonBust(scale=1.0, at=(13.2, 5.4))
%
%  ⚠️ SON DOS Y LA DIFERENCIA ES DE USO, no de estilo. Elegir mal no rompe nada
%    pero se ve:
%
%    · `Person` (cuerpo entero) para figuras PARADAS SOBRE UN PLANO: tienen
%      pies, y el contacto con el suelo es parte de lo que la figura afirma
%      —alguien está ahí, en el terreno—.
%    · `PersonBust` (cabeza y hombros) para la INSIGNIA DE GRUPO: sin suelo,
%      solapadas y de un centímetro. A ese tamaño el cuerpo entero encogido
%      deja la cabeza demasiado chica respecto a la mancha y el grupo se lee
%      como manchas; el busto acerca el encuadre y la cabeza vuelve a mandar.
%
%  Sobre la procedencia: es el vocabulario de señalética corriente (círculo,
%  silueta sólida, hombro redondeado) construido con primitivas, coordenada por
%  coordenada, no el calco de ningún icono ajeno.
% =====================================================================

% --- Person: cuerpo entero, de frente ---------------------------------
% Cabeza más oscura que el cuerpo: a tamaño chico ese contraste es lo que
% mantiene la cabeza separada de los hombros sin depender del contorno.
%
% La silueta es la de señalética corriente —hombros, brazos caídos, cintura y
% dos piernas con la escotadura en medio—, y LAS PIERNAS SON LA RAZÓN DE SER de
% esta variante frente a `PersonBust`: sin ellas no hay pies, y sin pies no hay
% contacto con el suelo que dibujar.
%
% ⚠️ LA PROPORCIÓN ES DE PICTOGRAMA, unas 4.3 cabezas de alto, no las 7.5 de una
%   figura humana real. Un pictograma alarga la cabeza a propósito: es lo que se
%   reconoce primero y lo único que sobrevive cuando la figura mide un
%   centímetro. Dibujada con proporción anatómica, a ese tamaño la cabeza se
%   convierte en un punto y la silueta se lee como un poste.
%
% ⚠️ NO ES CALCO DE NINGÚN ICONO AJENO. La forma —círculo sobre silueta sólida
%   con escotadura entre las piernas— es vocabulario de señalética, común a
%   cientos de juegos de iconos y a ninguno en particular; aquí está construida
%   con primitivas, coordenada por coordenada. Si alguna vez se quiere afinar,
%   se afina la geometría, no se traza encima de un archivo.
struct Person(head=gray(0.35), body=gray(0.7)) {
    circle(0.135, fill=head, color="black", line_width=0.4) { 0 0.545 }
    polygon(fill=body, color="black", line_width=0.4) {
        -0.100 0.405   -0.170 0.345
        -0.190 0.180   -0.180 -0.040
        -0.150 -0.110  -0.145 -0.470
        -0.048 -0.470  -0.030 -0.185
         0.030 -0.185   0.048 -0.470
         0.145 -0.470   0.150 -0.110
         0.180 -0.040   0.190 0.180
         0.170 0.345    0.100 0.405 }
}

% --- PersonBust: cabeza y hombros --------------------------------------
% El hombro es medio arco elíptico: redondo a propósito, porque es lo que
% separa el busto de una `Person` recortada. La base queda plana (la cuerda del
% arco), así que varios bustos se alinean solos sobre una misma horizontal.
struct PersonBust(head=gray(0.35), body=gray(0.7)) {
    circle(0.235, fill=head, color="black", line_width=0.4) { 0 0.33 }
    arc(0.42, 0.40, from=0, to=180, fill=body, color="black",
        line_width=0.4) { 0 -0.30 }
}

% --- PeopleGroup: tres bustos solapados, el de en medio al frente ------
% El grupo de usuarios, la lectura de «varios» sin repetir la invocación en el
% dibujo. Se pinta de atrás hacia adelante (§ sin z-buffer: manda el orden).
% El desplazamiento vertical de los de atrás NO es a ojo: se calcula para que
% las tres bases (la cuerda del arco, en y=-0.30 a escala 1) caigan sobre la
% MISMA horizontal pese a ir a escala distinta. Puestos a la misma altura que
% el de enfrente sobresalían por abajo y el grupo quedaba con escalón.
%
% ⚠️ LOS DE ATRÁS VAN MÁS CLAROS, y no es adorno: con el mismo tono los tres
%   hombros se funden en una sola mancha con tres cabezas encima, porque lo
%   único que los separa es un contorno de 0.4 pt que a este tamaño casi no
%   pesa. El salto de tono hace el trabajo que el contorno no puede.
struct PeopleGroup(sep=0.52, back=0.82) {
    dy = -0.30 * (1 - back)
    PersonBust(scale=back, head=gray(0.48), body=gray(0.82), at=(-sep, dy))
    PersonBust(scale=back, head=gray(0.48), body=gray(0.82), at=(sep, dy))
    PersonBust(scale=1.0, at=(0, 0))
}
