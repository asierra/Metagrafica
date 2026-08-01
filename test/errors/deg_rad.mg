% EXPECT_NO_WARN: no definida
% `deg` y `rad` son el puente entre las trigonométricas (radianes) y los ángulos de
% las primitivas (grados). Llevan la MISMA constante que el motor (deg2rad, matrix.h),
% que es la razón de que existan en vez de escribir `* 180 / pi` a mano.
%
% Este fixture fija que existan y compilen en los tres backends; que sean redondas se
% comprueba aquí mismo, cerrando el círculo: rad(deg(x)) tiene que devolver x.

display_size 6 6
world_window -3 3 -3 3
a = rad(30)
arc(2, from=(deg(a)), to=(deg(a) + 120)) { 0 0 }
arc(2.5, from=(deg(rad(45))), to=(deg(rad(200)))) { 0 0 }
polyline { (2*cos(rad(0))) 0   (2*cos(a)) (2*sin(a)) }
