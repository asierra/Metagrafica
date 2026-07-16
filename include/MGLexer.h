/*
       File:  MGLexer.h
              Wrapper around the flex-generated yyFlexLexer for the .mg
              lexer; keyword and matrix-operator lookup tables.
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
#ifndef MG_MGLEXER_H
#define MG_MGLEXER_H

#if ! defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

#include "mgpp_tab.h"
#include "primitives.h"
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <iostream>

using std::map;
using std::string;
using std::vector;

class MGLexer : public yyFlexLexer {
public:
    MGLexer(std::istream* in) : yyFlexLexer(in) { init_tables(); }
    virtual ~MGLexer() = default;

    using yyFlexLexer::yylex;
    int yylex() override;
    
    void setBegin(int s) { 
        yy_start = 1 + 2 * s; 
    }

    void push_stream(std::unique_ptr<std::istream> stream) {
        include_streams.push_back(std::move(stream));
    }

    int getLine() const { return lineno(); }

    YYSTYPE yylval;
    YYSTYPE yylvalaux;

private:
    void init_tables();
    int busca_key(const char* key);
    bool search_mat(string key, int &opm, int &dfm);
    
    struct KeyInfo {
        int token;
        int type;
    };
    
    map<string, KeyInfo> keyword_map;
    map<string, MatrixOperation> map_opmat;
    map<string, PredefinedMatrix> map_dfmat;
    
    std::vector<std::unique_ptr<std::istream>> include_streams;
};

#endif