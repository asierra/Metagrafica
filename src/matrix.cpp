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

void Matrix::ellipse_frame(double cx, double cy, double rx, double ry,
                           double &Cx, double &Cy,
                           double &ux, double &uy, double &vx, double &vy) const {
  Cx = M[0][0] * cx + M[0][1] * cy + M[0][2];
  Cy = M[1][0] * cx + M[1][1] * cy + M[1][2];
  ux = M[0][0] * rx;  uy = M[1][0] * rx;   // imagen de (rx, 0)
  vx = M[0][1] * ry;  vy = M[1][1] * ry;   // imagen de (0, ry)
}

void Matrix::ellipse_axes(double ux, double uy, double vx, double vy,
                          double &rx, double &ry, double &angDeg) {
  // SVD 2×2 en forma cerrada sobre A = [u v]. Toda A se escribe R(φ)·diag(s1,s2)·R(θ):
  // E,H capturan la parte que rota por igual ambas columnas y F,G la que las
  // separa, de modo que los valores singulares son Q±R y el ángulo del eje mayor
  // es el promedio de los dos arcotangentes.
  const double E = (ux + vy) * 0.5, F = (ux - vy) * 0.5;
  const double G = (uy + vx) * 0.5, H = (uy - vx) * 0.5;
  const double Q = sqrt(E * E + H * H), R = sqrt(F * F + G * G);
  rx = Q + R;
  ry = fabs(Q - R);
  // Un CÍRCULO (R≈0 ⟺ rx≈ry) no tiene eje mayor: el ángulo es indeterminado y la
  // fórmula devuelve θ/2 para una rotación θ. Se fija en 0 para no publicar un
  // número que no afecta al dibujo y que ataría los goldens a un dato irrelevante.
  angDeg = (R <= 1e-12 * Q) ? 0.0
                            : 0.5 * (atan2(G, F) + atan2(H, E)) / deg2rad;
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