% Basic shapes

% Con estas dimensiones, 1 equivale a 0.5 cm
% (pues en total mide 11 cm)
$D 11 11

% 15 puntos es aproximadamente 0.5 cm
$P 15

WW -1.1 1.1 -1.1 1.1

% Para checar ancho de línea en unidades 0.2 de punto tipográfico
LWIDTH 60
PL 0.05 0.1 0.05 0.2 }

LWIDTH 0

% Para checar tamaño de letra
PL 0.1 0 0.1 0.1}
XYDT 0 0 M

% Ejes
PL -1 0 1 0 }
PL 0 -1 0 0.95 }

% Se puede centrar el texto
TALIGN 1

XYDT 0 1 Primitivos gráficos

% Podemos cambiar la altura del texto
% en unidades de puntos tipográficos
THEIGHT 10

LWIDTH 4

XYDT 0.5 0.75 Círculo y arcos
CR 0.2 : 0.5 0.5 }
CR 0.25 90 : 0.5 0.5 }
CR 0.3 15 30 : 0.5 0.5 }
CR 0.3 -30 : 0.5 0.5 }

XYDT -0.5 0.75 Elipse
EL 0.2 0.1 : -0.5 0.5 }

XYDT 0.5 -0.2 Rectángulo
BR 0.5 -0.25 0.75 -0.6 }

XYDT -0.5 -0.2 Polígono
PL
-0.2000 -0.5000
-0.4073 -0.2147
-0.7427 -0.3237
-0.7427 -0.6763
-0.4073 -0.785
-0.2000 -0.5000
}

% Los mismos pero rellenos
%FGRAY 50
LCOLOR green
FCOLOR -cyan
FPATRN -2
FILL
%RTLC 45
TLLC -0.2 -0.15

PG
-0.2000 -0.5000
-0.4073 -0.2147
-0.7427 -0.3237
-0.7427 -0.6763
-0.4073 -0.785
-0.2000 -0.5000
}

CR 0.2 : 0.5 0.5 }

EL 0.2 0.1 : -0.5 0.5 }

BR 0.5 -0.6 0.75 -0.25 }


