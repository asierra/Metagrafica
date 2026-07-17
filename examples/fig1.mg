display_size 12 8

world_window 0 12 0 8

plot(x=(0,12), y=(0,10), box=(1.3,1.1, 11.5,7.5)) {

  line_width 0.6

  % Adem (1967) / Garduño and Adem (1988)
  bezier {
    0.0 10.0
    0.033 9.667 0.050 9.333 0.100 9.000
    0.150 8.667 0.217 8.333 0.300 8.000
    0.383 7.667 0.483 7.333 0.600 7.000
    0.717 6.667 0.867 6.333 1.000 6.000
    1.133 5.667 1.233 5.333 1.400 5.000
    1.567 4.667 1.750 4.333 2.000 4.000
    2.250 3.667 2.517 3.333 2.900 3.000
    3.283 2.667 3.667 2.333 4.300 2.000
    4.933 1.667 5.550 1.333 6.700 1.000
    7.850 0.667 9.700 0.333 11.200 0.000
    12.0 0
  }

  % Houghton (1971)/Holton (1979)
  marker(3, shape="circle-dot", color="black") {
    0.1 8.7
    0.4 7.3
    0.9 5.9
    1.9 4.4
    3.8 2.6
    6.4 1.6
    9.4 0.8
  11.6 0.4 }

  % Peixoto and Oort (1984)/Holton (1979)
  marker(size=4, shape="cross") {
    0.9 6.2
    1.8 4.8
    3.8 2.5
    6.5 1.5
    8.2 0.9
    9.2 0.1
  }

  legend(at="top-right", margin=10, sample_width=20, gap=5, font_size=8) {
    entry("Adem (1967)/Garduño and Adem (1988)") { polyline { 0 0.5  1 0.5 } }
    entry("Houghton (1971)/Holton (1979)") { marker(3, shape="circle-dot", color="black") { 0.5 0.5 } }
    entry("Peixoto and Oort (1984)/Holton (1979)") { marker(size=4, shape="cross") { 0.5 0.5 } }
  }

  xaxis(line_width=0.2, tick_label_font="italic",
  step=2, start=0, decimals=0,
  label="$-U_A$ ($10^-6$ cm)", label_font="roman", label_size=10)

  yaxis(line_width=0.2, tick_label_font="italic",
  step=1, decimals=0,
  label="z(km)", label_font="roman", label_size=10)
}
