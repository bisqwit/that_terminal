CXX=g++
CPPFLAGS=-Wall -Wextra
CXXFLAGS=-std=c++20

CXXFLAGS += -Ofast
#CXXFLAGS += -Og -g -fsanitize=address

CXXFLAGS += -fopenmp

CPPFLAGS += -Irendering -Itty -I. -Irendering/fonts -Ifile -Iutil/TinyDeflate -Iutil

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

OBJS=\
	tty/terminal.o \
	tty/forkpty.o \
	tty/256color.o \
	rendering/screen.o \
	rendering/person.o \
	rendering/color.o \
	rendering/cset.o \
	rendering/font.o \
	rendering/fonts/make_similarities.o \
	rendering/fonts/read_fonts.o \
	rendering/fonts/read_font.o \
	file/share.o \
	beeper.o \
	main.o \
	ctype.o \
	autoinput.o \
	clock.o

term: $(OBJS)
	$(CXX) -o $@ $(OBJS) $(CXXFLAGS) $(LDLIBS)
	
#rendering/color.cc: rendering/color.cc.re
#	re2c -P $< -o$@

rendering/fonts/table-packer: rendering/fonts/table-packer.o ctype.o rendering/fonts/dijkstra.hh
	$(CXX) -std=c++17 -o "$@" "$<" ctype.o -Ofast $(CPPFLAGS) -march=native -fopenmp



# IBM fonts (CGA/EGA/VGA)
IBMFONTS= \
	rendering/fonts/data/ib8x8u.bdf \
	rendering/fonts/data/ie8x14u.bdf \
	rendering/fonts/data/iv8x16u.bdf \
	rendering/fonts/data/vga8x19.bdf

# Mona fonts. License: PD
MONAFONTS= \
	rendering/fonts/data/mona6x12r.bdf \
	rendering/fonts/data/mona7x14r.bdf \
	rendering/fonts/data/monak12.bdf \
	rendering/fonts/data/monak14.bdf

# Unifont: Copyright (C) 1998-2019 Roman Czyborra, Paul Hardy, Qianqian Fang, Andrew Miller, Johnnie Weaver,
# David Corbett, et al. License GPLv2+: GNU GPL version 2 or later <http://gnu.org/licenses/gpl.html>
# with the GNU Font Embedding Exception.
UNIFONTS= \
	rendering/fonts/data/unifont-csur.bdf

# CMEX: Dynalab Ming, modified by 趙惟倫and 趙惟倫, PD
# GB: Song Ti, Copyright (c) 1988  The Institute of Software, Academia Sinica.
CHINESEFONTS= \
	rendering/fonts/data/cmex24m.bdf \
	rendering/fonts/data/gb16st.bdf \
	rendering/fonts/data/gb24st.bdf

# Misaki: Misaki Gothic, Copyright (C) 2002-2019 Num Kadoma. Unlimited permission/no warranty.
JAPANESEFONTS= \
	rendering/fonts/data/misakig2.bdf \
	rendering/fonts/data/f12.bdf \
	rendering/fonts/data/f14.bdf

# Traditional Linux 8-bit consolefonts.
CONSOLEFONTS_OLD= \
	rendering/fonts/data/iso01.f08.psf.gz \
	rendering/fonts/data/iso01.f14.psf.gz \
	rendering/fonts/data/iso01.f16.psf.gz \
	rendering/fonts/data/iso07.f08.psf.gz \
	rendering/fonts/data/iso07.f14.psf.gz \
	rendering/fonts/data/iso07.f16.psf.gz \
	rendering/fonts/data/iso08.f08.psf.gz \
	rendering/fonts/data/iso08.f14.psf.gz \
	rendering/fonts/data/iso08.f16.psf.gz \
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
	rendering/fonts/data/lat4-10.psf.gz \
	rendering/fonts/data/lat4-12.psf.gz \
	rendering/fonts/data/lat9-10.psf.gz \
	rendering/fonts/data/lat9-12.psf.gz

