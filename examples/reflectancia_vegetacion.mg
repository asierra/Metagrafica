% Reflectancia espectral: caducifolias contra coníferas
%
% Dos bandas ("envolventes") de reflectancia contra longitud de onda, 0.4 a
% 0.9 micras: una para árboles caducifolios (arce) y otra para coníferas
% (pino). Cada banda es un rango, no una línea, porque la reflectancia varía
% de un árbol a otro dentro de la misma especie. Enseña la razón de ser del
% infrarrojo cercano en teledetección forestal: en todo el visible las dos
% bandas se traslapan casi por completo —el ojo ve el mismo verde en ambas—,
% pero a partir de 0.7 µm se separan y quedan muy lejos una de otra, con las
% caducifolias reflejando bastante más que las coníferas.
%
% NOTAS ---------------------------------------------------------------------
% ORIGEN: reconstrucción de la Fig. 1.7 de Lillesand, Kiefer y Chipman (2015),
% «Remote Sensing and Image Interpretation», 7ª ed., p. 14 —a su vez adaptada
% por esos autores de Kalensky y Wilson (1975)—. No es un facsímil: los
% rótulos van en español y el remate derecho de cada banda, que en el original
% es un gancho decorativo de dibujo a mano, aquí se simplifica a un cierre
% recto (mismo argumento que un escaneo con medios tonos: se puede quitar lo
% que el libro arrastra por su época de dibujo, no lo que afirma).
%
% LOS VALORES SON APROXIMADOS, leídos a ojo sobre la curva publicada, no un
% dataset digitalizado: el propio Lillesand llama a esta figura «highly
% generalized» y no da una tabla. Lo que importa y sí se respeta con cuidado
% es la FORMA que sostiene el argumento del texto: pico verde chico (~3-4%)
% igual de bajo en las dos especies, valle rojo casi en cero hacia 0.68-0.70
% µm, y el borde rojo (red edge) subiendo en el mismo tramo 0.70-0.76 µm hasta
% mesetas separadas —caducifolias ~38-44%, coníferas ~24-30%— que ya no se
% tocan. Cambiar esos números en los `path` de abajo es todo lo que hace falta
% para ajustar la figura si aparece una fuente con datos reales.
%
% LA BANDA DE CADUCIFOLIAS SE CONSTRUYE COMO UN POLÍGONO CERRADO (la de
% coníferas no, ver abajo): `smooth` interpola una curva por los nodos leídos,
% pero lo que hace falta rellenar aquí no es una curva sino el ÁREA entre dos
% curvas. La salida se pasa por `sample(..., curve=true)`, que sí devuelve
% vértices literales; sin ese paso intermedio `polygon` habría leído los puntos
% de control de Bézier como vértices y el relleno habría salido con esquinas
% rectas. El cierre del trayecto lleva su propia trampa, documentada junto al
% `concat` de abajo: SUELDA, no yuxtapone.
%
% LA CAJA DE DATOS SE EXTIENDE POR DEBAJO DE 0%, hasta `ymin`, para tener
% dónde poner el renglón de bandas espectrales (Azul/Verde/Rojo/Infrarrojo
% cercano) y el nombre del eje x, exactamente como hace examples/fig4-4.mg
% con `xaxis(base=0, ...)`: el eje se dibuja en el valor de dato y=0, no en
% el borde de la caja, y todo lo que cae por debajo es margen para rótulos,
% no una escala que signifique algo.
%
% ⚠️ COBERTURA EXCLUSIVA: es el único ejemplo con `brace`, la llave extensible,
% y el único que ejercita el ESCAPE del marcado (`\{`, `\}`). Si sale del corpus,
% las dos se quedan sin una sola prueba de que dibujen lo que dicen.
%
% LA LLAVE DEL MARGEN DERECHO se dibuja tres veces en el original, y aquí también,
% por dos caminos distintos que conviene no confundir:
%   (1) la llave GRANDE que abarca de 44% (techo de caducifolias) a 24.5% (piso de
%       coníferas) es MOBILIARIO DE PÁGINA: cae fuera del ancho de datos
%       (lam1=0.90=bx1), así que se coloca FUERA del `plot` y su vano se calcula
%       aquí con la misma fórmula lineal que el `plot` usa por dentro (`pv_top`/
%       `pv_bot` abajo). Su `depth` va en PUNTOS —es tipografía, no un dato—, así
%       que la llave crece con la ventana y su gancho no.
%   (2) las DOS llaves del rótulo son literales del texto: en el original el
%       letrero dice «{Note range of spectral values}», con llaves de imprenta.
%       Se escriben `\{` y `\}`, el escape del marcado. Hasta el 2026-08-06 no
%       había forma de escribirlas —`{` y `}` son agrupadores y se los comía el
%       parser en silencio—, y por eso este rótulo llevaba paréntesis.
%
% El vano de (1) NO se puso a ojo: 44.0 es el último nodo de `Dt_s` y 24.5 el del
% tramo alto de `Cb_s`, o sea que la llave abarca exactamente lo que abarcan las
% dos bandas en su extremo derecho. Si se editan esos nodos, la llave los sigue.

