/*
       File:  structure.h
              Definition of a complex graphic item based on a list of basic
              Display items and a name as identifier.
MetaGrafica:  Human descriptive language to generate publication quality
              Display in PostScript.
     Author:  Alejandro Aguilar Sierra, UNAM
    Version:  2024
Antecedents: 2011, 1999 C++ STL; 1991 C; Original: 1988 Pascal and Assembler.
*/
#if !defined(__MG_H)
#define __MG_H

#include "version.h"
#include "text.h"

/** 
    A structure is defined by a name and a list of Display items. 
 */
class Structure {
public:
  Structure() { times_used = 0; defined_in_device = false; }

  void draw(Display &);

  string getName() { return name; }

  int setName(string n) { 
    if (structure_map.find(n)==structure_map.end()) {
      name = n; 
      structure_map[name] = this; //.insert({name, this});
    } else {
      // Error: The name already exists.
      return -1;
    }
    return 0;
  }

  static Structure* getStructure(string nombre) { 
    if (structure_map.find(nombre)==structure_map.end()) {
      // Error: The structure named nombre doesn't exist.
      return nullptr;
    } else {
      return structure_map[nombre];
    }
  }

  static void define_in_device(Display &g);
  
  void setGraphicsItems(GraphicsItemList p) { prlist = std::move(p); }
 
  bool isDefinedInDevice() { return defined_in_device; }
  
  void incUses() { times_used++; }
  
  void decUses() { times_used--; }

  void setDefinedInDevice() { defined_in_device = true; }
  
protected:
  string name;
  unsigned times_used;
  bool defined_in_device;
  GraphicsItemList prlist;
private:
  /// Contains all defined structures
  static map<string, Structure*> structure_map;
public:
  static void cleanup();
};


/* 
   The basic structure user has a reference to a structure and is displayed 
   using the structure matrix.
 */
class StructureUser : public GraphicsItem {
public:
  StructureUser() : GraphicsItem(GI_STRUCTURE_REF) { }
  
  ~StructureUser() { structure->decUses(); }

  void setStructure(Structure *s) { structure = s; structure->incUses(); }

  void draw(Display &)=0;
protected:
  Structure* structure;
};


/* 
   A rectangle structure user is displayed over a specific user defined rectangle.
*/
class StructureRectangle : public StructureUser {
public:
  void setPoints(point q1, point q2) {  llp = q1; rup = q2; }

  void draw(Display &);
private:
  point llp, rup;
};


/* 
   A line structure user is displayed with a line between two points and the
   referenced structure rotated according to the line inclination.
*/
class StructureLine : public StructureUser {
public:
  StructureLine() { both_sides = false; shift = 1; gap = 0; }
  void setScale(float sx, float sy) { scale.x = sx; scale.y = sy; }
  void setPoints(point q1, point q2) {  llp = q1; rup = q2; }
  void setShift(float p) {  shift = p; }
  void setGap(float p) {  gap = p; }
  void setBothSides(bool both=true) { both_sides = both; }
  void draw(Display &);
private:
  void draw_side(Display &g, bool side);
  point scale;
  point llp, rup;
  float shift;
  float gap;
  bool both_sides;
};


/* 
   An arc structure user is displayed with a circular arc and the
   referenced structure rotated according to the inclination.
*/
class StructureArc : public StructureUser {
public:
  StructureArc() { both_sides = false; shift = 1; }
  void setScale(float sx, float sy) { scale.x = sx; scale.y = sy; }  
  void setRadius(float rr) { r = rr; }
  void setAngles(float x, float y) { ai = x; af = y; }
  void setPoint(point q1) {  pos = q1; }
  void setShift(float p) {  shift = p; }
  void setBothSides(bool both=true) { both_sides = both; }
  void draw(Display &);
private:
  void draw_side(Display &g, bool side);
  point scale;
  float r;
  float ai, af;
  point pos;
  float shift;
  bool both_sides;
};


/* 
   A rectangle structure user is displayed over a specific user defined rectangle.
*/
class StructurePath : public StructureUser {
public:
  void setPath(Path p) {  path = p; }

  void draw(Display &);
private:
  Path path;
};


class MetaGrafica : public Structure {
public:
  ///
  MetaGrafica();
  ///
  void draw(Display &);

  void setDimension(float x, float y) {
    dcmx = x;
    dcmy = y;
  }

  void getDimension(float &x, float &y) {
    x = dcmx;
    y = dcmy;
  }
  
  void setDepth(int d) { maxDepth = d; }
  int getDepth() { return maxDepth; }

  void setFontSize(int d) { fontsize = d; }
//  int getFontSize() { return fontsize; }

private:
  /// Profundidad maxima de recursion
  int maxDepth;
  /// Dimensiones físicas en cm
  float dcmx, dcmy;
  /// Font size in points
  float fontsize;
  ///
  //Matrix matrix;
};

#endif
