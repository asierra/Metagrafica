#CXX = g++
CXX = clang++
CXXFLAGS = -g -std=c++14
LIBS = -lm -Wmultichar
LDFLAGS = -g -Wpedantic
CPPFLAGS = -I./include -fno-rtti -fno-exceptions -Wpedantic -Wall -O3

SHELL = /bin/sh
PREFIX = /usr/local
MANPREFIX ?= ${PREFIX}/share/man

SRCDIR = src
INCDIR = include
OBJDIR = obj
BINDIR = bin
MANDIR = man

SRCS = $(addprefix $(SRCDIR)/, EPSDisplay.cpp main.cpp structure.cpp matrix.cpp \
	primitives.cpp lexmg.cpp text.cpp text_parser.cpp Parser.cpp splines.cpp)

OBJS = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))

all: $(BINDIR)/mg $(MANDIR)/mg.1

$(MANDIR)/mg.1: $(MANDIR)/mg.1.md
	pandoc $< -s -t man -o $@

$(MANDIR)/mg.1.pdf: $(MANDIR)/mg.1.md
	pandoc $< -s -t pdf -o $@

$(INCDIR)/version.h:
	git log -1 --pretty=format:'#define MG_VERSION "%h - %cd"' > $@
	echo "\n" >> $@

$(SRCDIR)/lexmg.cpp: $(SRCDIR)/mgpp.l
	flex -o $@ $<

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

$(BINDIR)/mg: $(OBJS) | $(BINDIR)
	$(CXX) -o $@ -g $(OBJS) $(LDFLAGS) $(LIBS)

install: $(BINDIR)/mg $(MANDIR)/mg.1
	install -m 755 $(BINDIR)/mg $(PREFIX)/bin
	install $(MANDIR)/mg.1 ${MANPREFIX}/man1/

uninstall:
	rm $(PREFIX)/bin/mg
	rm ${MANPREFIX}/man1/mg.1

clean:
	rm -rf $(OBJDIR) $(BINDIR) $(MANDIR)/mg.1
# DO NOT DELETE

$(OBJDIR)/EPSDisplay.o: $(INCDIR)/EPSDisplay.h $(INCDIR)/Display.h $(INCDIR)/primitives.h
$(OBJDIR)/EPSDisplay.o: $(INCDIR)/matrix.h $(INCDIR)/text.h $(INCDIR)/font_cmmi.h $(INCDIR)/mg.h
$(OBJDIR)/lexmg.o: $(INCDIR)/mgpp_tab.h
$(OBJDIR)/main.o: $(INCDIR)/mg.h $(INCDIR)/text.h $(INCDIR)/primitives.h $(INCDIR)/matrix.h
$(OBJDIR)/main.o: $(INCDIR)/EPSDisplay.h $(INCDIR)/Parser.h $(INCDIR)/version.h
$(OBJDIR)/matrix.o: $(INCDIR)/matrix.h
$(OBJDIR)/Parser.o: $(INCDIR)/Parser.h $(INCDIR)/mg.h $(INCDIR)/text.h $(INCDIR)/primitives.h
$(OBJDIR)/Parser.o: $(INCDIR)/matrix.h $(INCDIR)/mgpp_tab.h $(INCDIR)/text_parser.h $(INCDIR)/splines.h
$(OBJDIR)/primitives.o: $(INCDIR)/primitives.h $(INCDIR)/matrix.h $(INCDIR)/Display.h $(INCDIR)/text.h
$(OBJDIR)/splines.o: $(INCDIR)/splines.h $(INCDIR)/primitives.h $(INCDIR)/matrix.h
$(OBJDIR)/structure.o: $(INCDIR)/mg.h $(INCDIR)/text.h $(INCDIR)/primitives.h
$(OBJDIR)/structure.o: $(INCDIR)/matrix.h $(INCDIR)/Display.h
$(OBJDIR)/text.o: $(INCDIR)/text.h $(INCDIR)/primitives.h $(INCDIR)/matrix.h $(INCDIR)/Display.h
$(OBJDIR)/text_parser.o: $(INCDIR)/text.h $(INCDIR)/primitives.h $(INCDIR)/matrix.h
