% Examples of bezier approximated sine curves usage.
%
% Author: Alejandro Aguilar Sierra, algsierra@gmail.com
% Version 1.0, April 2024

INPUT bzsinepaths

% Computed sine points to test curves
OPST CRSINEPI2
CR 0.02 :
0.0000 0.0000
0.1564 0.2433
0.3090 0.4666
0.4540 0.6542
0.5878 0.7976
0.7071 0.8960
0.8090 0.9553
0.8910 0.9854
0.9511 0.9970
0.9877 0.9998
1.0000 1.0000
}
CLST 

% All defined curves can be transformed with the matrix PT.
% A copy is created, the original path is not transformed.
TLPT .1 .75
SCPT .3 .2
BZ &cospi2

TLPT 1.6 0
BZ &sinpi2

IDPT
TLPT .1 .5
SCPT .3 .2
BZ &cospi

TLPT 1.6 0
BZ &sinpi

IDPT
TLPT .1 .25
SCPT .3 .2
BZ &cos2pi

TLPT 1.6 0
BZ &sin2pi

%LWIDTH 0
%IDPP
%XYPP 0 .25
%TLPP 0 .25
%TICKS 3 1 0

%LCOLOR blue
%XYPP 0 .45
%TICKS 3 1 0

% With CTPT and the same PT matrix you can concatenate 
% curves in a new larger one. Not much is automated yet,
% you have all the control.
% This is work in progress, surely it will be improved
% before the next official release of MG.
CTPT concatcurve
SCPT .2 1
RPPT sin2pi 3
SCPT .5 .5
&sinpi
CLPT

LCOLOR red
SCST .3 .2
CRSINEPI2 .58 .75 }

IDPT
TLPT .1 .01
SCPT 1 .18
BZ &concatcurve

LCOLOR black

XYDT .25 .965 /bBezier approximation to sine curves
XYDT .35 .21 Concatenated curve

TSTYLE italic
XYDT .06 .8 Cosine \pi/2
XYDT .8 .8 Sine \pi/2
XYDT .06 .6 Cosine \pi
XYDT .85 .6 Sine \pi
XYDT .8 .4 Sine 2\pi
XYDT .18 .4 Cosine 2\pi

