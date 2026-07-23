% Vidas medias de decaimiento alfa — la ley de Geiger-Nuttall.
%
% Eje y LOGARÍTMICO: los datos van en unidades reales y el plot los mapea a su
% caja; los ejes heredan los rangos y se rotulan solos, sin un solo rótulo de
% potencia de diez escrito a mano. Los isótopos son texto anclado a sus datos.
%
% NOTAS --------------------------------------------------------------------
% Fig. 6.4 de «Introducción a la mecánica cuántica», L. de la Peña, 3a. ed.
% Los datos van en unidades reales (E^{-1/2} en MeV^{-1/2}, λ⁻¹ en s),
% convertidos de los píxeles digitalizados del escaneo.
%
% Layout del libro: el eje x cae ABAJO de la primera marca (10^{-15}), con un
% hueco. Se reproduce con la caja de fondo al nivel del eje x y el rango y
% extendido a 1e-20; yaxis(start=1e-15) rotula desde ahí hacia arriba.
%
% ⚠ COBERTURA EXCLUSIVA: es el único ejemplo con texto anclado a datos DENTRO de
% un plot logarítmico, o sea el único que ejercita el remapeo puntual del ancla.
% Sin él, un bug ahí pasaría todas las compuertas sin que nadie lo note. También
% es el único sin `font` explícito, y por eso el que caza la cara ambiente en PDF.

display_size 9 6.7
font_size 8

% Lienzo exterior (world coords) con margen para el nombre del eje y y los isótopos.
world_window -1 7.2 -0.95 6.25

% x = E^{-1/2} (0.30..0.50, lineal) ; y = λ⁻¹ (log). box=(0,0,6,5) = región de datos
% en world coords; la anisotropía y el mapeo log los hace plot.
plot(x=(0.30,0.50), y=(1e-20,1e5), yscale="log", box=(0,0, 6,5)) {

    % recta de ajuste (Geiger-Nuttall), EN DATOS
    line_width 0.8
    polyline { 0.320 1.0e7  0.493 3.2e-17 }

    % puntos experimentales, EN DATOS (dot = marcador físico: posición mapeada,
    % radio inmune al mapper → discos redondos en su sitio)
    fill "black"
    dot(1.5) { 0.324 9.3e5  0.349 1.9e3  0.358 1.8e2  0.373 4.1e0
               0.360 2.4e-1  0.376 5.7e-2  0.383 6.7e-3  0.395 5.3e-3
               0.398 9.4e-5  0.409 2.0e-3  0.411 2.7e-6  0.416 3.4e-6
               0.406 6.4e-7  0.411 6.4e-7  0.422 1.4e-8  0.438 2.7e-9
               0.447 3.0e-11  0.441 1.1e-12  0.450 4.2e-13  0.454 1.7e-12
               0.442 9.4e-15  0.480 6.9e-16  0.482 5.1e-17  0.490 3.2e-17 }

    % Isótopos que abren y cierran la serie: anotaciones EN DATOS, junto a su punto
    % (Po: 0.324 9.3e5 ; U: 0.490 3.2e-17). Van DENTRO del plot para que el ancla la
    % mapee él —incluso en el eje log, donde el mapeo es puntual— en vez de fijarla a
    % mano en coords de ventana: mover un rango o la caja las reacomoda solas.
    text("$Po{^292}$") { 0.3367 3.16e5 }
    text("$ U{^238}$") { 0.4967 1.0e-17 }

    % Eje x: E^{-1/2} lineal, .30..50 cada .05, marcas adentro, sobrante de línea.
    xaxis(line_width=0.2, step=0.05, decimals=2, strip_zero=true, ticks="in",
          extend=0.4, label="$/iE^{/r-1/2} (MeV)^{/r-1/2} $")
    % Eje y: λ⁻¹ log; rótulos 10^n desde 10^{-15} (start), 5 décadas por marca.
    yaxis(line_width=0.2, start=1e-15, step=5, ticks="in", extend=(0, 0.3))
}

% Nombre del eje y: mobiliario de página, NO una anotación de datos — va horizontal
% y arriba (estilo del libro), y `label=` lo rotaría a lo largo del eje. Se queda en
% coords de ventana, fuera del plot.
text("$\lambda^{-1} (/rs)$") { -0.83 5.5 }
