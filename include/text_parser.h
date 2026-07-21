/*
       File:  text_parser.h
              Prepare a set of text chunks in parser time.
MetaGrafica:  Human descriptive language to generate publication quality
              Display in PostScript.
Copyright (c) 1988-2026 Alejandro Aguilar Sierra (algsierra@gmail.com)
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
#include <map>
#include "text.h"

// input a complex utf8 string
// output a set of string chunks latin1 single char per character
std::unique_ptr<GraphicsItem> parse_text(std::string input_utf8, FontFace ff, bool& using_reencode, bool& using_fontcmmi);

// Mapas byte->Unicode COMPARTIDOS por los backends para el texto matemático:
// traducen el byte de un run FN_SYMBOL / FN_TEX_CMMI al codepoint Unicode del
// glifo (ver kNameToUnicode en text_parser.cpp).
const std::map<unsigned char, unsigned int>& symbolUnicode();
const std::map<unsigned char, unsigned int>& cmmiUnicode();

FontFace change_font_face(unsigned char code_font_face, FontFace font_face, bool& using_fontcmmi);

FontFace get_font_face_from_string(std::string s, bool& using_fontcmmi);


// Caracteres de texto fuera de Latin-1 que las base-14 SI tienen (§14.4). Ver la
// tabla en text_parser.cpp: por que existen las ranuras y como las traduce cada
// backend.
struct ExtraGlyph {
    unsigned char slot;        // 1..27, posicion interna (control C0, nunca en texto)
    unsigned int  codepoint;   // Unicode
    const char   *psname;      // nombre de glifo PostScript (para el /Encoding de EPS)
    unsigned char winansi;     // byte en CP1252, o 0 si no esta
};
extern const ExtraGlyph kExtraTextGlyphs[];
extern const int kNumExtraTextGlyphs;
unsigned char extraSlotFor(unsigned int cp);
const ExtraGlyph *extraGlyphForSlot(unsigned char slot);

#endif