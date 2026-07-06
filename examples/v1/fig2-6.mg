% Figura 2.5 de IMQ2, 3a ed.,  sept. 1998, dic. 2002
% Diagrama de difraccion de electrones.

$D 12 4
$P 8

INPUT arrow

OPST PILA
LCOLOR red
WW -.125 .875 .5 1.5
LWIDTH 1
PL 0 0 0 1 ; .5 0 .5 1 ;
 -.125 .5 0 .5 ; .75 .5 
 .875 .5 }
LWIDTH 8
PL .25 .2 .25 .8 ; .75 
.2 .75 .8 }
LWIDTH 0
CLST

OPST DETECTOR
WW .5 1.2 .5 1.2
LWIDTH 5
PL 1 .3  1 0  0 0  0 1  1 1  1 .7  }
CLST

WW -1 11 -1 3

%  Construccion del dispositivo.

%$C 1 para que era $C ?
%FILL CR .08 : -.5 1 -.5 1.5 } % OJO 
% El nuevo comando DOT dibuja puntos negros. Se define el tamaño en puntos
% tipográficos en el primer parámetro y después, las posiciones de cada
% punto.
DOT 4 -.5 1 -.5 1.5 } 

LWIDTH 1
PL
-.5 1  .4 1  .4 1.15  .55 1.25  .4 1.35  .4 1.5  -.5 1.5 ;
.5 .8 -.2 .8 -.2 -.4  2 -.4 ; 2.6 -.4  4.5 -.4  4.5 0
}

LPATRN 2
PL 4.5 0  4.5 1 ; 4.5 1.5  4.5 2.5 }
LPATRN 0
LWIDTH 2
PL 1 0 1 1 ; 1 2.5 1 1.5 ; 8.9 .5  8.9 2  9.1 2  9.1 .5  8.9 .5 ;
.55 1.25 .4 1.35 }
% Catodo
LWIDTH 1
PL .5 .8 .7 .8 ; .7 1.7 .5 1.7 }
LWIDTH 8
PL .7 .8 .7 1.7 }
LWIDTH 2
PL 5.7 1.15  5.7 0  0 0  0 2.5  5.7 2.5  5.7 1.35 }
%SCST .033333 .1 OJO Ya no hace falta ajustar la distorsión de escala
SCST .1 .1
RTST 37
DETECTOR 7 -.2 }
MKST PILA
PWST 2 -.4 2.6 0

%   Construccion de los rayos catodicos 
LWIDTH 0
% Cotas:
PL 8 1.25 8.9 1.25 7.19 -.0552 }

LWIDTH 1
LPATRN 3
% Con ticks se puede hacer lo mismo que con el antiguo DRAW pero
% es más claro con vectores que con posiciones absolutas.
% Hay un paso mas al indicar la posicion de la pluma, pero vale
% la pena pues se gana claridad.
%TLDR 0 .1
%DRAW 10 .76 .8 1 .8 }
%DRAW 4 1 1.1 5.7 1.1 }
%DRAW 2 5.7 1.2 8.9 1.2 }
TLPP 0 .1
XYPP .76 .8
TICKS 10 .24 0
XYPP 1 1.1
TICKS 4 4.7 0
XYPP 5.7 1.2
TICKS 2 3.2 0

LPATRN 0
LWIDTH 1
% Estas tres líneas sustituyen todas las comentadas de abajo.
% ARCST y LINST colocan una estructura en uno o ambos extremos, con los
% mismos parámetros para línea y arco (CR), respectivamente, precedidos
% por la escala, que si es negativa se usan ambos extremos, si no, en el 
% segundo extremo de la línea o en el segundo ángulo. El separador de : es
% porque hay un parámetro opcional para indicar desplazamiento a partir
% de los extremos.
MKST arrowr
LCOLOR blue
ARCST -.02 2.39 40 197.35 : 8.9 1.25
LCOLOR black
LNST .02 : 8.9 1.25 7.5 .1816
%IDST 
%SCST .5 .5
%SCST .015 .04 % .02 .06
%RTST -7
%arrowsw 7.5 .1816 }
%IDST
%SCST .5 .5
%SCST .015 .04
%RTST 17.4
%arrowu 6.6144 .55 }
%IDST
%SCST .5 .5
%SCST .015 .04  OJO Ya no hace falta ajustar la distorsión de escala
%arrowse 7.61 -.7623 }
%CR 2.39 40 197.35 : 8.9 1.25 }

CR .7 38 180 : 8.9 1.25 }
TSTYLE roman
XYDT .1    2.70 cathode
XYDT  4    2.70 anode
XYDT 9.3  1.75 crystal
XYDT 7.8   -.50 detector
XYDT 2.20 -1.0 /iV
XYDT 2.55 -.75 +
XYDT 8 .9 \phi

