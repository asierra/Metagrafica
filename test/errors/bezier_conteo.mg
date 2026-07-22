% EXPECT: van en grupos de 3k+1
% EXPECT_AT: 6:34
% Un conteo que no cerraba se descartaba en silencio (5 o 6 puntos dibujaban lo
% mismo que 4). Al implementarlo cazó un punto muerto real en fig1.mg.
display_size 5 5
bezier { 0 0  1 1  2 1  3 0  4 0 }
