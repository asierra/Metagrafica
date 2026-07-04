% Fig. 7.1 IMQ, 3a. ed., oct. 1998
% Para el metodo WKB

$D 12 3.4
$P 8

INPUT bzsinepaths

WW 0 12 -.7 2.7
LWIDTH 0
FPATRN 2
PG 2.5 0 3.5 0  3.5 .5  3.15 .5  3.15 .8  3.5 .8  3.5 1.7 2.5 1.7 2.5 0 }
PG 8.5 0 9.5 0  9.5 .5  9.15 .5  9.15 .8  9.5 .8  9.5 1.7 8.5 1.7 8.5 0 }

PL .2 0 11.2 0 ; .2 1.7 11.2 1.7 ; 3 -.4  3 2 ; 9 -.4  9 2 }
LPATRN 2
PL 2.5 0 2.5 2 ; 3.5 0 3.5 1.7 ; 8.5 0 8.5 1.7 ; 9.5 0 9.5 2 }
LPATRN 0

LWIDTH 4
% Like with PWST but for paths and the result is stored in 
% the path buffer.
PWPT cos2pi 1 .4 11 2.4
BZ &buffer
%MKST SEN1
%PWST .4 2.7 6 .4
%PWST 6 .4 11 2.4

% TLCT 4.5 0 % OJO NO funcionan. Es que ahora usamos PP
TLPP 4.5 0
XYPP 1.5 1.2
DT I 
DT II
DT III 

% TLCT 4.47 0
%IDPP
%TLPP 4.47 0
XYPP 1.40 .13
DT \psi{_I} 
DT \psi{_II} 
DT \psi{_III}

% TLCT 6 0
% TLOT 1 0
XYDT 3.2 .6 \psi{_1} 
XYDT 9.2 .6 \psi{_2}
XYDT 2.3 -.3 {/ix}{_1}\prime
XYDT 8.3 -.3 {/ix}{_2}\prime
XYDT 3.3 -.3 {/ix}{_1}\prime\prime
XYDT 9.3 -.3 {/ix}{_2}\prime\prime
XYDT 2.9 -.62 {/ix}{_1} 
XYDT 8.9 -.62 {/ix}{_2}
XYDT 11.1 2.3 {/iV}({/ix})
XYDT 11.3 1.6 /iE

