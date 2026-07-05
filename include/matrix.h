/*
       File:  matrix.h
              Definition of matrix data structure and operations.
MetaGrafica:  Human descriptive language to generate publication quality
              Display in PostScript.
Copyright (c) 2026 Alejandro Aguilar Sierra (asierra@unam.mx)
    Version:  2026
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

enum MatrixOperation {
  OPMTL, // Translate
  OPMRT, // Rote
  OPMSC, // Scale
  OPMSH, // Shear
  OPMMT, // Define
  OPMID, // Identity
  OPMCP, // Compose
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

  // Transforma un par de semiejes (w,h) al dispositivo: cada uno escala con
  // la norma de la columna correspondiente (signo del elemento diagonal, para
  // reflejos). Bajo isometría+rotación un círculo sigue siendo círculo, cosa
  // que transf2d no garantiza (rota el vector (w,h) y mezcla componentes).
  void transform_radii(double&, double&);

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
