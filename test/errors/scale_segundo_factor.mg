% EXPECT: variable no definida: sy
% Fixture INDIRECTO, y conviene saber por qué: el arreglo de `scale sx sy` es un
% cambio POSITIVO (antes el 2º factor se descartaba en silencio y el escalado
% quedaba uniforme), así que una prueba negativa no puede cubrirlo de frente.
% Lo que sí discrimina es CUÁL error sale: si el 2º factor se TOMA, se evalúa y
% falla por la variable; si se descarta, el sobrante revienta como sentencia con
% «se esperaba una expresión». Verificado que los dos mensajes difieren.
display_size 5 5
{ scale 2 sy
  polyline { 0 0  1 1 } }
