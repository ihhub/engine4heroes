###########################################################################
#   engine4heroes: https://github.com/ihhub/engine4heroes                 #
#   Copyright (C) 2026                                                    #
#                                                                         #
#   fheroes2: https://github.com/ihhub/fheroes2                           #
#   Copyright (C) 2022 - 2025                                             #
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

cmake_minimum_required(VERSION 3.24)
cmake_policy(SET CMP0074 NEW)

file(STRINGS ${CMAKE_SOURCE_DIR}/version.txt ENGINE4HEROES_VERSION LIMIT_COUNT 1 REGEX "^[0-9]+\.[0-9]+\.[0-9]+$")

project(engine4heroes VERSION ${ENGINE4HEROES_VERSION} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CMakeDependentOption)
include(GNUInstallDirs)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

#
# Compile-time options
#
option(ENABLE_IMAGE "Enable the use of SDL_image (requires libpng)" OFF)
option(ENABLE_TOOLS "Enable the build of additional tools" OFF)

# Available only on macOS
cmake_dependent_option(MACOS_APP_BUNDLE "Create a Mac app bundle" OFF "APPLE" OFF)

set(USE_SDL_VERSION SDL2 CACHE STRING "Version of the SDL library used")
set(ENGINE4HEROES_DATA ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME} CACHE STRING "Path to the engine4heroes data directory (relative to CMAKE_INSTALL_PREFIX)")

# For ComboBox in CMake GUI
set_property(CACHE USE_SDL_VERSION PROPERTY STRINGS SDL2)

#
# Library & feature detection
#
find_package(${USE_SDL_VERSION} REQUIRED)
find_package(${USE_SDL_VERSION}_mixer REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Threads)

if(ENABLE_IMAGE)
	find_package(${USE_SDL_VERSION}_image REQUIRED)
	find_package(PNG REQUIRED)
endif(ENABLE_IMAGE)

#
# Source files
#
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(src)

#
# macOS app bundle
#
if(MACOS_APP_BUNDLE)
	add_custom_target(
		run_dylibbundler
		ALL
		COMMAND dylibbundler -od -b -x ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.app/Contents/MacOS/${PROJECT_NAME} -d ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.app/Contents/libs
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		)
endif(MACOS_APP_BUNDLE)
