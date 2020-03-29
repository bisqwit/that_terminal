CXX=g++
CPPFLAGS=-Wall -Wextra
CXXFLAGS=-std=c++17

CXXFLAGS += -Ofast
#CXXFLAGS += -Og -g -fsanitize=address

CXXFLAGS += -fopenmp

CPPFLAGS += -Irendering -Itty -I. -Irendering/fonts

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
	ctype.o \
	autoinput.o \
	clock.o

term: $(OBJS)
	$(CXX) -o $@ $(OBJS) $(CXXFLAGS) $(LDLIBS)
	
#rendering/color.cc: rendering/color.cc.re
#	re2c -P $< -o$@

rendering/fonts/table-packer: rendering/fonts/table-packer.cc rendering/fonts/dijkstra.hh
	$(CXX) -std=c++17 -o "$@" "$<" -Ofast $(CPPFLAGS) -march=native -fopenmp

FONTS= \
	rendering/fonts/data/10x20.bdf \
	rendering/fonts/data/12x13ja.bdf \
	rendering/fonts/data/18x18ja.bdf \
	rendering/fonts/data/18x18ko.bdf \
	rendering/fonts/data/4x5.bdf \
	rendering/fonts/data/4x6.bdf \
	rendering/fonts/data/5x7.bdf \
	rendering/fonts/data/5x8.bdf \
	rendering/fonts/data/6x10.bdf \
	rendering/fonts/data/6x12.bdf \
	rendering/fonts/data/6x13.bdf \
	rendering/fonts/data/6x9.bdf \
	rendering/fonts/data/7x13.bdf \
	rendering/fonts/data/7x14.bdf \
	rendering/fonts/data/850-8x14.inc \
	rendering/fonts/data/850-8x16.inc \
	rendering/fonts/data/850-8x8.inc \
	rendering/fonts/data/852-8x14.inc \
	rendering/fonts/data/852-8x16.inc \
	rendering/fonts/data/852-8x8.inc \
	rendering/fonts/data/860-8x14.asm \
	rendering/fonts/data/860-8x16.asm \
	rendering/fonts/data/860-8x8.asm \
	rendering/fonts/data/863-8x14.asm \
	rendering/fonts/data/863-8x16.asm \
	rendering/fonts/data/863-8x8.asm \
	rendering/fonts/data/865-8x14.asm \
	rendering/fonts/data/865-8x16.asm \
	rendering/fonts/data/865-8x8.asm \
	rendering/fonts/data/8x10.inc \
	rendering/fonts/data/8x12.inc \
	rendering/fonts/data/8x13.bdf \
	rendering/fonts/data/8x14.inc \
	rendering/fonts/data/8x15.inc \
	rendering/fonts/data/8x16.inc \
	rendering/fonts/data/8x8.inc \
	rendering/fonts/data/9x15.bdf \
	rendering/fonts/data/9x18.bdf \
	rendering/fonts/data/Arabic-VGA14.psf.gz \
	rendering/fonts/data/Arabic-VGA16.psf.gz \
	rendering/fonts/data/Arabic-VGA28x16.psf.gz \
	rendering/fonts/data/Arabic-VGA32x16.psf.gz \
	rendering/fonts/data/Arabic-VGA8.psf.gz \
	rendering/fonts/data/cp857-8x14.psf.gz \
	rendering/fonts/data/cp857-8x16.psf.gz \
	rendering/fonts/data/cp857-8x8.psf.gz \
	rendering/fonts/data/FullCyrAsia-Terminus12x6.psf.gz \
	rendering/fonts/data/FullCyrAsia-TerminusBold20x10.psf.gz \
	rendering/fonts/data/FullCyrAsia-TerminusBold22x11.psf.gz \
	rendering/fonts/data/FullCyrAsia-TerminusBold24x12.psf.gz \
	rendering/fonts/data/FullCyrAsia-TerminusBold28x14.psf.gz \
	rendering/fonts/data/FullCyrAsia-TerminusBoldVGA16.psf.gz \
	rendering/fonts/data/FullCyrSlav-Terminus12x6.psf.gz \
	rendering/fonts/data/FullCyrSlav-TerminusBold20x10.psf.gz \
	rendering/fonts/data/FullCyrSlav-TerminusBold22x11.psf.gz \
	rendering/fonts/data/FullCyrSlav-TerminusBold24x12.psf.gz \
	rendering/fonts/data/FullCyrSlav-TerminusBold28x14.psf.gz \
	rendering/fonts/data/FullCyrSlav-VGA14.psf.gz \
	rendering/fonts/data/FullCyrSlav-VGA16.psf.gz \
	rendering/fonts/data/FullCyrSlav-VGA28x16.psf.gz \
	rendering/fonts/data/FullCyrSlav-VGA32x16.psf.gz \
	rendering/fonts/data/FullCyrSlav-VGA8.psf.gz \
	rendering/fonts/data/FullGreek-TerminusBold22x11.psf.gz \
	rendering/fonts/data/FullGreek-TerminusBold24x12.psf.gz \
	rendering/fonts/data/FullGreek-TerminusBold28x14.psf.gz \
	rendering/fonts/data/FullGreek-VGA14.psf.gz \
	rendering/fonts/data/FullGreek-VGA16.psf.gz \
	rendering/fonts/data/FullGreek-VGA28x16.psf.gz \
	rendering/fonts/data/FullGreek-VGA32x16.psf.gz \
	rendering/fonts/data/FullGreek-VGA8.psf.gz \
	rendering/fonts/data/Hebrew-VGA14.psf.gz \
	rendering/fonts/data/Hebrew-VGA16.psf.gz \
	rendering/fonts/data/Hebrew-VGA28x16.psf.gz \
	rendering/fonts/data/Hebrew-VGA32x16.psf.gz \
	rendering/fonts/data/Hebrew-VGA8.psf.gz \
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
	rendering/fonts/data/Uni1-VGA14.psf.gz \
	rendering/fonts/data/Uni1-VGA16.psf.gz \
	rendering/fonts/data/Uni1-VGA28x16.psf.gz \
	rendering/fonts/data/Uni1-VGA32x16.psf.gz \
	rendering/fonts/data/Uni1-VGA8.psf.gz \
	rendering/fonts/data/Uni2-Terminus12x6.psf.gz \
	rendering/fonts/data/Uni2-TerminusBold20x10.psf.gz \
	rendering/fonts/data/Uni2-TerminusBold22x11.psf.gz \
	rendering/fonts/data/Uni2-TerminusBold24x12.psf.gz \
	rendering/fonts/data/Uni2-TerminusBold28x14.psf.gz \
	rendering/fonts/data/Uni2-VGA14.psf.gz \
	rendering/fonts/data/Uni2-VGA16.psf.gz \
	rendering/fonts/data/Uni2-VGA28x16.psf.gz \
	rendering/fonts/data/Uni2-VGA32x16.psf.gz \
	rendering/fonts/data/Uni2-VGA8.psf.gz \
	rendering/fonts/data/Uni3-Terminus12x6.psf.gz \
	rendering/fonts/data/Uni3-TerminusBold20x10.psf.gz \
	rendering/fonts/data/Uni3-TerminusBold22x11.psf.gz \
	rendering/fonts/data/Uni3-TerminusBold24x12.psf.gz \
	rendering/fonts/data/Uni3-TerminusBold28x14.psf.gz \
	rendering/fonts/data/Uni3-TerminusBoldVGA14.psf.gz \
	rendering/fonts/data/Uni3-TerminusBoldVGA16.psf.gz \
	rendering/fonts/data/unifont-csur.bdf \
	rendering/fonts/data/vga8x19.bdf

