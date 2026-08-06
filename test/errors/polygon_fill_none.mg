% EXPECT: polygon rellena por definición
% `polygon` rellena por definición (§4) —es lo que lo distingue de una polilínea
% cerrada—, así que `fill="none"` es una contradicción y no una opción. Se aceptaba
% EN SILENCIO y salía del revés: relleno NEGRO, y sin `color=` además sin contorno,
% o sea un manchón sólido donde se pidió el vacío. Es la peor forma del fallo mudo,
% porque el resultado no se parece en nada a lo pedido y nada avisa.
%
% La construcción que el autor quiere SÍ existe —`polyline(closed=true)`— y el
% mensaje la nombra: lo destapó una figura de curso cuyo autor leyó el fuente de
% primitives.cpp para explicarse el manchón y concluyó que no había forma de pedir
% un contorno cerrado sin relleno, teniéndola una línea más arriba en la referencia.
display_size 5 5
polygon(fill="none", color="black") { 1 1  4 1  4 4 }
