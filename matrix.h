/*
       File:  matrix.h
              Definition of matrix data structure and operations.
MetaGrafica:  Human descriptive language to generate publication quality
              graphics in PostScript.
     Author:  Alejandro Aguilar Sierra, UNAM
    Version:  2024
Antecedents: 2011, 1999 C++ STL, 1991 C. Original: 1988, Pascal and Assembler.
*/


#if !defined(__TMATRIX_H)
#define __TMATRIX_H

#include <math.h>


constexpr double deg2rad = M_PI / 180;

enum MatrixOperation {
  OPMTL, // Translate
  OPMRT, // Rote
  OPMSC, // Scale
  OPMMT, // Define
  OPMID, // Identity
};

using DataMatrix = float[3][3];

class Matrix {
public:
  Matrix();

  Matrix operator=(Matrix);

  Matrix operator*(Matrix);

  Matrix operator*=(Matrix);

  void initialize();

  void scale(float, float);

  void translate(float, float);

  void rotate(float);

  void transform(float&, float&);

  void transf2d(float&, float&);

  void set(DataMatrix m);

  //void getAfinData();
  void matmat(DataMatrix b);

  void print();

private:
  DataMatrix M;
};

#endif
