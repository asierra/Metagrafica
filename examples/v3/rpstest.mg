% Prueba de repeat — traducción V3 de examples/v1/rpstest.mg
%
% ⚠ AÚN NO COMPILABLE (no hay parser V3). Fixture de spec; su render debe igualar
% examples/v1/reference/rpstest.svg. Ejercita §8 (struct), §17 (repeat), §12
% (pluma moveto/step), §10 (place) y §7 (estado).

struct Cuadro() {
    % polyline en vez de rectangle para permitir rotaciones (comentario del V1)
    polyline { 0 0  1 0  1 1  0 1  0 0 }
}

% 1) Repetición simple: 3 copias avanzando en x
%    V1: SCST .3 .3 (tamaño del struct) + TLPP .3 0 (avance) + XYPP .05 .05 (pos)
moveto(0.05, 0.05)
step(translate=(0.3, 0))
repeat(Cuadro, count=3, scale=0.3)          % ⚠ G1: scale= = tamaño del struct (MTST)

align "center"
text(0.5, 0.94) { "Repeate Structure Test" }

% 2) 4 copias en diagonal, verdes, con struct deformado
color "green"
moveto(0.05, 0.37)
step(translate=(0.2, 0.135))
repeat(Cuadro, count=4, scale=(0.2, 0.125))  % ⚠ G2: scale anisotrópico (V1 SCST .2 .125)

% 3) Si la matriz de repetición no es identidad, se acumula en cada copia.
%    V1: XYPP .7 .5 (pos) + IDPP (sin avance) + RTRS 60 (rota acumulando) + RPST 5
color "red"
moveto(0.7, 0.5)
step(translate=(0, 0))                       % ⚠ G3: "sin avance" (V1 IDPP): ¿cómo se resetea step?
repeat(Cuadro, count=5, scale=(0.2, 0.125), transform=rotate(60))

% 4) Colocación puntual (V1: identifier placement `cuadro .1 .75 }`)
place(Cuadro, scale=(0.2, 0.125)) { 0.1 0.75 }
