% markers.mg — Draft de los 6 marcadores estándar de MetaGráfica.
%
% Biblioteca candidata a embeberse en el binario (ver plan_marcadores.md §C.2).
% Por ahora se usa como cualquier biblioteca: `INPUT markers` antes de colocarlos.
%
% Convenciones de diseño:
%   - Definidos en caja simétrica [-1,1] CENTRADA en el origen, para que la
%     colocación (place / LNST / ARCST / DPST) apoye el CENTRO del marcador
%     sobre el vértice, y el escalado por width sea uniforme.
%   - NO fijan color: dibujan con el estado gráfico vigente (LCOLOR/FCOLOR) al
%     colocarse. Así heredan el color de la curva por default y pueden
%     recibir un color propio con marker_color/marker_fill (plan §B.4).
%   - "arrow" apunta en +x local: al rotar con la tangente queda alineado con
%     la dirección de la línea (punta de flecha).
%
% Nota V1→V3: el lexer V1 no admite guión bajo en identificadores, así que los
% nombres internos usan el prefijo capitalizado `Mk`. En V3 el usuario los
% invoca por cadena ("circle", "arrow"…) y el resolvedor mapea a estas structs.

% Círculo sólido
OPST MkCircle
FILL CR 1 : 0 0 }
CLST

% Cuadrado sólido
OPST MkSquare
FILL PL -1 -1  1 -1  1 1  -1 1  -1 -1 }
CLST

% Rombo sólido
OPST MkDiamond
FILL PL 0 -1  1 0  0 1  -1 0  0 -1 }
CLST

% Cruz (+), solo trazo
OPST MkCross
PL -1 0  1 0 }
PL 0 -1  0 1 }
CLST

% Equis (x), solo trazo
OPST MkX
PL -1 -1  1 1 }
PL -1 1  1 -1 }
CLST

% Punta de flecha triangular sólida, apuntando en +x
OPST MkArrow
FILL PL -1 -0.7  1 0  -1 0.7  -1 -0.7 }
CLST
