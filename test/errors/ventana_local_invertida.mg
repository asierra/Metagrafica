% EXPECT_NO_WARN: world_window
% Casos LEGÍTIMOS que las reglas del 2026-11-25 no deben tocar:
% - Leer `pi`/`true` en una expresión, y un parámetro de PATH llamado `&pi` (los paths
%   viven en otro espacio de nombres).
% - La ventana LOCAL de una struct INVERTIDA: se normaliza por eje, y ahí invertir uno
%   lo refleja, y solo a él (`1 0 0 1` refleja x). Solo la DEGENERADA es error dentro de
%   una struct; la del documento, invertida o degenerada, también lo es.
display_size 4 4
world_window 0 10 0 10
struct Espejo() {
    world_window 1 0  0 1
    polyline { 0 0  1 0  1 0.5 }
}
struct Trazo(&pi) { polyline(&pi) }
path q = { 1 9  3 9 }
fit(Espejo, stretch=true) { 2 2  8 8 }
Trazo(&q)
x = 2*pi
if true { text(str(x, 3)) { 5 5 } }
