% EXPECT: número impar de coordenadas
% El reverso de `text_punto_calculado.mg`. Al aceptar `text` puntos calculados, su
% chequeo de paridad tuvo que MUDARSE de parse-time a eval-time (un punto guardado
% en variable no se distingue de un número hasta evaluarlo). Mudar una comprobación
% es justo cuando se pierde: `coords_impares.mg` vigila la de las primitivas y no
% habría notado nada si la de `text` desaparecía en el camino.

display_size 5 5
world_window 0 5 0 5
text("a") { 1 }
