/*
       File:  text_parser.h
              Prepare a set of text chunks in parser time.
MetaGrafica:  Human descriptive language to generate publication quality
              Display in PostScript.
Copyright (c) 2026 Alejandro Aguilar Sierra (asierra@unam.mx)
    Version:  2026
Antecedents: Version 0.0 1988 Pascal and Assembler, first published paper. 
			 Version 1.0 1991 C, first published book.
			 Version 2.0 1999-2024 C++ STL, EPS only, three published books. 
			 
 This file is part of MetaGrafica.
 Licensed under the GNU General Public License v3.0 (see LICENSE file).
*/

#if !defined(MG_TEXT_PARSER_H)
#define MG_TEXT_PARSER_H

#include <memory>
#include "text.h"

// input a complex utf8 string
// output a set of string chunks latin1 single char per character
std::unique_ptr<GraphicsItem> parse_text(std::string input_utf8, FontFace ff, bool& using_reencode, bool& using_fontcmmi);

FontFace change_font_face(unsigned char code_font_face, FontFace font_face, bool& using_fontcmmi);

FontFace get_font_face_from_string(std::string s, bool& using_fontcmmi);

#endif