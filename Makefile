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

# Options:
#
# ENGINE4HEROES_STRICT_COMPILATION: build in strict compilation mode (turns warnings into errors)
# ENGINE4HEROES_WITH_DEBUG: build in debug mode
# ENGINE4HEROES_WITH_ASAN: build with UB Sanitizer and Address Sanitizer (small runtime overhead, incompatible with ENGINE4HEROES_WITH_TSAN)
# ENGINE4HEROES_WITH_TSAN: build with UB Sanitizer and Thread Sanitizer (large runtime overhead, incompatible with ENGINE4HEROES_WITH_ASAN)
# ENGINE4HEROES_WITH_IMAGE: build with SDL_image (requires libpng)
# ENGINE4HEROES_WITH_TOOLS: build additional tools
# ENGINE4HEROES_MACOS_APP_BUNDLE: create a Mac app bundle (only valid when building on macOS)
# ENGINE4HEROES_DATA: set the built-in path to the engine4heroes data directory (e.g. /usr/share/engine4heroes)

PROJECT_NAME := engine4heroes
PROJECT_VERSION := $(file < version.txt)

.PHONY: all clean

all:
	$(MAKE) -C src/dist
ifdef ENGINE4HEROES_MACOS_APP_BUNDLE
	mkdir -p engine4heroes.app/Contents/MacOS
	mkdir -p engine4heroes.app/Contents/Resources/translations
	cp src/dist/engine4heroes/engine4heroes engine4heroes.app/Contents/MacOS
	cp src/resources/engine4heroes.icns engine4heroes.app/Contents/Resources
	sed -e "s/\$${MACOSX_BUNDLE_BUNDLE_NAME}/$(PROJECT_NAME)/" \
	    -e "s/\$${MACOSX_BUNDLE_BUNDLE_VERSION}/$(PROJECT_VERSION)/" \
	    -e "s/\$${MACOSX_BUNDLE_EXECUTABLE_NAME}/engine4heroes/" \
	    -e "s/\$${MACOSX_BUNDLE_GUI_IDENTIFIER}/org.engine4heroes.$(PROJECT_NAME)/" \
	    -e "s/\$${MACOSX_BUNDLE_ICON_FILE}/engine4heroes.icns/" \
	    -e "s/\$${MACOSX_BUNDLE_SHORT_VERSION_STRING}/$(PROJECT_VERSION)/" src/resources/Info.plist.in > engine4heroes.app/Contents/Info.plist
	dylibbundler -od -b -x engine4heroes.app/Contents/MacOS/engine4heroes -d engine4heroes.app/Contents/libs
else
	cp src/dist/engine4heroes/engine4heroes .
endif

clean:
	$(MAKE) -C src/dist clean
	-rm -rf engine4heroes engine4heroes.app
