#include "matrix.h"
#include <stdio.h>
#include <string.h>

constexpr DataMatrix MATIDEN = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};

Matrix::Matrix() { memcpy(M, MATIDEN, sizeof(DataMatrix)); }

Matrix& Matrix::operator=(const Matrix& A) {
  memcpy(M, A.M, sizeof(DataMatrix));
  return *this;
};

void Matrix::initialize() { memcpy(M, MATIDEN, sizeof(DataMatrix)); }

void Matrix::set(DataMatrix m) { memcpy(M, m, sizeof(DataMatrix)); }

void Matrix::scale(double x, double y) {
  DataMatrix A;
  memcpy(A, MATIDEN, sizeof(DataMatrix));
  A[0][0] = x;
  A[1][1] = y;
  matmat(A);
}

void Matrix::shear(double x, double y) {
  DataMatrix A;
  memcpy(A, MATIDEN, sizeof(DataMatrix));
  A[0][1] = x;
  A[1][0] = y;
  matmat(A);
}

void Matrix::translate(double x, double y) {
  DataMatrix A;
  memcpy(A, MATIDEN, sizeof(DataMatrix));
  A[0][2] = x;
  A[1][2] = y;
  matmat(A);
}

void Matrix::to_rectangle(double x1, double y1, double x2, double y2) {
  initialize();
  translate(x1, y1);
  scale(x2-x1, y2-y1);
}

void Matrix::rotate(double theta) {
  DataMatrix A;
  memcpy(A, MATIDEN, sizeof(DataMatrix));
  theta *= deg2rad;
  double cs=cos(theta), sn=sin(theta);

  A[0][0] = cs;
  A[0][1] = -sn;
  A[1][0] = -A[0][1];
  A[1][1] = A[0][0];
  matmat(A);
}

void Matrix::transform(double &x, double &y) {
  double xp = x, yp = y;

  x = M[0][0] * xp + M[0][1] * yp + M[0][2];
  y = M[1][0] * xp + M[1][1] * yp + M[1][2];
}

void Matrix::transf2d(double &x, double &y) {
  double xp = x, yp = y;

  x = M[0][0] * xp + M[0][1] * yp;
  y = M[1][0] * xp + M[1][1] * yp;
}

Matrix Matrix::operator*(const Matrix& B) {
  Matrix A(*this);
  A.matmat(B.M);
  return A;
}

Matrix& Matrix::operator*=(const Matrix& B) {
  matmat(B.M);
  return *this;
}

/*
bool Matrix::operator==(Matrix B) {
  int result = memcmp(M, B, sizeof(M));
  return (result==0);
}
*/

void Matrix::matmat(const DataMatrix b) {
  DataMatrix a;

  memcpy(a, M, sizeof(DataMatrix));

  for (int i = 0; i < 3; i++) {
    M[0][i] = a[0][0] * b[0][i] + a[0][1] * b[1][i] + a[0][2] * b[2][i];
    M[1][i] = a[1][0] * b[0][i] + a[1][1] * b[1][i] + a[1][2] * b[2][i];
    M[2][i] = a[2][0] * b[0][i] + a[2][1] * b[1][i] + a[2][2] * b[2][i];
  }
}

void Matrix::print() {
  printf("%6g %6g %6g\n", M[0][0], M[0][1], M[0][2]);
  printf("%6g %6g %6g\n", M[1][0], M[1][1], M[1][2]);
  printf("%6g %6g %6g\n", M[2][0], M[2][1], M[2][2]);
}

bool Matrix::is_identity() {
  int result = memcmp(M, MATIDEN, sizeof(M));
  return (result==0);
}