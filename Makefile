CXX=g++
CPPFLAGS=-Wall -Wextra
CXXFLAGS=-std=c++20

GCCVERS=$(shell $(CXX) --version|head -n1|sed 's/.* //;s/\..*//')

OPTIM_BUILD = -fopenmp -Ofast
OPTIM_DEBUG = -Og -g -fsanitize=address
OPTIM_GPROF = -Og -g -pg
OPTIM_GCOV  = -Og -g -fprofile-arcs -ftest-coverage
OPTIM_TEST  = -Og -g -DRUN_TESTS -Igoogletest/googletest/include -fprofile-arcs -ftest-coverage

CPPFLAGS += -Isrc/rendering -Isrc/tty -Isrc -Isrc/rendering/fonts -Isrc/file -Isrc/util/TinyDeflate -Isrc/util

#CPPFLAGS += -fanalyzer

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

# NOTE: The order of object files in this list also determines
#       the order of tests run by gtest.
OBJS=\
	main.o \
	ctype.o \
	autoinput.o \
	clock.o \
	keysym.o \
	file/share.o \
	tty/forkpty.o \
	tty/256color.o \
	tty/terminal.o \
	rendering/color.o \
	rendering/cset.o \
	rendering/fonts/make_similarities.o \
	rendering/fonts/read_fonts.o \
	rendering/fonts/read_font.o \
	rendering/fonts/font_planner.o \
	rendering/person.o \
	rendering/window.o \
	ui.o

GTEST := googletest/build/lib/libgtest.a googletest/build/lib/libgtest_main.a

OBJS_BUILD := $(foreach o,$(OBJS),obj/build/$(notdir $(o)))
OBJS_DEBUG := $(foreach o,$(OBJS),obj/debug/$(notdir $(o)))
OBJS_GPROF := $(foreach o,$(OBJS),obj/gprof/$(notdir $(o)))
OBJS_GCOV := $(foreach o,$(OBJS),obj/gcov/$(notdir $(o)))
OBJS_TEST := $(foreach o,$(OBJS),obj/test/$(notdir $(o)))

OBJS_TEST := $(filter-out obj/test/main.o,$(OBJS_TEST))

all: term term_gcov term_gprof term_debug term_test ;

term: $(OBJS_BUILD)
	$(CXX) -o "$@" $^ $(CXXFLAGS) $(LDLIBS) $(OPTIM_BUILD)
term_gcov: $(OBJS_GCOV)
	$(CXX) -o "$@" $^ $(CXXFLAGS) $(LDLIBS) $(OPTIM_GCOV) -pthread
term_gprof: $(OBJS_GPROF)
	$(CXX) -o "$@" $^ $(CXXFLAGS) $(LDLIBS) $(OPTIM_GPROF) -pthread
term_debug: $(OBJS_DEBUG)
	$(CXX) -o "$@" $^ $(CXXFLAGS) $(LDLIBS) $(OPTIM_DEBUG) -pthread
term_test: $(OBJS_TEST) $(GTEST)
	$(CXX) -o "$@" $^ $(CXXFLAGS) $(LDLIBS) $(OPTIM_TEST) -pthread $(GTEST)

test: term_test
	#- rm $$HOME/.cache/that_terminal/* || \
	#  rm /home/$$USER/.cache/that_terminal/* || \
	#  rm /run/user/`id -u`/{similarities.dat,fonts-list.dat}
	- rm obj/test/*.gcda
	./term_test | tee doc/test-report.txt
	- lcov --base-directory . --no-external -d obj/test --output-file main_coverage.info \
	     -c --gcov-tool "gcov-$(GCCVERS)" \
	     	--exclude '*gtest*' \
	     	--exclude '*gunzip*' \
	     	--exclude '$(GCCVERS)/*' \
	     --rc "lcov_branch_coverage=1" \
	     --rc "lcov_excl_br_line=LCOV_EXCL_BR_LINE|EXPECT_*|ASSERT_*|usleep|TEST\("
	- genhtml main_coverage.info --branch-coverage --output-directory doc/doxygen/docs/cov

define create_rule
obj/$(1)/$(notdir $(o)): $(foreach s,$(subst .o,.cc,$(o)),src/$(s))
	$$(CXX) -c -o "$$@" "$$<" $$(CXXFLAGS) $$(CPPFLAGS) $(2)

endef
$(eval $(foreach o,$(OBJS),$(call create_rule,build,$(OPTIM_BUILD))))
$(eval $(foreach o,$(OBJS),$(call create_rule,debug,$(OPTIM_DEBUG))))
$(eval $(foreach o,$(OBJS),$(call create_rule,gprof,$(OPTIM_GPROF))))
$(eval $(foreach o,$(OBJS),$(call create_rule,gcov,$(OPTIM_GCOV))))
$(eval $(foreach o,$(OBJS),$(call create_rule,test,$(OPTIM_TEST))))

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

doxygen: Doxyfile
	doxygen $<
	test -d doc/doxygen/docs/doc || ln -sf ../.. doc/doxygen/docs/doc

compress3:
	parallel -j45 util/compressor.sh -- \
	`echo \
		doc/coverage-*.png \
	`

$(GTEST):
	cmake -S googletest -B googletest/build
	- $(MAKE) -C googletest/build
	$(MAKE) -C googletest/build

-include $(addprefix .deps/,$(subst /,_,$(OBJS_BUILD:.o=.d)))
-include $(addprefix .deps/,$(subst /,_,$(OBJS_DEBUG:.o=.d)))
-include $(addprefix .deps/,$(subst /,_,$(OBJS_GPROF:.o=.d)))
-include $(addprefix .deps/,$(subst /,_,$(OBJS_GCOV:.o=.d)))
-include $(addprefix .deps/,$(subst /,_,$(OBJS_TEST:.o=.d)))

clean:
	rm -f term term_debug term_gprof term_gcov term_test
	rm -f $(OBJS_BUILD) $(OBJS_DEBUG) $(OBJS_GPROF) $(OBJS_GCOV) $(OBJS_TEST)
	- rm -f obj/gcov/*.gcno obj/gcov/*.gcda
	- rm -f obj/test/*.gcno obj/test/*.gcda
	rm -rf .deps/obj*.d
