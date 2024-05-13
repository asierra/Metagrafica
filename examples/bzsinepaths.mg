% Sine curves to be approximated with bezier curves
% Though it is possible to approximate a whole 2pi
% period with a simple bezier segment, it is recommended
% to use as much as pi, half period.
% Usage examples in bzsinepaths-examples.mg
%
% Author: Alejandro Aguilar Sierra, algsierra@gmail.com
% Version 1.0, April 2024

% Horizontal scale 0, 1 equivalent to 0, pi/2
% Vertical scale 0, 1
&sinpi2 0 0  0.417 0.6667  0.693 1  1 1 }

% Horizontal scale 0 1 equivalent to 0, pi
% Vertical scale 0, 1
&sinpi 0 0  0.41 1.335  .59 1.335  1 0 }

% Horizontal scale 0, 1 equivalent to 0, 2pi
% Vertical scale 0, 1 equivalent to -1, 1
&sin2pi 0.0 0.5  0.205 1.1675  0.295 1.1675  0.5 0.5  
0.705 -0.1675  0.795 -0.1675 1.0 0.5 }

&cospi2 0 1  .307 1   0.583 0.6667  1 0 }

&cospi 0 1  0.365 1  0.635 0  1 0 }

&cos2pi 0.0 1.0  0.1825 1.0  0.3175 0.0  0.5 0.0  
0.6825 0.0  0.8175 1.0  1.0 1.0 }

% Just for convenience, a simple straight line
&straightline 0 0  .3333 0  .6666 0  1 0 }

