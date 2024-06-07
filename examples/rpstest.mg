% RPST test

OPST cuadro
% Use PL instead of BR to allow rotations
PL 0 0  1 0  1 1 0 1 0 0 }
CLST

MKST cuadro

% Most of the times, RPST will just create a path
SCST .3 .3
TLPP .3 0
XYPP .05 .05
RPST 3

LCOLOR green
XYPP .05 .37
IDST
SCST .2 .125
IDPP
TLPP .2 .135
RPST 4

% If the RS matrix is not the identity, it will
% be applied each time a structure is repeated.
LCOLOR red
XYPP .7 .5
IDPP
RTRS 60
RPST 5

% The ST matrix must not be changed after RPST
cuadro .1 .75 }
