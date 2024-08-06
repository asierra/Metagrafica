#CXX = g++
CXX = clang++
CXXFLAGS = -g -std=c++14
LIBS = -lm -Wmultichar
LDFLAGS = -g -Wpedantic
CPPFLAGS = -I. -fno-rtti -fno-exceptions -Wpedantic -Wall -O3

SHELL = /bin/sh
PREFIX = /usr/local
MANPREFIX ?= ${PREFIX}/share/man

SRCS = EPSDisplay.cpp main.cpp structure.cpp matrix.cpp primitives.cpp \
	lexmg.cpp text.cpp text_parser.cpp Parser.cpp splines.cpp

OBJS = EPSDisplay.o main.o structure.o matrix.o primitives.o \
	lexmg.o text.o text_parser.o Parser.o splines.o

all: mg mg.1

mg.1: mg.1.md
	pandoc mg.1.md -s -t man -o mg.1

mg.1.pdf: mg.1.md
	pandoc mg.1.md -s -t pdf -o mg.1.pdf

lexmg.cpp: mgpp.l
	flex mgpp.l

%.o: %.cpp mg.h
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $<

mg: $(OBJS) 
	$(CXX) -o mg -g $(OBJS) $(LDFLAGS) $(LIBS)

install: mg mg.1
	install -m 755 mg $(PREFIX)/bin
	install mg.1 ${MANPREFIX}/man1/

uninstall:
	rm $(PREFIX)/bin/mg
	rm ${MANPREFIX}/man1/mg.1

clean: 
	rm -f *.o *~ mg mg.1
# DO NOT DELETE

EPSDisplay.o: EPSDisplay.h Display.h primitives.h
EPSDisplay.o: matrix.h text.h font_cmmi.h mg.h
lexmg.o: mgpp_tab.h
main.o: mg.h text.h primitives.h matrix.h EPSDisplay.h Parser.h
matrix.o: matrix.h 
Parser.o: Parser.h mg.h text.h primitives.h matrix.h
Parser.o: mgpp_tab.h text_parser.h splines.h
primitives.o: primitives.h matrix.h Display.h text.h
splines.o: splines.h primitives.h matrix.h
structure.o: mg.h text.h
structure.o: primitives.h matrix.h Display.h
text.o: text.h primitives.h matrix.h Display.h
text_parser.o: text.h primitives.h matrix.h
