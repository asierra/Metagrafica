% Fig. 2.6 de IMQ, 3a. ed., sept. 1998
% Difraccion de electrones por un cristal.

$D 3.5 2.5
$P 8

INPUT arrown
INPUT arco1


WW -.3333 4.3333 -.48 2.02
LWIDTH 0
%FILL CR .07 : 0 0  1 0  2 0  3 0  4 0 }
DOT 4 0 0  1 0  2 0  3 0  4 0 }

WW 0 3.5 -.5 2
% Cotas:
PL .75 -.5  .75 -.2 ; .75 -.35  1.5 -.35 ; 1.5 -.5 1.5 -.2 ; 1.5 0 1.02 .35 }
%$C 1
CR 1 36.87 53.13 : .75 0 }
% Haz incidente:
LWIDTH 1
%PL .75 2  .75 0  2.25 2 ; 1.5 2  1.5 0  3 2 }
MKST arrownr
% En este caso usamos el parámetro opcional para alejarnos de los
% extremos. Si es cercano a 0, se desplaza sobre la línea un poco 
% alejado del extremo. Si es cercano a 1, se acerca al extremo opuesto.
LNST .035 0.1 : .75 0  2.25 2
LNST .035 0.1 : 1.5 0 3 2
LNST .035 0.8 : .75 2  .75 0
LNST .035 0.8 : 1.5 2  1.5 0

%PL .75 2  .75 0  2.25 2 ; 1.5 2  1.5 0  3 2 }
%LWIDTH 3
%SCST .015 .03
%arrowd .75 1.65 1.5 1.65 }
%arrowne 2.14 1.85  2.882 1.85 }

% Remarcar segmento:
PL .75 0 1.02 .36 }
LWIDTH 0
% Arco para acotacion:
MKST ARCO1
PWST .86 .38 2.2 .2

XYDT 1 -.32 /id
XYDT 2.1 .4 /id 
XYDT 2.28 .4 /rsin\phi
XYDT 1 1.1 \phi
