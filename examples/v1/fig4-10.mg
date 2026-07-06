% Fig. 3.3 de IMQ, 3a. ed., sept. 1998
% Niveles del pozo infinito.

$D 12 6
$P 8

% Reemplazamos seno.mg con aproximaciones bezier
INPUT bzsinepaths

OPST POZO
PL 0 1 0 0 1 0 1 1 }
CLST

OPST ENR
WW 0 10 0 19
PL 0 1 10 1 ; 0 4 10 4 ; 0 9 10 9 ; 0 16 10 16 }
CLST

OPST PHI
WW 0 10 0 19
LWIDTH 0
LPATRN 2
PL 0 1 10 1 ; 0 4 10 4 ; 0 9 10 9 ; 0 16 10 16 }
LPATRN 0
LWIDTH 2

IDPT
TLPT 0 1
SCPT 1 0.0789473
BZ &sinpi

IDPT
TLPT 0 2.5
SCPT 1 0.1578946
BZ &sin2pi

IDPT
TLPT 0 7
SCPT .6666 .210526316
BZ &sin2pi
TLPT 10 9.5
SCPT .5 .5
BZ &sinpi

IDPT
TLPT 0 14
SCPT .5 .210526316
RPPT sin2pi 2
BZ &buffer
CLST

OPST RO
WW 0 10 0 19
LPATRN 2
LWIDTH 0
PL 0 1 10 1 ; 0 4 10 4 ; 0 9 10 9 ; 0 16 10 16 }
LPATRN 0
LWIDTH 2
IDPT
TLPT 0 2.5
SCPT 1 -0.08
BZ &cos2pi

TLPT 0 -37.5
SCPT .5 1
RPPT cos2pi 2
BZ &buffer

IDPT
TLPT 0 11
SCPT .3333 -0.105
RPPT cos2pi 3
BZ &buffer

IDPT
TLPT 0 18
SCPT .25 -0.105
RPPT cos2pi 4
BZ &buffer
CLST

WW 0 10 -2 19
SCST 0.4 0.895
LWIDTH 1
ENR 1 0 }
PHI 4 0 }
RO  7 0 }
LWIDTH 5
POZO 1 0 4 0 7 0 }

TSTYLE roman
% TLOT 3 0
% TLTD 3 0
XYDT 3.1 16 16{/iE}_1
XYDT 6.1 16 \varphi_4
XYDT 9.1 16 \rho_4

XYDT 3.1 9 9{/iE}_1
XYDT 6.1 9 \varphi_3
XYDT 9.1 9 \rho_3

XYDT 3.1 4 4{/iE}_1
XYDT 6.1 4 \varphi_2
XYDT 9.1 4 \rho_2

XYDT 3.1 1 {/iE}_1
XYDT 6.1 1 \varphi_1
XYDT 9.1 1 \rho_1

XYDT 0.7 1  /iE_1
XYDT 0.7 4  /iE_2
XYDT 0.7 9  /iE_3
XYDT 0.7 16  /iE_4

XYDT 1.9 -1.5 a)
XYDT 4.9 -1.5 b)
XYDT 7.9 -1.5 c)
