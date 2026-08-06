/*
       File:  brace.h
              Geometría física de la llave extensible: única fuente de verdad,
              consultada por los tres backends. Mismo papel que markers.h, y por
              la misma razón — una llave que cada backend reconstruyera a su
              manera es la familia de bugs de plan_anisotropia.md.
MetaGrafica:  Human descriptive language to generate publication quality
              Display in PostScript.
Copyright (c) 1988-2026 Alejandro Aguilar Sierra (algsierra@gmail.com)
 This file is part of MetaGrafica.
 Licensed under the GNU General Public License v3.0 (see LICENSE file).
*/
#if !defined(MG_BRACE_H)
#define MG_BRACE_H

#include <algorithm>
#include <cmath>
#include <vector>

#include "primitives.h"

// ---------------------------------------------------------------------------
// POR QUÉ SE DIBUJA Y NO SALE DE LA FUENTE (plan_llaves.md §3)
//
// El subset de Latin Modern Math no trae las piezas extensibles (U+23A7…23AB) ni
// la tabla `MATH` que declara cómo ensamblarlas, así que la fuente daría el ARTE
// y no el COMPORTAMIENTO: el ensamblado —cuántos extensores, cuánto solape— hay
// que escribirlo igual. Es la doctrina ya escrita de la casa para `\hat`/`\vec`
// en text.h, y el mismo criterio por el que la raya de `\frac` es un trazo.
//
// Y el argumento que decide: ⚠️ **una llave alta y delgada NO es una llave
// escalada.** Es una llave con el vástago más largo y los ganchos DEL MISMO
// TAMAÑO. Estirar un glifo, o estirar una struct de `lib/` con `fit(stretch)`,
// deforma los ganchos. Por eso la geometría se calcula con la longitud del vano
// y la profundidad como cantidades INDEPENDIENTES, y por eso vive aquí y no en
// una caja unitaria al estilo de MarkerShape: una caja unitaria escalada es
// justo lo que no se puede hacer.
//
// FORMA: cuatro cuartos de círculo de radio R y dos rectas. Los dos extremos caen
// EXACTOS sobre los puntos del vano; la cúspide sobresale `depth` hacia el
// costado. El vástago corre a media profundidad, que es la proporción clásica.
//
// ⚠️ **CADA BRAZO ES UNA S, NO UNA C: el gancho del extremo curva al REVÉS que el
// arco de la cúspide.** Es la forma que tiene la llave de LaTeX, y no es un gusto:
// se llegó a ella corrigiendo la versión ingenua —los cuatro cuartos curvando en
// el mismo sentido—, que además de no parecer una llave dejaba un CODO DE 90° en
// cada unión con el vástago, porque el gancho llegaba con tangente horizontal a un
// vástago vertical. Con la curvatura invertida las cuatro uniones son tangentes y
// el único quiebre de toda la llave es la cúspide, que es donde debe estar.
//
// Consecuencia útil: los dos extremos tienen tangente HORIZONTAL, o sea que las
// puntas salen apuntando hacia lo que la llave abarca. Es lo que hace el original
// de Lillesand, cuyos extremos rematan en sendos trazos horizontales que van a
// tocar las dos bandas.
// ---------------------------------------------------------------------------

// Un tramo de la llave. `curve` falso = recta hasta `to` (c1/c2 sin usar).
struct BraceSeg {
  bool  curve;
  point c1, c2, to;
};

// Cuarto de círculo como cúbica: los puntos de control van a K·R sobre las
// tangentes. Es la constante de siempre, 4/3·(√2−1).
inline double braceKappa() { return 0.55228474983079339; }

