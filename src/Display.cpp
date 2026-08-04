/*
       File:  Display.cpp
              Device-independent part of the graphics state machine: the
              structure resolution shared by every backend.
MetaGrafica:  Human descriptive language to generate publication quality
              graphics.
     Author:  Alejandro Aguilar Sierra, UNAM
*/
#include <stdio.h>
#include "Display.h"

// Aviso de lienzo en blanco. Ver el comentario de `inkPoint` en Display.h para
// qué mide la cobertura; aquí solo va la REGLA de cuándo hablar.
//
// Se avisa únicamente cuando la caja de la tinta NO TOCA la página. Es la
// condición segura: una caja que no intersecta el lienzo garantiza que no se
// pintó nada visible, así que no hay falsos positivos. Al revés no vale —una
// caja que sí toca puede seguir sin pintar nada dentro—, y esos casos se
// callan a propósito: en un aviso, un falso negativo cuesta mucho menos que
// enseñar a ignorarlo.
//
// Y NO se avisa cuando no hubo tinta ninguna: un .mg que no dibuja nada suele
// ser deliberado (`examples/curvas3.mg` es una biblioteca de datos y compila en
// blanco, y está en el corpus del golden). "No dibujé nada" es una decisión;
// "dibujé fuera del papel" es un accidente.
void Display::warnIfOffCanvas() const {
  if (!has_ink) return;
  if (inkx1 >= 0 && inkx0 <= dvx && inky1 >= 0 && inky0 <= dvy) return;

  // dvx/dvy y la cobertura están en puntos (los tres backends convierten en
  // start()); el mensaje va en cm, que es la unidad en la que el usuario
  // escribió `display_size`.
  const double k = point_to_cm;
  fprintf(stderr,
          "Aviso: la figura sale EN BLANCO — todo lo dibujado cae fuera del lienzo.\n"
          "  lienzo: x 0..%.2f, y 0..%.2f cm\n"
          "  dibujo: x %.2f..%.2f, y %.2f..%.2f cm\n"
          "  La ventana del mundo (world_window %g %g %g %g) es un recorte fijo: no se\n"
          "  ajusta a los datos. Si usas plot, su box= va en unidades de MUNDO, no en cm.\n",
          dvx * k, dvy * k,
          inkx0 * k, inkx1 * k, inky0 * k, inky1 * k,
          wwx, wwx + wwdx, wwy, wwy + wwdy);
}

void Display::structure(const std::string &name) {
  Structure *strct = mg_context ? mg_context->getStructure(name) : nullptr;
  if (!strct) {
    fprintf(stderr, "Error: estructura '%s' no definida\n", name.c_str());
    return;
  }
  dsstack.push(dspstate);
  Matrix prevmtst = mtst;
  strct->draw(*this);
  dspstate = dsstack.top();
  dsstack.pop();
  mtst = prevmtst;
}
