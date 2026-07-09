% Flechas (no rellenas) usadas como marcadores — traducción V3 de
% examples/v1/arrow.mg. Puntos verbatim del V1. Se usan como structs-marcador
% colocadas con place (§10.1); ⚠ su tamaño va por scale (cantidad de mundo) — el
% render físico del marcador colocado sigue pendiente (plan_marcadores).

% Flecha grande (doble punta), en su propia ventana
struct Arrow() {
    world_window 0 2 0 3
    polyline { 1 0  1.2 1  1.5 2  2 3  1 2  0 3  0.5 2  0.8 1  1 0 }
}

% Flechas con punta en el origen, hacia abajo / arriba / izquierda / derecha
struct Arrowd() { polyline { 0 0  0.2 1  0.5 2  1 3  0 2  -1 3  -0.5 2  -0.2 1  0 0 } }
struct Arrowu() { polyline { 0 0  0.2 -1  0.5 -2  1 -3  0 -2  -1 -3  -0.5 -2  -0.2 -1  0 0 } }
struct Arrowl() { polyline { 0 0  1 0.2  2 0.5  3 1  2 0  3 -1  2 -0.5  1 -0.2  0 0 } }
struct Arrowr() { polyline { 0 0  -1 0.2  -2 0.5  -3 1  -2 0  -3 -1  -2 -0.5  -1 -0.2  0 0 } }
