% EXPECT: atributo desconocido en text()
% Mismo silencio que tenían las primitivas, por la puerta de atrás: text() no pasa
% por PrimStmt, así que se quedó fuera del arreglo del 2026-07-22 hasta este.
% Lista PROPIA y no la de las primitivas: los ejes no se solapan (align/valign/font
% no tienen sentido en un polyline, ni hatch/closed/marker_* en un text).
display_size 5 5
text("hola", tamano=20) { 1 1 }
