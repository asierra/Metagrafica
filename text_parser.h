/*
       File:  text_parser.h
              Prepare a set of text chunks in parser time.
MetaGrafica:  Human descriptive language to generate publication quality graphics.
              Display in PostScript.
     Author:  Alejandro Aguilar Sierra, UNAM
    Version:  2024
Antecedents: 2011, 1999 C++ STL, 1991 C. Original: 1988, Pascal and Assembler.
*/

#if !defined(__TEXT_PARSER_H)
#define __TEXT_PARSER_H

#include <memory>
#include "text.h"

// input a complex utf8 string
// output a set of string chunks latin1 single char per character
std::unique_ptr<GraphicsItem> parse_text(string input_utf8, FontFace ff);

FontFace change_font_face(unsigned char code_font_face, FontFace font_face);

FontFace get_font_face_from_string(string s);

#endif