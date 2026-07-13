display_size 3.5 2.5
font_size 8
%include "arrown.mg"
include "arco1v3.mg"
world_window -0.3333 4.3333 -0.48 2.02
line_width 0
dot(2) { 0.249979 -0.02  0.999989 -0.02  1.75 -0.02  2.500011 -0.02  3.250021 -0.02 }
world_window 0 3.5 -0.5 2
polyline { 0.75 -0.5  0.75 -0.2 }
polyline { 0.75 -0.35  1.5 -0.35 }
polyline { 1.5 -0.5  1.5 -0.2 }
polyline { 1.5 0  1.02 0.35 }
arc(1, from=53.13, to=90) { 0.75 0 }
line_width 0.2
place(Arrownr, scale=0.035, shift=0.9) { 0.75 0  2.25 2 }
place(Arrownr, scale=0.035, shift=0.9) { 1.5 0  3 2 }
place(Arrownr, scale=0.035, shift=0.2) { 0.75 2  0.75 0 }
place(Arrownr, scale=0.035, shift=0.2) { 1.5 2  1.5 0 }
polyline { 0.75 0  1.02 0.36 }
line_width 0
fit(ARCO1, stretch=true) { 0.86 0.38  2.2 0.2 }
text("/id") { 1 -0.32 }
text("/id ") { 2.1 0.4 }
text("$/rsin\phi$") { 2.28 0.4 }
text("$\phi$") { 1 1.1 }
