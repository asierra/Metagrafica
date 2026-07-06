% markers-demo.mg — Demostración de los 6 marcadores estándar, de la orientación
% tangente sobre un path (§C.1 de plan_marcadores) y del color independiente
% marcador/curva (§B.4).
%
% Requiere la biblioteca de marcadores: `INPUT markers` (examples/markers.mg).
% Ejecutar desde examples/:  ../bin/mg markers-demo.mg markers-demo.eps
%
% NOTA: la orientación tangente se activa aquí con el control TEMPORAL `$O 1`
% (§C.1). En la gramática V3 será el atributo marker_orient="auto"; este archivo
% se reescribirá entonces. `IDST` resetea la matriz de estructura antes de cada
% grupo, porque `SCST` COMPONE sobre ella (no la reemplaza).

$D 12 12
WW 0 12 0 12
INPUT markers

TALIGN 1
XYDT 6 11.4 /bMarcadores estandar y orientacion tangente

% --- 1. Fila con los 6 marcadores estándar (sin orientar) ---
IDST
SCST .028 .028
MkCircle  1.5 10 }
MkSquare  3.5 10 }
MkDiamond 5.5 10 }
MkCross   7.5 10 }
MkX       9.5 10 }
MkArrow  11.5 10 }

% --- 2. Flechas orientadas siguiendo la tangente de una curva ($O 1) ---
LCOLOR blue
PL 1 6  3 8  6 8  9 6  11 5 }
LCOLOR black
IDST
SCST .04 .04
$O 1
MkArrow 1 6  3 8  6 8  9 6  11 5 }

% --- 3. Dispersión: línea roja, puntos negros (marcador ≠ color de curva) ---
$O 0
LCOLOR red
PL 1 2  3 3.2  5 2.4  7 3.6  9 2.8  11 3.4 }
LCOLOR black
IDST
SCST .03 .03
MkCircle 1 2  3 3.2  5 2.4  7 3.6  9 2.8  11 3.4 }
