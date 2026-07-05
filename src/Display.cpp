/*
       File:  Display.cpp
              Device-independent part of the graphics state machine: the
              structure resolution shared by every backend.
MetaGrafica:  Human descriptive language to generate publication quality
              graphics.
     Author:  Alejandro Aguilar Sierra, UNAM
    Version:  2026
*/
#include <stdio.h>
#include "Display.h"

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
