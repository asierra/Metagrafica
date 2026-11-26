% Un error dentro de un `include` se cuenta desde la línea 1 del INCLUIDO. Antes el
% contador de flex seguía desde el principal y esto decía «en 6:9».
% EXPECT: Error de sintaxis en 2:9
polyline { 0 0  1 1 }
include "inc/linea2.mg"
