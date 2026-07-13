% La manera eficiente de aproximar un cuarto de círculo es con BZ
% En este programa definimos los segmentos de arco más comunes, 
% con simetría central.

OPST ARCO1
BZ 
0 1.0  
0.55342686 0.99873585  
0.99873585 0.55342686 
1.0 0
}
CLST

OPST ARCO2
MKST ARCO1
PWST 0 0.5 1 1
PWST 0 0.5 1 0
CLST

OPST ARCO3
MKST ARCO1
PWST 0.5 0.5 1 1
PWST 0.5 0.5 1 0
PWST 0.5 0.5 0 0
CLST

OPST ARCO4
MKST ARCO1
PWST 0.5 0.5 1 1
PWST 0.5 0.5 1 0
PWST 0.5 0.5 0 0
PWST 0.5 0.5 0 1
CLST


OPST ARCO5
MKST ARCO1
PWST .5 0 1 1
PWST .5 0 0 1
CLST

