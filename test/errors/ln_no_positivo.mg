% EXPECT: ln: argumento no positivo
% evalError es FATAL a propósito: en franck_condon este error reportó un error de
% FÍSICA (D no es parámetro libre; la relación de Morse lo fija) como error de
% compilación. Es el argumento más fuerte a favor de abortar.
display_size 5 5
a = ln(0)
polyline { 0 0  1 1 }
