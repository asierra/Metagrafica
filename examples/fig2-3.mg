% Fig. 2.3 The variation of the heat capacity of diamond 
% with temperature % compared with the prediction of Einstein’s 
% quantum theory.

$D 13 8
$P 8

WW -1 11  -1 7

% Curva
LWIDTH 4
LPATRN 2

% Este segmento es recto, no hace falta aproximarlo.
PL 0 0  0.76 0 }
% Cada segmento de una curva bezier requieren 4 puntos, 
% p1 y p4 están sobre la curva y los otros dos,  p2 y p3 
% son sus respectivos vectores tangente.
BZ 
0.7600 0
2.6602 0.0247
1.7879 5.0734
9.9809 5.4533 }

% Puntos experimentales, se puede usar cualquier marcador
LWIDTH 2
LPATRN 0
CR 0.1 :
1.6011 0.7290
1.9421 1.1410
2.1008 1.3624
2.2753 1.5638
2.4370 1.8089
2.6464 2.0984
3.0605 2.6922
3.5098 3.3768
3.8756 3.6056
6.5876 5.3581
8.0844 5.3956
9.4408 5.5524 }

% ejes
LWIDTH 1
PL 0 6 0 0 10 0 }

% Para el estilo de texto, ahora se pone el nombre en minúsculas
TSTYLE italic

% PP siempre denota la posición de la pluma, que se puede
% indicar específicamente con el comando XYPP 
XYPP .7 -0.3

% El prefijo TL es traslación y el sufijo indica la matriz,
% que en este caso es la que controla la posición de la pluma. 
TLPP 1 0

% GNNUM genera una serie de números con 4 parámetros en este orden:
% Número inicial, incremento, número de números, cuántos decimales.
% Cada que se grafica un número, se actualiza la posición de la pluma
% usando la matriz PP.
GNNUM 0.1 0.1 10 1

XYPP -0.2 -0.1
% el prefijo ID inicializa la matriz
IDPP
TLPP 0 1
GNNUM 0 1 7 0

% Ticks
LWIDTH 0
LGRAY 50

IDPP
TLPP 1 0
XYPP 1 0

% Como su nombre lo indica, genera ticks con los parámetros:
% número de ticks y vector del tick
% Cada tick actualiza la pp.
TICKS 10 0 6

IDPP
TLPP 0 1
XYPP 0 1
TICKS 6 10 0

TSTYLE roman
TSIZE 10
XYDT 4.3 -0.7 relative temperature

XYPP -0.5 2.3
RTLC 90
DT heat capacity



