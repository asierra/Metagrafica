/*
       File:  structure.h
              Definition of a complex graphic item based on a list of basic
              Display items and a name as identifier.
MetaGrafica:  Human descriptive language to generate publication quality
              Display in PostScript.
     Author:  Alejandro Aguilar Sierra, UNAM
    Version:  2024
Antecedents: 2011, 1999 C++ STL, 1991 C. Original: 1988, Pascal and Assembler.
*/
#if !defined(__PARSER_H)
#define __PARSER_H

#include "mg.h"

#include <string.h>


// typedef hash_map<const char*, PrimitiveList, hash<const char*>, eqstr>
// PrimitiveMap;

//typedef map<const char *, PrimitiveList, ltstr> PrimitiveMap;
//typedef map<const char *, PrimitiveList, ltstr>::iterator mapIterator;


class Parser {

public:
  ///
  Parser(string filename);
  ///
  ~Parser();
  ///
  MetaGrafica *parse();
  ///
  string getName() { return filename; }
  ///
  string getCanonicalName() { return canfilename; }
  ///
  void setLogFile(FILE *);

private:
  float parseFloat();
  string parseString();
  void parseDef(int def);
  Path parsePoints();
  GraphicsItemList parsePrimitives();
  void oldParseMatrix(int mo, Matrix &mt);
  Transform* parseMatrix(int mo);
  GraphicsItem *parsePrimitive(int);
  Path parseRectangle();
  point parsePoint();

  /// Window
  float wmx, wmy, wdx, wdy;
  point pp;
  //StructureMap structure_map;
  map<string, Path> listmap;
  MetaGrafica *mg;
  int lastyylex;
  string filename;
  string canfilename;
  bool is_spline_to_bezier;
  int spline_nodes_per_segment;
};

#endif