// Radio de gancho EFECTIVO para un vano dado. La llave necesita 2R por debajo de
// la cúspide y 2R por encima, así que un vano corto obliga a achicar el gancho.
//
// ⚠️ Se achica R, o sea que se achica TAMBIÉN la profundidad (la cúspide está a
// 2R del vano). Es deliberado: mantener la profundidad y bajar solo el radio
// vertical convertiría los cuartos de círculo en cuartos de ELIPSE y la llave se
// deformaría — que es exactamente lo que este archivo existe para evitar. Una
// llave muy corta es poco profunda; eso es honesto, y es lo que hace un tipógrafo.
inline double braceRadius(double span, double depth, double tip) {
  double yc = tip * span;
  double r  = std::fabs(depth) / 2.0;
  r = std::min(r, yc / 2.0);
  r = std::min(r, (span - yc) / 2.0);
  return std::max(r, 0.0);
}

// Tramos de la llave en un marco LOCAL en unidades de dispositivo (pt):
// el vano va de (0,0) a (0,span) sobre el eje +y, y la cúspide sale hacia −x.
// El consumidor rota y traslada ese marco a los dos puntos ya transformados.
//
// Quien llama decide el COSTADO orientando el segmento: −x local es el vano
// girado 90° en sentido antihorario, así que intercambiar los dos puntos voltea
// la llave. No hay bandera de lado, y no la hay a propósito — sería un segundo
// modo de decir lo mismo, que se puede contradecir con el primero.
inline std::vector<BraceSeg> braceSegments(double span, double depth, double tip) {
  std::vector<BraceSeg> segs;
  double R = braceRadius(span, depth, tip);
  if (span <= 0 || R <= 0)
    return segs;                       // degenerada: no se dibuja nada
  double K  = braceKappa() * R;
  double yc = tip * span;

  // (1) del extremo inferior al vástago — curvatura INVERSA a la del gancho (3)
  segs.push_back({ true,  point(-K, 0),         point(-R, R - K),    point(-R, R) });
  // (2) vástago inferior
  segs.push_back({ false, point(0, 0),          point(0, 0),         point(-R, yc - R) });
  // (3) del vástago a la cúspide
  segs.push_back({ true,  point(-R, yc - R + K), point(-2*R + K, yc), point(-2*R, yc) });
  // (4) de la cúspide al vástago  — aquí la tangente se INVIERTE, y esa inversión
  //     ES la punta: los dos cuartos se encuentran en un vértice, no en una curva.
  segs.push_back({ true,  point(-2*R + K, yc),  point(-R, yc + R - K), point(-R, yc + R) });
  // (5) vástago superior
  segs.push_back({ false, point(0, 0),          point(0, 0),         point(-R, span - R) });
  // (6) del vástago al extremo superior — inversa a (4), como (1) lo es de (3)
  segs.push_back({ true,  point(-R, span - R + K), point(-K, span),  point(0, span) });
  return segs;
}

// Lleva los tramos al dispositivo, dados los dos extremos YA TRANSFORMADOS por el
// marco. Devuelve los tramos con sus puntos en dispositivo y el arranque en
// `start`. Vacío si el vano es degenerado.
//
// Existe aquí, y no repetida en cada backend, porque es donde se decide el
// COSTADO: el eje +x local es el vano girado −90°, así que la cúspide (que va a
// −x) cae 90° en sentido antihorario respecto de la dirección del vano. Tres
// copias de esta cuenta serían tres oportunidades de que una salga espejada, y un
// espejo es de lo más difícil de ver en un golden.
inline std::vector<BraceSeg> bracePlaced(double dx0, double dy0,
                                         double dx1, double dy1,
                                         double depth, double tip,
                                         point &start) {
  double vx = dx1 - dx0, vy = dy1 - dy0;
  double span = std::sqrt(vx * vx + vy * vy);
  std::vector<BraceSeg> segs = braceSegments(span, depth, tip);
  if (segs.empty())
    return segs;
  double ux = vx / span, uy = vy / span;    // +y local
  double ex = uy,        ey = -ux;          // +x local
  auto dev = [&](const point &p) {
    return point(dx0 + p.x * ex + p.y * ux,
                 dy0 + p.x * ey + p.y * uy);
  };
  for (auto &s : segs) {
    if (s.curve) { s.c1 = dev(s.c1); s.c2 = dev(s.c2); }
    s.to = dev(s.to);
  }
  start = point(dx0, dy0);
  return segs;
}

#endif  // MG_BRACE_H
