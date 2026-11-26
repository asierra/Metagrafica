% EXPECT: 'circle' no tiene bloque de coordenadas
% EXPECT_AT: 12:1
% Una primitiva SIN coordenadas no tiene dónde dibujarse: V3 no tiene posición de
% pluma (descartada el 2026-11-25). Hasta esa fecha `circle(4)` compilaba con exit 0 y
% dejaba la figura EN BLANCO, sin una palabra. Pasar de error a válido es compatible
% si algún día hiciera falta; lo contrario cambiaría figuras en silencio. El bloque
% vacío (`circle(4) { }`) da el mismo error. Las formas legítimas sin bloque
% —`rectangle(w=, h=, at=)` y la de trayecto `polyline(&p)`— las cubre
% primitiva_sin_bloque_legitima.mg.
display_size 4 4
world_window -5 5  -5 5
circle(4)
