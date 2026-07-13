$D 8 4.6
$P 8

% En esta figura usamos la transformación lineal SHEAR, que se
% usa para simular perspectiva, entre otras cosas. Creamos un solo
% cuadro y cuando se aplica la transformación, todas las estructuras
% se deforman hasta que es inicializa su matriz con IDST.

% Alejandro Aguilar Sierra, 2024

INPUT arrow

OPST cuadro
WW 0 1.5 0 1 
LWIDTH 3
FGRAY -80
PG 0 0  0 1  1 1  1 0  0 0 }
CLST

OPST corner
WW 0 1.5 0 1
LWIDTH 0
LPATRN 3
PL 1 .5  1 0  .72 0 }
CLST



OPST caja
% La transformación shear  usa mismos parámetros que escala
%SHST 0 1.1
cuadro 0 0  1.45 0  1.78 0 }
corner 1.45 0 }
IDST

LWIDTH 0
LPATRN 3
PL 0 0  1.78 0 }
PL 0 .69  1.78 .69 }
PL .46 .505  2.24 .505 }
PL .46 1.2  2.24 1.2 }
LPATRN 0
PL 0 -.02  0 -.08 }
PL 1.45 -.02  1.45 -.08 }
PL 1.78 -.02  1.78 -.08 }

MKST arrowr
LNST .03 : .87 -0.05  1.45 -0.05
LNST .03 : .58 -0.05  0 -0.05
LNST .03 : 1.65 -0.05  1.78 -0.05
LNST .03 : 1.57 -0.05  1.45 -0.05

TSTYLE italic
TALIGN 1
XYDT 0.725 -0.075 D=A-R
XYDT 1.615 -0.075 R
XYDT 2.1 .22 L
XYDT 2.3 0.73 L
CLST

WW 0 8 0 4.6
SCST .69 .69
caja .3 .5 }

