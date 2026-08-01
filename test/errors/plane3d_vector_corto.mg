% EXPECT: lista de tres
% Los vectores de plane3d son [x,y,z]. Un número suelto (o una pareja) dejaría
% el plano en su default silenciosamente y el dibujo saldría en el papel, plano.
display_size 5 5
world_window 0 5 0 5
view3d(azimuth=30, elevation=20)
{ plane3d(u=5)
  circle(1) { 0 0 } }
