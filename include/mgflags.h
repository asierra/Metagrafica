/*
       File:  mgflags.h
              Feature-usage flags collected while parsing, consulted by
              backends that need to emit optional support code.
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
#if !defined(MG_MGFLAGS_H)
#define MG_MGFLAGS_H

struct MGFlags {
  bool using_dot = false;
  bool using_ellipse = false;
  bool using_reencode = false;
  bool using_textalign = false;
  bool using_fontcmmi = false;
  bool using_hatcher = false;
};

#endif
