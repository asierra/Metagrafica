% EXPECT: sentencia de estado desconocida: colour
% `colour 0.5` compilaba, no hacía nada y no avisaba: emitStyleAttr YA devolvía
% bool y StateStmt::exec descartaba el retorno.
display_size 5 5
colour 0.5
polyline { 0 0  1 1 }
