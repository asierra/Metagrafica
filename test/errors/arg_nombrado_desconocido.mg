% EXPECT: argumento nombrado desconocido en la struct
% Un typo en el nombre de un parámetro no puede quedarse sin ligar en silencio.
% at=/rotate=/scale= NO caen aquí: son modificadores de colocación (§8).
display_size 5 5
struct S(ancho) { polyline { 0 0  (ancho) 1 } }
S(anchp=2)