# Unicode-aware Linux consolefonts.
CONSOLEFONTS_NEW= \
	rendering/fonts/data/Arabic-VGA16.psf.gz \
	rendering/fonts/data/Arabic-VGA32x16.psf.gz \
	rendering/fonts/data/Arabic-VGA8.psf.gz \
	rendering/fonts/data/CyrKoi-Terminus28x14.psf.gz \
	rendering/fonts/data/FullCyrAsia-Terminus22x11.psf.gz \
	rendering/fonts/data/FullCyrAsia-TerminusBold22x11.psf.gz \
	rendering/fonts/data/FullCyrAsia-TerminusBold24x12.psf.gz \
	rendering/fonts/data/FullCyrAsia-TerminusBold28x14.psf.gz \
	rendering/fonts/data/FullCyrAsia-TerminusBoldVGA16.psf.gz \
	rendering/fonts/data/FullCyrSlav-Terminus20x10.psf.gz \
	rendering/fonts/data/FullCyrSlav-TerminusBold22x11.psf.gz \
	rendering/fonts/data/FullCyrSlav-TerminusBold24x12.psf.gz \
	rendering/fonts/data/FullCyrSlav-TerminusBold28x14.psf.gz \
	rendering/fonts/data/FullCyrSlav-VGA16.psf.gz \
	rendering/fonts/data/FullCyrSlav-VGA28x16.psf.gz \
	rendering/fonts/data/FullCyrSlav-VGA32x16.psf.gz \
	rendering/fonts/data/FullCyrSlav-VGA8.psf.gz \
	rendering/fonts/data/FullGreek-Terminus14.psf.gz \
	rendering/fonts/data/FullGreek-Terminus32x16.psf.gz \
	rendering/fonts/data/FullGreek-VGA28x16.psf.gz \
	rendering/fonts/data/FullGreek-VGA8.psf.gz \
	rendering/fonts/data/Hebrew-VGA16.psf.gz \
	rendering/fonts/data/Hebrew-VGA32x16.psf.gz \
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
	rendering/fonts/data/Uni3-Terminus24x12.psf.gz \
	rendering/fonts/data/Uni3-TerminusBold22x11.psf.gz \
	rendering/fonts/data/Uni3-TerminusBold24x12.psf.gz \
	rendering/fonts/data/Uni3-TerminusBold28x14.psf.gz \
	rendering/fonts/data/Uni3-TerminusBoldVGA14.psf.gz \
	rendering/fonts/data/Uni3-TerminusBoldVGA16.psf.gz

# Xorg Misc-Fixed fonts, PD
# Technically 4x5 (micro) and 4x6 are not part of misc-fixed.
MISCFIXEDFONTS = \
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
	rendering/fonts/data/8x13.bdf \
	rendering/fonts/data/9x15.bdf \
	rendering/fonts/data/9x18.bdf \
	rendering/fonts/data/10x20.bdf \
	rendering/fonts/data/12x13ja.bdf \
	rendering/fonts/data/18x18ja.bdf \
	rendering/fonts/data/18x18ko.bdf


FONTS= \
	$(IBMFONTS) \
	$(MISCFIXEDFONTS) \
	$(MONAFONTS) \
	$(UNIFONTS) \
	$(CONSOLEFONTS_OLD) \
	$(CONSOLEFONTS_NEW) \
	$(CHINESEFONTS) \
	$(JAPANESEFONTS) \

rendering/fonts/similarities.dat: rendering/fonts/make-similarities.php rendering/fonts/UnicodeData.txt
	(cd rendering/fonts; php make-similarities.php > similarities.dat)
rendering/fonts/similarities.inc: rendering/fonts/make-similarities2.php rendering/fonts/similarities.dat
	(cd rendering/fonts; php make-similarities2.php > similarities.inc)

define dummy
rendering/fonts.inc: rendering/fonts/make.php rendering/fonts/similarities.inc \
		rendering/fonts/table-packer \
		rendering/fonts/read_font.php \
		rendering/fonts/font_gen2.php \
		$(FONTS)
	(cd rendering/fonts; php make.php    2>inc-files.dat \
		| sed 's@//.*@@' |grep . > ../fonts.inc)
	#(cd rendering/fonts; php make.php    2>inc-files.dat >/dev/null )

rendering/fonts-authentic.inc: rendering/fonts/make.php rendering/fonts/similarities.inc \
		rendering/fonts/table-packer \
		rendering/fonts/read_font.php \
		rendering/fonts/font_gen2.php \
		$(FONTS)
	(cd rendering/fonts; php make.php 1  2>inc-files2.dat \
		| sed 's@//.*@@' |grep . > ../fonts-authentic.inc)
endef

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

compress12:
	parallel -j45 util/compressor.sh -- \
	`echo \
		rendering/fonts/data/.tran-* \
		rendering/fonts/data/.1tran-* \
	`
compress3:
	parallel -j45 util/compressor.sh -- \
	`echo \
		doc/coverage-*.png \
	`
compress123:
	parallel -j45 util/compressor.sh -- \
	`echo \
		rendering/fonts/data/.tran-*monak12* \
		rendering/fonts/data/.1tran-*monak12* \
		doc/coverage-*12x12*.png \
		doc/coverage-*14x14*.png \
	`

-include $(addprefix .deps/,$(subst /,_,$(OBJS:.o=.d)))
