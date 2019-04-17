CXX=gcc-8
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
LDLIBS   += -lutil -lc++ -lncurses -lstdc++

OBJS=\
	tty/terminal.o \
	tty/forkpty.o \
	tty/256color.o \
	rendering/screen.o \
	rendering/person.o \
	rendering/color.o \
	beeper.o \
	main.o \
	ctype.o

all: $(OBJS)
	$(CXX) -o $@ $(OBJS) $(CXXFLAGS) $(LDLIBS)
	
#rendering/color.cc: rendering/color.cc.re
#	re2c -P $< -o$@

-include $(addprefix .deps/,$(subst /,_,$(OBJS:.o=.d)))
