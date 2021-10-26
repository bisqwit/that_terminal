CXX=g++
CPPFLAGS=-Wall -Wextra
CXXFLAGS=-std=c++20

CXXFLAGS += -Ofast
#CXXFLAGS += -Og -g -fsanitize=address
#CXXFLAGS += -Og -g

CXXFLAGS += -fopenmp

CPPFLAGS += -Irendering -Itty -I. -Irendering/fonts -Ifile -Iutil/TinyDeflate -Iutil

CPPFLAGS += -fanalyzer

CXXFLAGS += $(shell pkg-config sdl2 --cflags)
LDLIBS   += $(shell pkg-config sdl2 --libs)

# For beep:
LDLIBS   += $(shell pkg-config x11 --libs)

# For video recording:
#CXXFLAGS += $(shell pkg-config libavcodec libavformat libavutil --cflags)
#LDLIBS   += $(shell pkg-config libavcodec libavformat libavutil --libs)

CPPFLAGS += -MP -MMD -MF$(subst .o,.d,$(addprefix .deps/,$(subst /,_,$@)))

#CXXFLAGS += -pg

# for forkpty:
LDLIBS   += -lutil

#CPPFLAGS += -g

PREFIX := $(shell if [ `id -u` = 0 ]; then echo /usr/local ; else echo $$HOME/.local; fi)

OBJS=\
	tty/terminal.o \
	tty/forkpty.o \
	tty/256color.o \
	rendering/screen.o \
	rendering/person.o \
	rendering/color.o \
	rendering/cset.o \
	rendering/fonts/make_similarities.o \
	rendering/fonts/read_fonts.o \
	rendering/fonts/read_font.o \
	rendering/fonts/font_planner.o \
	file/share.o \
	beeper.o \
	main.o \
	ctype.o \
	autoinput.o \
	clock.o

term: $(OBJS)
	$(CXX) -o $@ $(OBJS) $(CXXFLAGS) $(LDLIBS)
	
install:
	- install term $(PREFIX)/bin/that_terminal
	mkdir -p $(PREFIX)/share/that_terminal
	(cd share; find -type f|grep -v '~'|xargs cp -a -v --parents -t $(PREFIX)/share/that_terminal)
uninstall:
	rm -vf $(PREFIX)/bin/that_terminal
	rm -vrf $(PREFIX)/share/that_terminal

util/make-fonts-list: util/make-fonts-list.cc ctype.o \
		rendering/fonts.inc rendering/fonts-authentic.inc \
		rendering/fonts-list.inc ctype.hh 
	$(CXX) -o "$@" "$<" ctype.o $(CXXFLAGS)      -Irendering
util/make-fonts-list-bitmap: util/make-fonts-list-bitmap.cc ctype.o \
		rendering/fonts.inc rendering/fonts-authentic.inc \
		rendering/fonts-list.inc ctype.hh 
	$(CXX) -o "$@" "$<" ctype.o $(CXXFLAGS) -lgd -Irendering

doc/fonts.md.tmp: util/make-fonts-list
	(cd util; ./make-fonts-list) >   doc/fonts.md.tmp4
	grep -n accents                  doc/fonts.md.tmp4 | sed 'ss:.*ss' > doc/fonts.md.tmp5
	head -n$$(cat doc/fonts.md.tmp5) doc/fonts.md.tmp4 > "$@"
	tail -n +$$(($$(cat doc/fonts.md.tmp5)+1)) doc/fonts.md.tmp4 > doc/fonts.md.tmp3
	rm -f doc/fonts.md.tmp4 doc/fonts.md.tmp5
doc/fonts.md.tmp3: doc/fonts.md.tmp ;
doc/fonts.md.tmp2: util/make-fonts-list-bitmap
	(cd util; ./make-fonts-list-bitmap) > "$@"
doc/fonts.md: doc/fonts.md.tmp doc/fonts.md.tmp2 doc/fonts.md.tmp3
	cat "$@".tmp "$@".tmp2 "$@".tmp3 > "$@"
	rm -f "$@".tmp "$@".tmp2 "$@".tmp3

compress3:
	parallel -j45 util/compressor.sh -- \
	`echo \
		doc/coverage-*.png \
	`

-include $(addprefix .deps/,$(subst /,_,$(OBJS:.o=.d)))
