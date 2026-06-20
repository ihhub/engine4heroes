###########################################################################
#   engine4heroes: https://github.com/ihhub/engine4heroes                 #
#   Copyright (C) 2026                                                    #
#                                                                         #
#   fheroes2: https://github.com/ihhub/fheroes2                           #
#   Copyright (C) 2021 - 2025                                             #
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
#   This program is distributed in the hope that it will be useful,       #
#   but WITHOUT ANY WARRANTY; without even the implied warranty of        #
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
#   GNU General Public License for more details.                          #
#                                                                         #
#   You should have received a copy of the GNU General Public License     #
#   along with this program; if not, write to the                         #
#   Free Software Foundation, Inc.,                                       #
#   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             #
###########################################################################

#
# Name of the main target executable file
#

TARGET := engine4heroes

#
# Common build options for engine4heroes and third-party libraries
#

# Common flags for both C and C++ compilers
CCFLAGS := -fsigned-char -pthread
# Flags for the C compiler
CFLAGS := $(CFLAGS)
# Flags for the C++ compiler
CXXFLAGS := $(CXXFLAGS)
# Flags for the preprocessor
CPPFLAGS := $(CPPFLAGS)
# Flags for the linker
LDFLAGS := $(LDFLAGS) -pthread
# Flags for additional libraries
LDLIBS := $(LDLIBS)

ifdef ENGINE4HEROES_WITH_DEBUG
CCFLAGS := $(CCFLAGS) -O0 -g
else
CCFLAGS := $(CCFLAGS) -O3
endif

ifneq ($(or $(ENGINE4HEROES_WITH_ASAN),$(ENGINE4HEROES_WITH_TSAN)),)
SANITIZERS := undefined

ifdef ENGINE4HEROES_WITH_ASAN
SANITIZERS := $(SANITIZERS),address
endif
ifdef ENGINE4HEROES_WITH_TSAN
SANITIZERS := $(SANITIZERS),thread
endif

CCFLAGS := $(CCFLAGS) -fsanitize=$(SANITIZERS)
LDFLAGS := $(LDFLAGS) -fsanitize=$(SANITIZERS)
endif

#
# Platform-specific build options
#

ifndef PLATFORM
ifndef OS
OS := $(shell uname)
endif

ifeq ($(OS),FreeBSD)
PLATFORM := bsd
endif
ifeq ($(OS),Darwin)
PLATFORM := osx
endif
ifeq ($(OS),Linux)
PLATFORM := all
endif
ifeq ($(OS),Haiku)
PLATFORM := all
endif
endif

include Makefile.$(PLATFORM)

#
# Build options for third-party libraries
#

CCFLAGS_TP := $(CCFLAGS)
CFLAGS_TP := $(CFLAGS)
CXXFLAGS_TP := $(CXXFLAGS)
CPPFLAGS_TP := $(CPPFLAGS)

#
# Build options for engine4heroes
#

# *FLAGS_FH2 can be passed from platform-specific Makefiles
CCFLAGS := $(CCFLAGS) $(CCFLAGS_FH2)
CFLAGS := $(CFLAGS) $(CFLAGS_FH2)
CXXFLAGS := $(CXXFLAGS) $(CXXFLAGS_FH2) -std=c++17
CPPFLAGS := $(CPPFLAGS) $(CPPFLAGS_FH2)

ifdef ENGINE4HEROES_WITH_SYSTEM_SMACKER
LIBS := $(LIBS) -lsmacker
endif
LIBS := $(LIBS) -lz $(LDLIBS)

ifdef ENGINE4HEROES_WITH_DEBUG
CCFLAGS := $(CCFLAGS) -DWITH_DEBUG
endif
ifdef ENGINE4HEROES_WITH_IMAGE
CCFLAGS := $(CCFLAGS) -DWITH_IMAGE
endif
ifdef ENGINE4HEROES_DATA
CCFLAGS := $(CCFLAGS) -DENGINE4HEROES_DATA="$(ENGINE4HEROES_DATA)"
endif

# TODO: Add -Wconversion -Wsign-conversion flags once we fix all the corresponding code smells
CCWARNOPTS := -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wdouble-promotion -Wfloat-conversion -Wfloat-equal \
              -Wredundant-decls -Wshadow -Wswitch-default -Wundef -Wunused
CFLAGS := $(CFLAGS) $(CCWARNOPTS)
CXXFLAGS := $(CXXFLAGS) $(CCWARNOPTS) -Wctor-dtor-privacy -Wextra-semi -Wmissing-declarations -Wold-style-cast \
                                      -Woverloaded-virtual -Wsuggest-override

ifdef ENGINE4HEROES_STRICT_COMPILATION
CCFLAGS := $(CCFLAGS) -Werror
endif

#
# SDL-related build options
#

# SDL_FLAGS can already be defined in a platform-specific Makefile
ifeq ($(origin SDL_FLAGS),undefined)
SDL_FLAGS := $(shell sdl2-config --cflags)

ifdef ENGINE4HEROES_WITH_IMAGE
SDL_FLAGS := $(SDL_FLAGS) $(shell libpng-config --cflags)
endif
endif

# SDL_LIBS can already be defined in a platform-specific Makefile
ifeq ($(origin SDL_LIBS),undefined)
SDL_LIBS := -lSDL2_mixer $(shell sdl2-config --libs)

ifdef ENGINE4HEROES_WITH_IMAGE
SDL_LIBS := $(SDL_LIBS) -lSDL2_image $(shell libpng-config --libs)
endif
endif

CCFLAGS := $(CCFLAGS) $(SDL_FLAGS)
LIBS := $(SDL_LIBS) $(LIBS)

export CC CXX AR CCFLAGS CFLAGS CXXFLAGS CPPFLAGS LDFLAGS LIBS TARGET

.PHONY: all clean

all:
	$(MAKE) -C engine
	$(MAKE) -C engine4heroes
ifdef FHEROES2_WITH_TOOLS
	$(MAKE) -C tools
endif

clean:
	$(MAKE) -C engine clean
	$(MAKE) -C engine4heroes clean
	$(MAKE) -C tools clean
