% EXPECT: 'pi' es una constante del lenguaje
% EXPECT_AT: 11:1
% `pi`, `true` y `false` se resuelven ANTES que las variables, así que ligarlos no tenía
% efecto: hasta el 2026-11-25 `pi = 3.1416` compilaba con exit 0 y `pi` seguía valiendo
% 3.14159…, sin una palabra. Quien lo escribe para fijar decimales obtiene otra cosa sin
% saberlo. La misma regla cubre la variable de un for y el parámetro de una struct; los
% usos legítimos —leerlos, y un path `&pi`, que vive en otro espacio de nombres— los
% cubre ventana_local_invertida.mg.
display_size 4 4
world_window 0 10 0 10
pi = 3.1416
text(str(2*pi, 4)) { 5 5 }
