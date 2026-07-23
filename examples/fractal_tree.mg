% Árboles fractales — una estructura que se contiene a sí misma.
%
% Los dos árboles son la misma estructura de cuatro líneas —un tronco con dos
% copias menores de sí mismo en la punta— invocada con ángulos de rama distintos;
% cada uno son 511 segmentos. La condición de paro es un `if` corriente, y
% `max_depth` es la red de seguridad.
%
% NOTAS --------------------------------------------------------------------
% Reconstrucción de la Fig. 4 de: Alejandro Aguilar Sierra, «Metagrafic: hacia un
% lenguaje para la graficación por computadora», Ciencias 21, enero 1991,
% pp. 29-33 (docs/11195-10937-0-PB.pdf), a partir del listado impreso en su
% apéndice. Es la figura que demuestra la afirmación central del artículo:
% «Lo que hace realmente poderoso a MG es el uso de estructuras. […]
% Recursividad. Una estructura puede contenerse a sí misma, permitiendo
% sorprendentes efectos.»
%
% Aquel listado NO tenía condición de paro —no había condicionales en el lenguaje
% de 1991—, así que el límite de profundidad era lo ÚNICO que detenía el árbol:
% era infraestructura de carga, no una guarda defensiva.

% --- Procedencia y advertencia -----------------------------------------------
% El Apéndice 1 del artículo IMPRIME el listado V0 de esta figura, así que esto
% es un port y no una invención. El PDF es un ESCANEO sin capa de texto y el
% listado sale garabateado (O por 0, "AS" por "AF", "STST" por "SCST"), de modo
% que lo de abajo es una RECONSTRUCCIÓN, no una transcripción. Lo que el listado
% fija sin ambigüedad —y es todo lo que hace falta— es la geometría:
%
%   OPST AF            % abre la estructura
%   VAR THETA PHI      % ¡structs parametrizadas, en 1991!
%   PL 0 0 0 0.5 NL    % el tronco: un segmento de (0,0) a (0,0.5)
%   SCST 0.6 0.6       % las ramas van a 0.6 de la escala
%   RTST THETA         % ...giradas theta...
%   AF 0 0.5 NL        % ...y colocadas en la PUNTA del tronco
%   RTST PHI           % la segunda rama, con el OTRO ángulo
%   AF 0 0.5 NL
%   CLST
%
% El mapeo a V3 es casi 1:1 (VAR → parámetros de struct; SCST/RTST → scale y
% rotate como sentencias; la colocación por lista de puntos → translate).

% --- Lo que este archivo cubre en exclusiva ----------------------------------
% ⚠ Es el ÚNICO ejemplo del corpus donde una struct se invoca a sí misma
% (recursión). Verificado por barrido de examples/ y lib/ el
% 2026-07-22: cero recursión en todo el árbol antes de este archivo.
%
% 🔎 Y el listado de arriba es lo que motivó implementar `max_depth`: NO
% TIENE CONDICIÓN DE PARO. No podía tenerla: no había condicionales. El límite de profundidad era lo ÚNICO que detenía
% este árbol: infraestructura de carga, no una red de seguridad. De ahí el tope
% por default de 32 niveles, `max_depth n` para cambiarlo, y un error con mensaje
% —no un volcado— al pasarse.
%
% Aquí el paro es explícito, con `if`, que es el idioma V3 correcto: `max_depth`
% es la red, no el freno. Con 8 niveles la profundidad de anidamiento es 9, muy
% por debajo del tope; súbelo si quieres un árbol más profundo.

display_size 16 9

% El árbol crece hacia +y desde el origen. La ventana se fija a mano porque el
% alcance depende de los ángulos: con escala 0.6 la altura converge a lo más a
% 0.5/(1-0.6) = 1.25 (el caso degenerado de ramas verticales), y menos en cuanto
% se abren. Medido sobre la salida: el contenido mide 2.143 × 1.202 y queda con
% 0.130 / 0.126 de margen lateral.
%
% 2.4 × 1.35 es EXACTAMENTE el 16:9 del lienzo, así que el meet isométrico no
% letterboxea (0.00 pt por los cuatro lados) y la escala es 188.98 pt por unidad.
% ⚠ Vale la pena conservarlo así: en cuanto la ventana deja de ser 16:9, el meet
% ajusta al lado que sobra y ENCOGE el dibujo — ensancharla a 2.72 lo achicaba al
% 88.2% y metía 0.53 cm de banda arriba y abajo. Para dar más aire, mover
% los dos números de x por igual (o cambiar display_size), no uno solo.
world_window -1.2 1.2 -0.05 1.30

% --- La estructura recursiva -------------------------------------------------
% Un tronco vertical y, en su punta, DOS copias de sí misma: más chicas y con
% ángulos distintos. Todo el detalle del dibujo sale de eso.
%
%   theta, phi  ángulos de las dos ramas (grados; el signo abre a cada lado)
%   n           niveles que faltan por dibujar = la condición de paro
%   s           razón de escala entre un nivel y el siguiente
struct arbol(theta, phi, n, s) {
    polyline { 0 0  0 0.5 }            % el tronco

    if n > 0 {
        % Cada rama se arma en su propio bloque: `rotate` y `scale` son estado
        % con ámbito, así que la segunda rama no hereda el giro de la
        % primera. Componen en orden de escritura (T·S·R): el giro ocurre en la
        % base de la rama, la escala la encoge, y la traslación la lleva a la
        % punta del tronco.
        { translate 0 0.5   scale s   rotate theta
          arbol(theta, phi, n-1, s) }

        { translate 0 0.5   scale s   rotate phi
          arbol(theta, phi, n-1, s) }
    }
}

% --- Los dos árboles de la Fig. 4 --------------------------------------------
% El artículo los pone lado a lado, y el contraste es justo lo que se quiere
% mostrar: MISMA estructura, mismo número de niveles, misma escala — solo cambian
% dos números y el resultado no se parece.

% Los troncos NO están en ±x simétricas, y la razón es medible: el árbol derecho
% se recuesta, así que su copa no queda sobre su tronco (medido sobre la salida:
% ocupa 0.384 a la izquierda del tronco y 0.621 a la derecha, contra ±0.54 del
% simétrico). Las posiciones están puestas para que el CONJUNTO quede centrado,
% que es lo que se ve; centrar los troncos dejaría la figura cargada a un lado.

% Izquierdo: ángulos simétricos (theta = -phi) → copa equilibrada.
{ translate -0.53 0   arbol(28, -28, 8, 0.6) }

% Derecho: «ángulos distintos de rotación», como dice el texto del artículo.
% La asimetría se propaga a todas las escalas: el árbol se recuesta.
{ translate 0.45 0    arbol(15, -42, 8, 0.6) }
