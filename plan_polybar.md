# Gráficos de Barras (Polybar)

Para facilitar la creación de histogramas y gráficas de series de datos, MetaGráfica V2 introduce la primitiva polybar, que de hecho se usó en la versión 0 en el primer paper publicado. 

Conceptualmente, polybar toma un path y dibuja un rectángulo vertical (una barra) en cada punto X, elevándose desde una base común hasta la altura Y especificada y con un ancho dado.

## 1. Sintaxis y Atributos

polybar(width=w) { x1 y1   x2 y2   x3 y3 ... }


width (obligatorio): Define el ancho geométrico de la barra en las unidades de la ventana mundial actual (world_window). La barra se dibuja centrada horizontalmente sobre la coordenada X.

(X, Y) en el bloque: Representan el centro superior de cada barra.

## 2. Superposición (Overlapping)

Para lograr el efecto clásico de barras superpuestas (como medir dos variables en el mismo mes, por ejemplo, precipitación teórica vs real), simplemente se declaran dos polybar iterando sobre las mismas coordenadas X, variando el width, el fill (relleno) o el dash (patrón de línea).

  ``` MG 
  % Ejemplo: Gráfica de barras superpuestas
  {
    line_width 1.5   % Todas las barras tendrán un borde de 1.5pt
    
    % Serie 1: Barras más anchas, fondo sólido gris
    polybar(width=0.8, fill="gray") { 
        1 40   2 55   3 30   4 70 
    }
    
    % Serie 2: Barras más angostas superpuestas, fondo rayado o negro
    polybar(width=0.4, fill="black") { 
        1 35   2 60   3 25   4 65 
    }
  }
  ```

## 3. Barras Horizontales (Atributo dir)

Si se requiere que las barras crezcan desde el eje Y hacia la derecha (histograma horizontal), se puede agregar un atributo de dirección dir="horizontal". En este caso, la coordenada X del bloque actúa como la longitud de la barra, y la Y como su posición vertical.

% Barras horizontales (Crecen en X, apiladas en Y)
polybar(width=0.6, dir="horizontal") {
    40 1    55 2    30 3    70 4
}


Impacto en la Implementación (C++ AST)

Implementar polybar es extremadamente elegante porque no requiere modificar los backends (ni SVG, ni EPS, ni PDF).

El nodo PolybarNode en el Árbol de Sintaxis Abstracta (AST) actúa como una "macro de expansión". Al llamar a su método evaluate(), simplemente itera sobre su lista de puntos (X, Y) y, matemáticamente, genera y dibuja internamente un rectángulo para cada uno usando la primitiva rect() que ya existe en el motor base:
