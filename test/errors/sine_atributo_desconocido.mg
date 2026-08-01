% EXPECT: atributo desconocido en la primitiva: sine
% Hasta el 2026-08-01 `sine` tenía clase propia (SineStmt) y su parser aceptaba
% CUALQUIER argumento nombrado sin validarlo: `sine(color="red")` compilaba sin una
% queja y no hacía nada, y un `half_cicles=` mal escrito se perdía igual. Es la misma
% clase de no-op mudo que el proyecto ya había cerrado dos veces para las demás
% primitivas; `sine` se libró de aquellas pasadas por no pasar por PrimStmt.
%
% Ahora se enruta por PrimStmt, así que hereda la lista blanca. El fixture usa el
% typo de un argumento de GEOMETRÍA a propósito: es el que demuestra que el reparto
% —geometría al generador, el resto a la lista blanca— no deja un agujero por el que
% se cuele lo que sobra.
%
% Que los atributos SURTAN EFECTO no lo fija esto sino el golden de `sines.mg`, que
% dibuja una onda rellena, una morada gruesa y una discontinua.

display_size 6 6
world_window 0 6 0 6
sine(half_cicles=2, amplitude=1) { 1 3  5 3 }
