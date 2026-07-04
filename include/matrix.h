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
