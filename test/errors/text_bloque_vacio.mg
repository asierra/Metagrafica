% EXPECT: 'text' tiene el bloque de coordenadas vacío
% EXPECT_AT: 9:1
% Un text con el bloque VACÍO (o sin bloque) no dibujaba nada y compilaba con exit 0:
% cero <text> en el SVG. La especificación lo prometía en la posición de pluma, que V3
% no tiene (descartada el 2026-11-25), así que es un error. La forma sin bloque,
% `text("hola")`, da el mismo error con «no tiene bloque de coordenadas».
display_size 4 4
world_window 0 1  0 1
text("hola") { }
