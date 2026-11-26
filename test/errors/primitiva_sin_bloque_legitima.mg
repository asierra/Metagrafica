% EXPECT_NO_WARN: bloque de coordenadas
% Las dos formas de primitiva que SÍ van sin bloque, porque llevan su posición en los
% argumentos: rectangle(w=, h=, at=) —centro + tamaño; sin at= se centra en el origen—
% y la de trayecto, polyline(&p). El error de «primitiva sin bloque» (2026-11-25) no
% debe tocarlas: si las alcanzara, un .mg correcto dejaría de compilar.
display_size 4 4
world_window -5 5  -5 5
path p = { -4 -4  4 4 }
rectangle(w=2, h=1, at=(1, 1))
rectangle(w=3)
polyline(&p)