display_size 18 10.5
world_window 0 18 0 10.5
font "roman"

% ── Curvas superior/inferior de cada banda, leídas sobre la Fig. 1.7 ────────
% (longitud de onda en µm, reflectancia en %)
path Dt_s = smooth {
    0.40 1.0   0.44 1.6   0.48 2.6   0.52 3.8   0.55 4.4   0.58 3.9
    0.62 2.8   0.66 1.8   0.69 1.4   0.70 1.6   0.71 3.0   0.72 8.0
    0.73 16.0  0.74 25.0  0.75 32.0  0.76 36.0  0.78 38.5  0.80 38.0
    0.82 39.5  0.84 41.5  0.86 43.5  0.88 44.0  0.90 42.5
}
path Db_s = smooth {
    0.40 0.4   0.44 0.9   0.48 1.7   0.52 2.6   0.55 3.0   0.58 2.7
    0.62 2.0   0.66 1.3   0.69 1.0   0.70 1.1   0.71 2.0   0.72 5.5
    0.73 11.0  0.74 18.0  0.75 25.0  0.76 30.0  0.78 34.0  0.80 33.5
    0.82 35.0  0.84 37.0  0.86 38.5  0.88 39.0  0.90 37.5
}
path Ct_s = smooth {
    0.40 0.8   0.44 1.3   0.48 2.2   0.52 3.2   0.55 3.8   0.58 3.3
    0.62 2.3   0.66 1.5   0.69 1.2   0.70 1.4   0.71 2.4   0.72 5.5
    0.73 10.5  0.74 15.5  0.75 19.5  0.76 22.0  0.78 23.8  0.80 25.2
    0.82 26.0  0.84 27.5  0.86 29.0  0.88 30.0  0.90 28.5
}
path Cb_s = smooth {
    0.40 0.3   0.44 0.7   0.48 1.5   0.52 2.2   0.55 2.5   0.58 2.2
    0.62 1.6   0.66 1.0   0.69 0.8   0.70 0.9   0.71 1.6   0.72 3.8
    0.73 7.5   0.74 11.0  0.75 14.5  0.76 17.0  0.78 20.5  0.80 21.0
    0.82 22.5  0.84 23.5  0.86 24.5  0.88 25.5  0.90 24.5
}

n_muestra = 60
path Dt = sample(&Dt_s, n_muestra, curve=true)
path Db = sample(&Db_s, n_muestra, curve=true)
path Ct = sample(&Ct_s, n_muestra, curve=true)
path Cb = sample(&Cb_s, n_muestra, curve=true)

% Banda de caducifolias, cerrada. El segmento de cierre del extremo derecho NO es
% decorativo: `concat` SUELDA (§9), o sea traslada cada pieza para que arranque
% donde acabó la anterior, así que `concat(&Dt, reverse(&Db))` a secas sube la
% curva inferior entera por la diferencia entre los dos extremos que une —aquí
% 42.5 − 37.5 = 5 puntos porcentuales—. Compila limpio, no avisa nada y la banda
% sale plausible pero falsa: gorda en el visible, que es justo donde esta figura
% afirma que las dos especies se traslapan. Interponiendo el tramo vertical que ya
% une los dos extremos, cada soldadura es la identidad y el trayecto queda absoluto.
path Dcierre = { 0.90 42.5   0.90 37.5 }   % = último nodo de Dt_s → último de Db_s
path Dring = concat(&Dt, &Dcierre, reverse(&Db))

