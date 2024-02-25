#include "matrix.h"
#include <stdio.h>
#include <string.h>

constexpr DataMatrix MATIDEN = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};

Matrix::Matrix() { memcpy(M, MATIDEN, sizeof(Matrix)); }

Matrix Matrix::operator=(Matrix A) {
  memcpy(M, A.M, sizeof(Matrix));

  return *this;
};

void Matrix::initialize() { memcpy(M, MATIDEN, sizeof(Matrix)); }

void Matrix::set(DataMatrix m) { memcpy(M, m, sizeof(Matrix)); }

void Matrix::scale(float x, float y) {
  DataMatrix A;
  memcpy(A, MATIDEN, sizeof(Matrix));
  A[0][0] = x;
  A[1][1] = y;
  matmat(A);
}

void Matrix::translate(float x, float y) {
  DataMatrix A;
  memcpy(A, MATIDEN, sizeof(Matrix));
  A[0][2] = x;
  A[1][2] = y;
  matmat(A);
}

void Matrix::rotate(float theta) {
  DataMatrix A;
  memcpy(A, MATIDEN, sizeof(Matrix));
  theta *= deg2rad;
  float cs=cos(theta), sn=sin(theta);

  A[0][0] = cs;
  A[0][1] = -sn;
  A[1][0] = -A[0][1];
  A[1][1] = A[0][0];
  matmat(A);
}

void Matrix::transform(float &x, float &y) {
  float xp = x, yp = y;

  x = M[0][0] * xp + M[0][1] * yp + M[0][2];
  y = M[1][0] * xp + M[1][1] * yp + M[1][2];
}

void Matrix::transf2d(float &x, float &y) {
  float xp = x, yp = y;

  x = M[0][0] * xp + M[0][1] * yp;
  y = M[1][0] * xp + M[1][1] * yp;
}

Matrix Matrix::operator*(Matrix B) {
  Matrix A(*this);

  A.matmat(B.M);
  return A;
}

Matrix Matrix::operator*=(Matrix B) {
  matmat(B.M);
  return *this;
}

void Matrix::matmat(DataMatrix b) {
  DataMatrix a;

  memcpy(a, M, sizeof(Matrix));

  for (int i = 0; i < 3; i++) {
    M[0][i] = a[0][0] * b[0][i] + a[0][1] * b[1][i] + a[0][2] * b[2][i];
    M[1][i] = a[1][0] * b[0][i] + a[1][1] * b[1][i] + a[1][2] * b[2][i];
    M[2][i] = a[2][0] * b[0][i] + a[2][1] * b[1][i] + a[2][2] * b[2][i];
  }
}

void Matrix::print() {
  printf("%g %g %g\n", M[0][0], M[0][1], M[0][2]);
  printf("%g %g %g\n", M[1][0], M[1][1], M[1][2]);
  printf("%g %g %g\n", M[2][0], M[2][1], M[2][2]);
}
