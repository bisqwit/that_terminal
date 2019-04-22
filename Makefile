CXX=g++
CPPFLAGS=-Wall -Wextra
CXXFLAGS=-std=c++17

#CXXFLAGS += -Ofast
CXXFLAGS += -Og -g -fsanitize=address

CXXFLAGS += -fopenmp

CPPFLAGS += -Irendering -Itty -I. -Irendering/fonts

CXXFLAGS += $(shell pkg-config sdl2 --cflags)
LDLIBS   += $(shell pkg-config sdl2 --libs)

CPPFLAGS += -MP -MMD -MF$(subst .o,.d,$(addprefix .deps/,$(subst /,_,$@)))

#CXXFLAGS += -pg

# for forkpty:
LDLIBS   += -lutil

OBJS=\
	tty/terminal.o \
	tty/forkpty.o \
	tty/256color.o \
	rendering/screen.o \
	rendering/person.o \
	rendering/color.o \
	rendering/cset.o \
	beeper.o \
	main.o \
	ctype.o

term: $(OBJS)
	$(CXX) -o $@ $(OBJS) $(CXXFLAGS) $(LDLIBS)
	
#rendering/color.cc: rendering/color.cc.re
#	re2c -P $< -o$@

rendering/fonts.inc: rendering/fonts/make.php \
		rendering/fonts/read_font.php \
		rendering/fonts/font_gen2.php \
		rendering/fonts/data/cp857-8x14.psf.gz \
		rendering/fonts/data/cp857-8x16.psf.gz \
		rendering/fonts/data/cp857-8x8.psf.gz \
		rendering/fonts/data/iso01.f08.psf.gz \
		rendering/fonts/data/iso01.f14.psf.gz \
		rendering/fonts/data/iso01.f16.psf.gz \
		rendering/fonts/data/iso02.f08.psf.gz \
		rendering/fonts/data/iso02.f14.psf.gz \
		rendering/fonts/data/iso02.f16.psf.gz \
		rendering/fonts/data/iso03.f08.psf.gz \
		rendering/fonts/data/iso03.f14.psf.gz \
		rendering/fonts/data/iso03.f16.psf.gz \
		rendering/fonts/data/iso04.f08.psf.gz \
		rendering/fonts/data/iso04.f14.psf.gz \
		rendering/fonts/data/iso04.f16.psf.gz \
		rendering/fonts/data/iso05.f08.psf.gz \
		rendering/fonts/data/iso05.f14.psf.gz \
		rendering/fonts/data/iso05.f16.psf.gz \
		rendering/fonts/data/iso06.f08.psf.gz \
		rendering/fonts/data/iso06.f14.psf.gz \
		rendering/fonts/data/iso06.f16.psf.gz \
		rendering/fonts/data/iso07.f08.psf.gz \
		rendering/fonts/data/iso07.f14.psf.gz \
		rendering/fonts/data/iso07.f16.psf.gz \
		rendering/fonts/data/iso08.f08.psf.gz \
		rendering/fonts/data/iso08.f14.psf.gz \
		rendering/fonts/data/iso08.f16.psf.gz \
		rendering/fonts/data/iso09.f08.psf.gz \
		rendering/fonts/data/iso09.f14.psf.gz \
		rendering/fonts/data/iso09.f16.psf.gz \
		rendering/fonts/data/iso10.f08.psf.gz \
		rendering/fonts/data/iso10.f14.psf.gz \
		rendering/fonts/data/iso10.f16.psf.gz \
		rendering/fonts/data/iso14.f16.psf.gz \
		rendering/fonts/data/koi8-8x14.psf.gz \
		rendering/fonts/data/koi8-8x16.psf.gz \
		rendering/fonts/data/koi8-8x8.psf.gz \
		rendering/fonts/data/lat1-08.psf.gz \
		rendering/fonts/data/lat1-10.psf.gz \
		rendering/fonts/data/lat1-12.psf.gz \
		rendering/fonts/data/lat1-14.psf.gz \
		rendering/fonts/data/lat1-16.psf.gz \
		rendering/fonts/data/lat2-08.psf.gz \
		rendering/fonts/data/lat2-10.psf.gz \
		rendering/fonts/data/lat2-12.psf.gz \
		rendering/fonts/data/lat2-14.psf.gz \
		rendering/fonts/data/lat2-16.psf.gz \
		rendering/fonts/data/lat4-08.psf.gz \
		rendering/fonts/data/lat4-10.psf.gz \
		rendering/fonts/data/lat4-12.psf.gz \
		rendering/fonts/data/lat4-14.psf.gz \
		rendering/fonts/data/lat4-16.psf.gz \
		rendering/fonts/data/lat9-08.psf.gz \
		rendering/fonts/data/lat9-10.psf.gz \
		rendering/fonts/data/lat9-12.psf.gz \
		rendering/fonts/data/lat9-14.psf.gz \
		rendering/fonts/data/lat9-16.psf.gz \
		rendering/fonts/data/437-8x14.inc \
		rendering/fonts/data/437-8x16.inc \
		rendering/fonts/data/437-8x8.inc \
		rendering/fonts/data/850-8x14.inc \
		rendering/fonts/data/850-8x16.inc \
		rendering/fonts/data/850-8x8.inc \
		rendering/fonts/data/852-8x14.inc \
		rendering/fonts/data/852-8x16.inc \
		rendering/fonts/data/852-8x8.inc \
		rendering/fonts/data/860-8x14.inc \
		rendering/fonts/data/860-8x16.inc \
		rendering/fonts/data/860-8x8.inc \
		rendering/fonts/data/863-8x14.inc \
		rendering/fonts/data/863-8x16.inc \
		rendering/fonts/data/863-8x8.inc \
		rendering/fonts/data/865-8x14.inc \
		rendering/fonts/data/865-8x16.inc \
		rendering/fonts/data/865-8x8.inc \
		rendering/fonts/data/8x10.inc \
		rendering/fonts/data/8x12.inc \
		rendering/fonts/data/8x14.inc \
		rendering/fonts/data/8x15.inc \
		rendering/fonts/data/8x16.inc \
		rendering/fonts/data/8x8.inc \
		rendering/fonts/data/6x9.bdf \
		rendering/fonts/data/vga8x19.bdf
	(cd rendering/fonts; php make.php > ../fonts.inc)

-include $(addprefix .deps/,$(subst /,_,$(OBJS:.o=.d)))
