% Fig. 5 de docs/first_article.pdf (p. 13) — espectros de CO₂ calculados con el
% método de Smith (1969) a 0.5 µm de resolución. El histograma inferior es 261 cm
% de CO₂, el superior 522 cm; el área rayada entre ambos vale 0.374 µm.
%
% Primer ejemplo del corpus que ejercita `polybar` (§4.12) y `fill`-SIN-`outlinefill`.
%
% PROCEDENCIA. Esta figura es de 1988: la V0 imprimía directo a láser (no había EPS)
% y el PDF es un escaneo (JPEG gris a 300 dpi + capa OCR). No hay vectores que
% rescatar ni fórmula que recuperar — son la salida de un modelo de banda de Smith,
% no un 1-exp(-tau). Los píxeles son la fuente primaria, así que aquí SÍ se vale
% digitalizar (a diferencia de fig4-5, cuyas curvas resultaron analíticas). El
% método de digitalización y su inversión están en plan_polybar.md.
%
% VERIFICADOR (regalo de la leyenda): sum(a522 - a261)*0.5 debe dar 0.374 µm.
% Lo reconstruido da 0.3695 (-1.2%, dentro del grosor de línea de la cadena
% láser 1988 → papel → escáner 2008 → umbral). SI ESE NÚMERO SE MUEVE, ALGO SE ROMPIÓ.
%
% LA FIGURA SON TRES PASADAS, no una barra blanca encima de una rayada: eso habría
% borrado los costados verticales del rayado, que en el escaneo sí están.
%   1. 522 con trama, sin contorno  → rayado de 0 a a522
%   2. 261 en blanco opaco, sin contorno → borra el rayado bajo a261, costados incluidos
%   3. 522 solo contorno → repone costados de altura completa + tope en a522
% Cada pasada completa va antes de la siguiente (el z-order es el orden de escritura).
%
% Medidas tomadas del escaneo: caja de datos 12.46 × 8.0 cm, eje y en λ=11.5 con
% marcas adentro, eje x sin marcas (la tinta bajo 12..20 son cantos de barra, no
% marcas: en 22 no hay nada).

display_size 14.25 9.35
font_size 9.5
line_width 0.4

world_window -0.8 13.45 -0.4 8.95

% x = λ (µm, lineal), y = a_B (absortividad de banda, 0..1).
% El tope del eje y y la meseta 14.0-16.0 caen en el MISMO píxel del escaneo:
% esa meseta está saturada en 1.000 exacto para ambas concentraciones.
plot(x=(11.5,22.5), y=(0,1), box=(0,0, 12.46,8.0)) {

    % --- Pasada 1: 522 cm, trama sin contorno (hatch= enciende el relleno; sin
    % color= no hay outlinefill, así que rellena sin trazar).
    polybar(width=0.5, hatch=45, hatch_gap=1.4) {
        12.25 0.031   12.75 0.132   13.25 0.599   13.75 0.976
        14.25 1.000   14.75 1.000   15.25 1.000   15.75 1.000
        16.25 0.983   16.75 0.908   17.25 0.526   17.75 0.165
        18.25 0.062   18.75 0.033   19.25 0.020   19.75 0.007
    }

    % --- Pasada 2: 261 cm, blanco opaco sin contorno. Borra el rayado por debajo
    % de a261; NO deja línea en a261 (en el escaneo tampoco la hay).
    polybar(width=0.5, fill="white") {
        12.25 0.016   12.75 0.067   13.25 0.454   13.75 0.930
        14.25 1.000   14.75 1.000   15.25 1.000   15.75 1.000
        16.25 0.946   16.75 0.805   17.25 0.347   17.75 0.078
        18.25 0.030   18.75 0.015   19.25 0.008   19.75 0.007
    }

    % --- Pasada 3: 522 cm, solo contorno (polybar pelado = stroke sin fill).
    % MISMAS coords que la pasada 1 — si se editan unas hay que editar las otras
    % (y re-verificar el checksum de 0.374 de arriba).
    polybar(width=0.5) {
        12.25 0.031   12.75 0.132   13.25 0.599   13.75 0.976
        14.25 1.000   14.75 1.000   15.25 1.000   15.75 1.000
        16.25 0.983   16.75 0.908   17.25 0.526   17.75 0.165
        18.25 0.062   18.75 0.033   19.25 0.020   19.75 0.007
    }

    % Eje x: λ de 12 a 22 cada 2, SIN marcas (así está en el original).
    xaxis(step=2, start=12, ticks="none", label_gap=2)
    % Eje y: a_B de 0 a 1 cada 0.2, marcas adentro y cortas.
    yaxis(step=0.2, decimals=1, ticks="in", tick_size=1.5, label_gap=2)
}

% Rótulos de eje al EXTREMO (estilo del libro), en coords de mundo fuera del plot:
% title= los centraría. Posiciones tomadas del escaneo; λ(µm) comparte la línea
% base con los números del eje x.
text("$a_B$") { -0.62 8.75 }
text("$\lambda(\mu/rm)$") { 12.34 -0.28 }
