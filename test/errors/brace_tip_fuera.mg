% EXPECT: va en (0,1)
% `tip` es la posición de la cúspide A LO LARGO del vano, o sea una fracción. Los
% extremos están excluidos a propósito: en tip=0 o tip=1 la cúspide coincide con un
% extremo del vano, no queda medio vano para el gancho y la llave degenera en un
% garabato. La geometría lo aguanta —`braceRadius` daría R=0 y no dibujaría nada—,
% que es justo el problema: saldría un archivo válido y VACÍO en vez de un error.
display_size 8 6
world_window 0 8 0 6
brace(depth=8, tip=1.4) { 4 1  4 5 }
