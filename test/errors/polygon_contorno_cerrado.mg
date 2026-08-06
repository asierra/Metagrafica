% EXPECT_NO_WARN: polygon rellena por definición
% La contraparte legítima de polygon_fill_none.mg: `polyline(closed=true)` ES la
% forma de pedir un contorno cerrado sin relleno, y tiene que seguir compilando
% limpio en los TRES backends. Sin esta prueba, la manera fácil de "arreglar" un
% falso positivo del error nuevo sería endurecerlo hasta que también alcance a la
% construcción que el propio mensaje recomienda — y entonces el diagnóstico
% dejaría de tener salida que ofrecer.
display_size 5 5
polyline(closed=true, color="black") { 1 1  4 1  4 4 }
