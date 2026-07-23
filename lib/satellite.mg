% Panel solar: base azul + rejilla RECTA azul oscuro. hatch_angle=0 endereza el
% crosshatch (por defecto sale a 45°). El color de las líneas sale de fill=, no de
% color= (la trama ES el relleno; color= contornearía el borde).
struct SolarPanel(w, h) {
  rectangle(fill="blue") { 0 0 w h }
  rectangle(fill="darkblue", hatch="crosshatch", hatch_angle=0, hatch_gap=10) { 0 0 w h }
}

struct Satellite() {
  % antenna
  arc(.3, from=190, to=350, fill="black") { 0 .6 }

  % mast
  rectangle(fill="gray") { -0.03 0.25  0.03 0.3 }

  % body
  rectangle(fill="gray") { -0.15 -0.5  0.15 0.25 }

  % solar panels
  SolarPanel(.7, .3, at=(.2, -0.275))
  SolarPanel(.7, .3, at=(-0.9, -0.275))
}
