/*
       File:  matrix.h
              Definition of matrix data structure and operations.
MetaGrafica:  Human descriptive language to generate publication quality
              Display in PostScript.
Copyright (c) 1988-2026 Alejandro Aguilar Sierra (algsierra@gmail.com)
Antecedents: Version 0.0 1988 Pascal and Assembler, first published paper. 
			 Version 1.0 1991 C, first published book.
			 Version 2.0 1999-2024 C++ STL, EPS only, three published books. 
			 
 This file is part of MetaGrafica.
 Licensed under the GNU General Public License v3.0 (see LICENSE file).
*/


#if !defined(MG_MATRIX_H)
#define MG_MATRIX_H

#include <math.h>


constexpr double deg2rad = M_PI / 180;

// Redondea a cero lo que ya ES cero. Una rotación recta deja residuos de ~1e-16
// en los términos que deberían anularse (`cos(pi/2)` da 6.123e-17), y componer
// varias los amplifica: `repeat(..., transform=rotate(60))` acumulado tres veces
// llega a 1e-15 donde debería haber un 0 exacto.
//
// No es cosmética, y tiene DOS razones:
//  1. **Precisión.** El valor verdadero es cero, así que redondear ACERCA al
//     resultado exacto en vez de alejarse.
//  2. **Portabilidad.** Ese residuo es el único sitio donde la salida dependía de
//     la plataforma: lo imprime un printf con varias cifras y cada libm da un
//     último bit distinto. Medido el 2026-07-27: con esto, Linux, Windows y macOS
//     producen los 72 archivos del corpus byte a byte idénticos; sin esto, no.
//
// `ref` es la escala contra la que «pequeño» significa algo (típicamente la suma
// de los términos de la matriz): un umbral absoluto sería incorrecto, porque en
// coordenadas de dispositivo un 1e-9 legítimo existe.
inline double snap_zero(double v, double ref) {
  return (fabs(v) <= 1e-12 * ref) ? 0.0 : v;
}

enum MatrixOperation {
  OPMTL, // Translate
  OPMRT, // Rote
  OPMSC, // Scale
  OPMSH, // Shear
  OPMMT, // Define
  OPMID, // Identity
  OPMCP, // Compose
  // V3 (al final: no desplazar ordinales que usa V1). Apilan/restauran la matriz
  // acumulada `mt` con una Matrix dada (marco T·R·S de una invocación con
  // at/rotate/scale, §8). Igual que los placements: pushMatrix(m)/popMatrix().
  OPMPUSH,
  OPMPOP,
};

using DataMatrix = double[3][3];

class Matrix {
public:
  Matrix();

  Matrix& operator=(const Matrix&);

  Matrix operator*(const Matrix&);

  Matrix& operator*=(const Matrix&);

  //bool operator==(Matrix);

  void initialize();

  void scale(double, double);

  void shear(double, double);

  void translate(double, double);

  void rotate(double);

  void transform(double&, double&);

  void transf2d(double&, double&);

  // §4.5/§4.9 — marco de la elipse de dispositivo. Mapea la elipse local de
  // semiejes (rx,ry) centrada en (cx,cy) y devuelve su centro C y los dos
  // SEMIDIÁMETROS CONJUGADOS u,v (las imágenes de (rx,0) y (0,ry)):
  //
  //     P(t) = C + u·cos t + v·sin t
  //
  // Esta forma SÍ es cerrada bajo afinidad —vale igual para rotación, reflejo,
  // escala anisótropa y shear—, y por eso sustituyó al par
  // `transform_radii`+`get_rotation`, que solo acertaba cuando la matriz era
  // (escala uniforme)·(rotación)·(escala alineada a ejes). Se entrega tal cual
  // como matriz [ux uy vx vy Cx Cy] a PostScript (`concat`) y a las Béziers del
  // PDF, que son invariantes afines.
  void ellipse_frame(double cx, double cy, double rx, double ry,
                     double &Cx, double &Cy,
                     double &ux, double &uy, double &vx, double &vy) const;

  // Ejes y ángulo VERDADEROS de esa elipse (forma cerrada del SVD 2×2), para el
  // formato que no acepta una matriz y exige rx/ry/rotación: el comando `A` de SVG.
  //
  // ⚠️ NO son |u| y |v|. Esos son los semidiámetros CONJUGADOS, y solo coinciden
  // con los ejes cuando u⊥v. Confundirlos era el bug: en rpstest la elipse medía
  // 20.888×13.049 cuando la verdadera es 21.757×11.541 (13% de error en el eje
  // menor, 10.5° en el ángulo).
  static void ellipse_axes(double ux, double uy, double vx, double vy,
                           double &rx, double &ry, double &angDeg);

  void set(DataMatrix m);

  void to_rectangle(double x1, double y1, double x2, double y2);
  
//  bool has_translation() { return (fabs(M[0][2]) > 0.01 || fabs(M[1][2]) > 0.01); }

  bool is_identity();

  //void getAfinData();
  void matmat(const DataMatrix b);

  void print();

private:
  DataMatrix M;
};

#endif