% ── La caja de datos: y baja de 0 para dejar sitio a los rótulos de abajo ───
lam0 = 0.40
lam1 = 0.90
ymin = -22
ymax = 50
bx0 = 3.0
bx1 = 14.5
by0 = 1.0
by1 = 9.6

plot(x=(lam0,lam1), y=(ymin,ymax), box=(bx0,by0, bx1,by1)) {

    % Divisores de banda espectral, solo en la zona de datos (0-50%).
    line_width 0.3
    color gray(0.45)
    polyline { 0.50 0  0.50 50 }
    polyline { 0.60 0  0.60 50 }
    polyline { 0.70 0  0.70 50 }
    polyline { 0.80 0  0.80 50 }
    color "black"

    % Coníferas primero, caducifolias encima — mismo orden que el original.
    % La banda de coníferas va sin relleno, como en el original (banda blanca,
    % contorno punteado), y por eso son dos curvas sueltas. Que no sea un
    % `polygon(fill="none")` no es una limitación sino la definición: un polígono
    % rellena —es lo que lo distingue de una polilínea cerrada—, y desde el
    % 2026-08-06 el compilador rechaza esa combinación en vez de aceptarla en
    % silencio y salir relleno de negro. Si algún día se quiere la banda cerrada
    % por sus extremos, la forma es `polyline(closed=true)`, no `polygon`.
    dash "dashed"
    line_width 0.5
    polyline(&Ct)
    polyline(&Cb)
    dash "solid"

    polygon(&Dring, fill=gray(0.87), color="black", line_width=0.6)

    % Rótulos de las bandas, con flecha guía hacia dentro de cada banda ya en
    % la meseta del infrarrojo cercano (0.75-0.76), donde el trazo no es tan
    % delgado como justo en el borde rojo.
    font_size 9
    text("Caducifolias/n(arce)", align="center") { 0.545 41 }
    polyline(marker_end="arrow", line_width=0.3) { 0.585 38  0.75 29 }

    text("Coníferas/n(pino)", align="center") { 0.475 19 }
    polyline(marker_end="arrow", line_width=0.3) { 0.50 15.5  0.755 20 }

    % Renglón de bandas espectrales, debajo del eje (y=0 es el eje mismo).
    font_size 8
    text("Azul",  align="center", valign="top") { 0.45 -8.5 }
    text("Verde", align="center", valign="top") { 0.55 -8.5 }
    text("Rojo",  align="center", valign="top") { 0.65 -8.5 }
    text("Infrarrojo cercano", align="center", valign="top") { 0.80 -8.5 }

    % Nombre del eje x, más abajo que el renglón de bandas.
    font_size 9
    text("Longitud de onda ($\mu$m)", align="center", valign="top") { 0.65 -16.7 }

    xaxis(step=0.1, decimals=1, base=0, ticks="out", tick_label_gap=3)
    yaxis(step=10, start=0, decimals=0, label="Reflectancia (%)", label_gap=24)
}

% ── Corchete del margen derecho: rango de valores espectrales ──────────────
% Mismo mapeo lineal que usa `plot` por dentro, pero calculado aquí porque es
% mobiliario de página (cae fuera del ancho de datos, lam1=0.90=bx1).
pv_top = by0 + (44.0 - ymin)/(ymax - ymin)*(by1 - by0)     % techo de caducifolias
pv_bot = by0 + (24.5 - ymin)/(ymax - ymin)*(by1 - by0)     % piso de coníferas

% La llave abre a la DERECHA, o sea que su cúspide apunta al rótulo. El costado
% sale del ORDEN de los dos puntos (arriba→abajo aquí), no de una bandera: la
% cúspide cae 90° a la izquierda de la dirección del vano.
line_width 0.4
brace(depth=7) { 14.9 pv_top   14.9 pv_bot }
font_size 8
text("\{nótese el/nrango de valores/nespectrales\}", align="left", valign="middle") { 15.5 ((pv_top+pv_bot)/2) }
