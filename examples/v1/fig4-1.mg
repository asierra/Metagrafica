% Fig 2.1 de IMQ, 3a. ed., sept. 1998, dic. 2002
% Difraccion de electrones

% Los cambios de 2024 aprovechan algunas mejoras a MG, como hacer las
% flechas repetitivas con los LNST múltiples. Y considerar que las
% deformaciones por razón de aspecto no cuadrado se compensan
% automáticamente.
$D 11 7.7
$P 8

INPUT arrow.mg
INPUT curvas3.mg

OPST FG1
RTST 90
SNDVTSQ 1 0 }
CLST

OPST FG2
RTST 90
SNCSDVTSQ 1 0 }
CLST

OPST MARCO
WW 0 2.2 0 1
LWIDTH 0
% Pantallas
PL
.3 0 .3 .45 ; .3 .55 .3 1 ; .9 0 .9 1 ;
1.6 0 1.6 .3 ; 1.6 .4 1.6 .6 ; 1.6 .7 1.6 1 ; 2.2 0 2.2 1
}
CLST

OPST flechas
WW 0 2.2 0 1
LWIDTH 1
IDPP
TLPP 0 .1
MKST arrowr
LNST .02 0 9: .01 .1 .3 .1
CLST

OPST FIG1
MKST MARCO
PWST 0 0 1 1
WW 0 2.2 0 1
LWIDTH 2
MKST FG1
PWST .35 0 .9 1 
MKST FG2
PWST 1.6 0 2.2 1
flechas 0 0 1.3 0 }
CLST

OPST FIG2
MKST MARCO
PWST 0 0 1 1
WW 0 2.2 0 1
LWIDTH 2
PL .9 .45 .5 .45  .5 .55  .9 .55 ;
2.2 .3  1.8 .3  1.8 .4  2.2 .4 ;
2.2 .7  1.8 .7  1.8 .6  2.2 .6 ;
} 
flechas 0 0 1.3 0 }
LWIDTH 0
CLST

WW -.07 1.03 -.05 1.05
MKST FIG1
PWST 0 0 1 .45
MKST FIG2
PWST 0 .55 1 1

XYDT .22 .53 a)
XYDT .81 .55 b)
XYDT .22 -.03 c)
XYDT .81 -.03 d)

