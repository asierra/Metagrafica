% EXPECT: Si lo escribiste como x0 y0 x1 y1, es: world_window 0 10 0 13
% El orden de world_window es xmin xmax ymin ymax. Escrito con el de SVG/PostScript
% (x0 y0 x1 y1), `0 0 10 13` da xmin == xmax: hasta el 2026-11-25 compilaba con solo
% el aviso «la figura sale EN BLANCO», que dice el síntoma y no la causa. Ahora es error
% y, como reordenar da una ventana válida, sugiere la correcta. (Sin EXPECT_AT:
% evalError no lleva ubicación; el mensaje repite los cuatro valores.)
display_size 4 4
world_window 0 0 10 13
polyline { 1 1  9 12 }
