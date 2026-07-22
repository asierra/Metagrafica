% EXPECT: número impar de coordenadas
% Caza el silencio de checkCoordPairs: antes la coordenada sobrante se descartaba
% SIN evaluarla, y la primitiva desaparecía muda.
display_size 5 5
polyline { 0 0  1 }
