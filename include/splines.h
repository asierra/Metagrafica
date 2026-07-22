/*
       File:  splines.h
              Implementation of Catmull-Rom splines and conversion to Bezier.
MetaGrafica:  Human descriptive language to generate publication quality
              Display in PostScript.
Copyright (c) 1988-2026 Alejandro Aguilar Sierra (algsierra@gmail.com)
    Version:  2026
Antecedents: Version 0.0 1988 Pascal and Assembler, first published paper. 
			 Version 1.0 1991 C, first published book.
			 Version 2.0 1999-2024 C++ STL, EPS only, three published books. 
			 
 This file is part of MetaGrafica.
 Licensed under the GNU General Public License v3.0 (see LICENSE file).
*/
#if !defined(MG_SPLINES_H)
#define MG_SPLINES_H

#include "primitives.h"
#include <vector>

/**
   SOLO V1 (fuera del build). El comando `spline` se RETIRÓ de V3 el 2026-07-20
   (spec §9.1): era la misma curva que `smooth` —Catmull-Rom pasa por sus puntos
   de control— pero descartando los extremos en vez de recuperarlos por reflexión.
   Las dos funciones de abajo solo las llama `src/Parser.cpp` (front-end V1); en
   V3 la interpolación suave entra por path_to_bezier().

   splines(): muestrea la curva a `intervals` nodos por segmento. Muestrear
   produce DATOS, no tinta; si V3 lo necesita algún día será como `sample(&p, n)`
   del álgebra §9, no como un modo de dibujo.
 */
Path splines(Path controlpoints, int intervals);

/**
   SOLO V1 (ver arriba). Catmull-Rom UNIFORME → controles Bézier.
 */
Path splines_to_bezier(Path controlpoints);

Path process_path(Matrix matrix, Path path);

// Ops unarias del álgebra de paths §9 (reflexiones respecto al origen; la
// posición absoluta se reabsorbe en concat_paths y en fit):
//   transpose: (x,y)→(y,x)  reflexión en y=x (intercambia ejes de datos)
//   flip_x:    (x,y)→(−x,y)  voltea en x (izquierda↔derecha; espeja en el eje y)
//   flip_y:    (x,y)→(x,−y)  voltea en y (arriba↔abajo; espeja en el eje x)
Path transpose_path(const Path &p);
Path flip_x_path(const Path &p);
Path flip_y_path(const Path &p);

// reverse(p): el mismo path recorrido al revés (§9). Con concat sin
// auto-reversión, es la forma explícita de orientar un segmento antes de
// soldarlo (concat pega el INICIO de cada operando al final del anterior).
inline Path reverse_path(const Path &p) { return Path(p.rbegin(), p.rend()); }

// concat_paths(a, b): suelda dos paths en uno continuo (álgebra de paths §9).
// Traslada b para que su INICIO continúe desde el FINAL de a, sin invertir
// ningún operando (spec §9: sin auto-reversión; el autor orienta con reverse()).
// Operación gráfica sobre los datos, no de parseo.
Path concat_paths(const Path &a, const Path &b);

void normalize_path(Path& path);

/**
  From four control points, compute the corresponding bezier tangent vectors for
  points p1 and p2 and so, create a bezier segment.
 */
void get_bezier_tangents(point p0, point p1, point p2, point p3, point &t1, point &t2);

/** 
   From a path of control points, creates a set of Bezier control points, 
   ready to be used in PostScript.
 */
Path path_to_bezier(Path controlpoints);

/**
   Extensión en x del path al nivel y dado: xmin/xmax de sus cruces con la
   recta horizontal y = y_level. Devuelve false (y NO toca xmin/xmax) si el
   path no la cruza — el llamador debe verificar el retorno antes de leerlos.

   Segundo miembro de la familia de reducciones path→número, junto a
   path_width (ver `plan_struct_params.md`). Hereda su advertencia: opera
   sobre el polígono de los puntos del path. Sobre paths monótonos en x
   (las ondas de fig16-9) es exacto; sobre una bezier genuinamente curva el
   polígono de control puede cruzar y_level en x distintos —y en distinto
   número de veces— que la curva.
 */
bool path_x_bounds_at_y(const Path &path, double y_level, double &xmin, double &xmax);

/**
   Familia de muestreo sobre un path (§9), con el modelo α+β decidido 2026-07-21:
   el path-valor es NEUTRO (la misma lista de puntos se dibuja recta con polyline o
   curva con bezier; la interpretación la pone la primitiva), así que estas funciones
   la reciben en el flag `curve`:

     curve=false — los puntos son VÉRTICES. Interp lineal entre ellos; longitud de
                   arco = suma de segmentos. Exacto para una polilínea.
     curve=true  — los puntos son CONTROLES bézier (3k+1): p0 c1 c2 p1 [c1 c2 p2 …].
                   Se evalúa la cúbica de Bernstein. Toca la CURVA, no la envolvente.

   El parámetro global `t∈[0,1]` recorre el path por LONGITUD DE ARCO (t=0 inicio,
   t=1 final), así que `t=0.5` es el medio GEOMÉTRICO — no la mitad de los segmentos
   (medido: en segmentos desiguales difieren hasta 2.9 unidades, spike 2026-07-21).
 */

// Evalúa UNA cúbica de Bézier (control p0,c1,c2,p1) en t∈[0,1].
point bezier_point(point p0, point c1, point c2, point p1, double t);

// El punto en t∈[0,1] a lo largo del path, por longitud de arco. curve: ver arriba.
point path_point(const Path &path, double t, bool curve);

// n puntos equiespaciados por longitud de arco (t=0 y t=1 incluidos; n>=2).
Path path_sample(const Path &path, int n, bool curve);

// Ángulo (grados) de la tangente en t∈[0,1]. curve: ver arriba.
double path_angle(const Path &path, double t, bool curve);

#endif
