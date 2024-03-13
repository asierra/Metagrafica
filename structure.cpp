#include <math.h>
#include <stdio.h>
#include "mg.h"
#include "Display.h"


std::map<std::string, Structure*> Structure::structure_map;


 void Structure::define_in_device(Display &g) {
  for (auto it : structure_map) {
    Structure* st = it.second;
    printf("Structure name %s\n", st->getName().c_str());

    if (st->times_used > 1 && !st->isDefinedInDevice()) {
      g.structureDefBegin(st->getName());
      st->draw(g);
      g.structureDefEnd();
      st->setDefinedInDevice();
    }
  }
}

void Structure::draw(Display &g) {
  for (const auto &pr : prlist) {
    pr->draw(g);
  }
}

void StructurePath::draw(Display &g) {
  for (const auto &p : path) {
    Matrix mtpt;
    mtpt.translate(p.x, p.y);

    // Hacer este ajuste solo una vez en el primer nivel o no hacerlo
    /*if (g.getRatio() > 1.0) { // ojo
      mtpt.scale(1.0, 1/g.getRatio());
    } else if (g.getRatio() < 1.0) {
      mtpt.scale(g.getRatio(), 1.0);
    }*/
    g.pushMatrix(mtpt);
    g.pushMatrix(MTST);
    g.save();
    g.structure(structure->getName());
    g.restore();
    g.popMatrix(MTST);
    g.popMatrix();
  }
}

void StructureRectangle::draw(Display &g) {
  Matrix mtrc;
  float dx = rup.x - llp.x;
  float dy = rup.y - llp.y;
  g.save();
  mtrc.translate(llp.x, llp.y);
  mtrc.scale(dx, dy);
  g.pushMatrix(mtrc);
  g.structure(structure->getName());
  g.popMatrix();
  g.restore();
}


void StructureLine::draw_side(Display &g, bool side) {
  Matrix mtln;
  point pos;
  float rt = 0.0;
  float dx = (rup.x - llp.x);
  float dy = (rup.y - llp.y);

  if (side) {
    pos.x = llp.x + shift*dx;
    pos.y = llp.y + shift*dy;
  } else {
    dx = -dx;
    dy = -dy;
    pos.x = rup.x + shift*dx;
    pos.y = rup.y + shift*dy;
  }
  mtln.translate(pos.x, pos.y);
  if (g.getRatio() > 1.0) {
    mtln.scale(1.0, 1/g.getRatio());
    dy *= g.getRatio();
  } else if (g.getRatio() < 1.0) {
    mtln.scale(g.getRatio(), 1.0);
    dy *= g.getRatio();
  }
  mtln.scale(scale.x, scale.y);
  rt = atan2(dy, dx)*180/M_PI;
  if (rt!=0.0)
    mtln.rotate(rt);
  g.save();
  g.pushMatrix(mtln);
  g.structure(structure->getName());
  g.popMatrix();
  g.restore();
}

void StructureLine::draw(Display &g) {
  g.line(llp.x, llp.y, rup.x, rup.y);
  draw_side(g, true);
  if (both_sides) 
    draw_side(g, false);
}

void StructureArc::draw_side(Display &g, bool side) {
  Matrix mtar;
  float angi, angf, rt = 0.0;
  point stpos;

  if (side) {
    angi = ai;
    angf = af;
  } else {
    angf = ai;
    angi = af;
  }
  if (shift!=1.0) {
    float da = (angf - angi)*shift;
    angf = angi + da;
  }
  if (angi < angf)
    rt = angf + 90;
  else
    rt = angf - 90;
  stpos.x = r*cos(angf*M_PI/180);
  stpos.y = r*sin(angf*M_PI/180);
  if (g.getRatio() > 1.0) {
    stpos.x *= g.getRatio();
  } else if (g.getRatio() < 1.0) {
    stpos.x *= g.getRatio();
  }
  stpos.x += pos.x;
  stpos.y += pos.y;
  mtar.translate(stpos.x, stpos.y);
  if (g.getRatio() > 1.0) {
    mtar.scale(1.0, 1/g.getRatio());
  } else if (g.getRatio() < 1.0) {
    mtar.scale(g.getRatio(), 1.0);
  }
  mtar.scale(scale.x, scale.y);
  if (rt!=0.0)
    mtar.rotate(rt);
  
  g.save();
  g.pushMatrix(mtar);
  g.structure(structure->getName());
  g.popMatrix();
  g.restore();
}

void StructureArc::draw(Display &g) {
  g.arc(pos.x, pos.y, r, r, ai, af);
  draw_side(g, true);
  if (both_sides)
    draw_side(g, false);
}

MetaGrafica::MetaGrafica() : Structure() {
  maxDepth = 10;
  dcmx = 0;
  dcmy = 0;
}

void MetaGrafica::draw(Display &g) {
  if (dcmx > 0)
    g.setDimension(dcmx, dcmy);
  if (fontsize > 0)
    g.setFontSize(fontsize);
  g.start();
  for (const auto &pr : prlist) {
      if (pr != nullptr) // ojo
       pr->draw(g);
  }
  g.end();
}
