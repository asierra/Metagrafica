% EXPECT_NO_WARN: número impar de coordenadas
% Una coordenada de `text` puede ser un PUNTO ya hecho —una lista de dos, como la
% que devuelven `xyz()` y `point_at()`—, y entonces vale por las dos. Es el caso
% natural de rotular algo calculado, y hasta el 2026-08-01 NO compilaba: la paridad
% se validaba en parse-time contando TÉRMINOS, así que un punto contaba como uno y
% el error decía «número impar de coordenadas (1)», señalando una línea correcta.
% Las primitivas ya lo aceptaban (PrimStmt::evalPath); `text` era la excepción.
%
% El fixture es EXPECT_NO_WARN y no un golden porque lo que fija es que COMPILE:
% la regresión natural es volver al chequeo estricto, y eso aborta.
% El reverso —que el error impar de `text` siga existiendo tras mudarse a
% eval-time— lo fija `text_impar.mg`.

display_size 6 6
world_window 0 6 0 6

% desde el espacio de la escena
view3d(azimuth=35, elevation=25)
text("CIV") { xyz(1, 2, 1) }

% y desde un trayecto
path p = { 0.5 0.5  3 1.5  5 4 }
polyline(&p)
text("A") { point_at(&p, 0.5) }

% mezclado con coordenadas sueltas, que es lo que obliga a validar al evaluar
text("B") { 1 5   point_at(&p, 0.9)   4 5 }
