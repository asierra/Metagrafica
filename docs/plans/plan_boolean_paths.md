# Plan: path_boolean — operaciones booleanas y primitivas como generadores de trayectos

Motivado por la necesidad de construir figuras geométricas complejas, enmascarar contornos y generar regiones de interés precisas que serían imposibles o muy tediosas de trazar a mano o calcular analíticamente.

**Estado: EN PLANIFICACIÓN.** Este plan consolida las decisiones arquitectónicas para incorporar operaciones booleanas en MetaGráfica, eligiendo un modelo híbrido que atiende tanto a la generación de datos puros (álgebra funcional) como a la composición visual directa (`compound` extendido), además de un cambio en la evaluación de primitivas para que puedan comportarse como funciones generadoras de trayectos.

---

## Qué se va a implementar (Las tres patas)

### 1. Primitivas como generadores de trayectos
Desacoplar la semántica de "dibujar" de la de "generar geometría". Permitir que las primitivas cerradas devuelvan un valor de tipo `path` si se asignan, en lugar de emitir comandos de dibujado.

```octave
path eli = ellipse(2, 1) { 0 0 }    % No dibuja, solo guarda la geometría
path r = rectangle { -1 -1  1 1 }
```

### 2. Álgebra funcional (Nuevos generadores)
Nuevas funciones puras en la sección de Álgebra de trayectos que toman dos o más trayectos y devuelven uno nuevo con la geometría resultante.

*   `intersect(&a, &b)`: Retorna la geometría común ($A \cap B$).
*   `union(&a, &b)`: Retorna la silueta combinada ($A \cup B$).
*   `difference(&a, &b)`: Retorna el primer trayecto menos el segundo ($A \setminus B$).
*   `exclude(&a, &b)`: Retorna el área exclusiva (XOR).

### 3. El enfoque de renderizado (`compound` extendido)
Añadir el argumento `mode=` a la primitiva `compound` existente, permitiendo operaciones de recorte on-the-fly sin necesidad de declarar variables intermedias.

```octave
compound(fill="orange", mode="intersect") {
    circle(2) { 0 0 }
    circle(2) { 1.5 0 }
}
```

---

## Dónde vivirá el código y arquitectura

- **Backend Geométrico:** Se requiere integrar una biblioteca robusta de recorte de polígonos en C++ (como Clipper2 o Boost.Geometry) en el motor. Implementar algoritmos booleanos desde cero es propenso a errores (precisión de punto flotante, auto-intersecciones).
- **`src/path.cpp` / `src/primitives.cpp`:** Envoltura de la biblioteca externa para exponer las funciones `intersect`, `union`, etc., asegurando que los trayectos resultantes mantengan la convención interna de MetaGráfica (subtrayectos separados por `;`, orientaciones de contorno).
- **`src/evaluator.cpp` / `src/parser.cpp`:** Modificar la gramática y el evaluador para que una llamada a una primitiva (ej. `ellipse`) dentro de un contexto de asignación (`path p = ...`) devuelva la lista de vértices generada en lugar de despachar un `GraphicsItem` al canvas.
- **`src/graphics.cpp` (Compound):** Modificar la lógica de `GI_COMPOUND` para que, si `mode` no es el default, intercepte los paths de sus hijos, aplique la operación booleana sobre ellos secuencialmente, y emita un único path resultante al backend de salida (SVG/EPS/PDF).

---

## Qué NO está (El costo real y consideraciones)

1. ⚠️ **Béziers verdaderas (Curvas vs. Polígonos):** Las bibliotecas booleanas operan sobre polígonos (líneas rectas). Si hacemos operaciones booleanas sobre primitivas curvas (`circle`, `bezier`), el motor tendrá que **aplanarlas** (flattening) a polilíneas con una tolerancia de error antes de operar.
    - *Impacto:* El trayecto resultante de `intersect(&circulo1, &circulo2)` será una polilínea densa, no un conjunto de arcos exactos. Al exportar a SVG/EPS, esto generará más vértices en el archivo final. Esto es estándar en la mayoría de herramientas, pero debe documentarse.
2. ⚠️ **Manejo de estilos en trayectos mixtos:** Al usar `compound(mode="...")`, ¿qué color prevalece si los hijos declaran colores distintos?
    - *Decisión:* Igual que el `compound` actual, el estilo (fill/color) se dicta en la declaración de `compound` o en el estado global. Los estilos individuales de las primitivas internas se ignoran en el modo booleano.
3. 🔴 **Curvas abiertas vs cerradas:** Las operaciones booleanas requieren polígonos **cerrados**.
    - *Validación:* `intersect` de una polilínea abierta con un círculo debe lanzar un error claro de compilación, o bien cerrar automáticamente la polilínea (menos predecible). Se optará por requerir trayectos cerrados explícitamente.

---

## Costo estimado

| Pieza | Costo |
|---|---|
| Integración de lib C++ (ej. Clipper2) y build system | ~1 día |
| Primitivas como generadores (parser/eval) | ~2 días (requiere cuidado con la máquina de estados) |
| Aplanamiento de curvas (flattening con tolerancia) | ~1 día |
| Álgebra funcional (`intersect`, `union`, etc.) | ~1 día |
| Extensión de `compound(mode=...)` | ~1 día |
| **Total producción** | **~1 semana** |

---

## Propuesta para producción (Fases de ejecución)

1. **Paso 1 — Primitivas como datos:** Modificar el parser/evaluador para permitir `path p = circle(1) { 0 0 }`. Validar que `polyline(&p)` lo dibuja correctamente. Esto sienta la base: las primitivas ya pueden producir datos puros.
2. **Paso 2 — Motor Booleano Base:** Integrar Clipper2 (o similar) en el árbol de fuentes o dependencias. Construir la función interna C++ que recibe dos vectores de puntos y devuelve el vector booleano. Implementar la función de *flattening* para curvas.
3. **Paso 3 — Exposición en el lenguaje:** Conectar las funciones internas a la tabla de símbolos de MetaGráfica (`intersect`, `union`, `difference`, `exclude`). Crear un script de prueba exhaustivo.
4. **Paso 4 — Compound Visual:** Implementar el caso híbrido en `compound` reutilizando el motor del Paso 2.
5. **Paso 5 — Documentación:** Actualizar `referencia.md` en la sección 10 (Álgebra de trayectos) y 4 (Primitivas) explicando el aplanamiento de curvas.
