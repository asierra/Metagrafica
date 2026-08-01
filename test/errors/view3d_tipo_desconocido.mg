% EXPECT: view3d: type desconocido
% Un type mal escrito caía en silencio a la proyección axonométrica y la figura
% salía plausible pero con OTRA cámara que la pedida.
display_size 5 5
world_window 0 5 0 5
view3d(type="isometrica", angle=30)
polyline { xyz(0,0,0)  xyz(1,1,1) }