rendering/fonts.inc: rendering/fonts/make.php \
		rendering/fonts/table-packer \
		rendering/fonts/read_font.php \
		rendering/fonts/font_gen2.php \
		$(FONTS)
	(cd rendering/fonts; php make.php    2>inc-files.dat \
		| sed 's@//.*@@' |grep . > ../fonts.inc)
	#(cd rendering/fonts; php make.php    2>inc-files.dat >/dev/null )

rendering/fonts-authentic.inc: rendering/fonts/make.php \
		rendering/fonts/table-packer \
		rendering/fonts/read_font.php \
		rendering/fonts/font_gen2.php \
		$(FONTS)
	(cd rendering/fonts; php make.php 1  2>inc-files2.dat \
		| sed 's@//.*@@' |grep . > ../fonts-authentic.inc)

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

compress1: ;
	parallel -j40 advdef -z4 -i16384 -- rendering/fonts/data/.translations-*
	for s in rendering/fonts/data/.translations-*;do ln "$$s" tmp.gz && DeflOpt tmp.gz;rm tmp.gz;done
compress2: ;
	parallel -j40 advdef -z4 -i16384 -- rendering/fonts/data/.1translations-*
	for s in rendering/fonts/data/.1translations-*;do ln "$$s" tmp.gz && DeflOpt tmp.gz;rm tmp.gz;done
compress12: ;
	parallel -j40 advdef -z4 -i16384 -- rendering/fonts/data/.translations-* rendering/fonts/data/.1translations-*
	for s in rendering/fonts/data/.1translations-* rendering/fonts/data/.translations-*;do ln "$$s" tmp.gz && DeflOpt tmp.gz;rm tmp.gz;done

compress3: ;
	parallel -j42 \
		sh -c 'for s in 1 2 3 4 5 6 7 8 9 10 11 12 13;do optipng -o7 $$0 ; pngout -c3 -n$$s $$0;done ; advpng -z4 -i16 $$0 ; DeflOpt $$0' \
		-- doc/coverage-*.png

-include $(addprefix .deps/,$(subst /,_,$(OBJS:.o=.d)))
