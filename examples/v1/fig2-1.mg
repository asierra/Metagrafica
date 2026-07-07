% Fig. 1.1 de IMQ, 3a. ed., sept. 1998
% Modelo de cuerpo negro

% Puesto que ya no usamos toda la hoja, no es preciso indicar la
% posición, de hecho $O es ignorada.
% $O 5.3 5.0
$D 3 3

INPUT arrow

OPST FG1
%LSTYLE 0 Ahora el comando para definir el ancho de línea es más claro.
LWIDTH 0
PL
-1.11111 -0.046
0.7071  0.7071
-0.0000 -1.0000
-0.7071  0.7071
1.0000 -0.0000
-0.7071 -0.7071
0.0000  0.8000
}

% Con Line Structure (LNST) podemos colocar flechas o cualquier
% estructura, en el ángulo correcto y no se deforman en las rotaciones.
% No hace falta calcular el ángulo, este se calcula automáticamente
% con la pendiente de la línea, cuya longitud es arbitraria. 
% Los parámetros son la escala y los dos puntos extremos de la línea.
% El marcador se coloca en el segundo punto.
% La estructura a usar se indica como marcador con MKST.
MKST arrowr
LNST 0.04 : -1.11111 -0.046 -.93 .03
LNST 0.04 : -0.7071 -0.7071 0.0000  0.8000 

% Y así es más sencillo que lo que está comentado enseguida
% y no hace falta más de una estructura.
%SCST 0.05 0.05
%RTST 15
%arrowr -.93 .03 }
%RTST -15
%RTST -37
%arrowu 0 0.8 }

LWIDTH 2
FILL
% Con FPATRN se hace un relleno achurado. Un negativo indica que se
% trace el contorno.
FPATRN -8
% O el relleno puede ser sólido con un nivel de gris (100 es blanco)
%FGRAY 50

% El nuevo comando OPPT y su correspondiente CLPT permiten abrir
% rutas de modo que la polilínea de abajo se conecte con el círculo
% abierto de abajo sin necesidad de conectarlo manualmente y así 
% podemos trazar rutas (paths) no triviales. Noten que no hay una 
% línea explícita que los una, al cerrar el path se unirán solas.
% Por supuesto es importante el orden para que quede claro cuales
% puntos hay que unir.
% El sufijo PT se refiere a path como el ST se refiere a estructura.
OPPT
PL -1 .1736  -1 1.015  1.015 1.015  1.015 -1.015  -1 -1.015  -1 -.1736 }
% El arco se define por el radio, el ángulo total y el ángulo inicial
CR 1 340 190 : 0 0 }
CLPT
CLST

WW 0.0 1.03 0.08 .92
SCST 0.46 0.46
FG1 .53 .5 }

