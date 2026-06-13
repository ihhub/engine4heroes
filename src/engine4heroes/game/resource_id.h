/***************************************************************************
 *   engine4heroes: https://github.com/ihhub/engine4heroes                 *
 *   Copyright (C) 2026                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#pragma once

#include <cstdint>

// This file exists by 2 reasons:
// - minimize the memory footprint in the code to avoid using const char * copies
// - using std::string to access data requires to avoid std::map or some other containers which don't guarantee the lifetime of objects
//
// TODO: find a better way of accessing game resources.

namespace ImageId
{
    enum : int32_t
    {
        NONE,
        LAYERS_MENU_MAIN_0800,
        LAYERS_MENU_MAIN_1024,
        LAYERS_MENU_MAIN_1280,
        BITMAP_RAW_MENU_MAIN_0800,
        BITMAP_RAW_MENU_MAIN_1024,
        BITMAP_RAW_MENU_MAIN_1280,
        ANIMATION_SAMPLE_CURSOR,

        FONT_PROSE_ANTIQUE_10,
        FONT_PROSE_ANTIQUE_12,
        FONT_PROSE_ANTIQUE_14,
        FONT_PROSE_ANTIQUE_16,
        FONT_PROSE_ANTIQUE_18,
        FONT_PROSE_ANTIQUE_20,
        FONT_PROSE_ANTIQUE_22,
        FONT_PROSE_ANTIQUE_24,
        FONT_PROSE_ANTIQUE_26,

        // WARNING!!!
        // Put all entries above this line.
        COUNT
    };
}

namespace AudioId
{
    enum : int32_t
    {
        NONE,
        MAIN_MENU,
        MISCELLANEOUS_BUTTON,

        // WARNING!!!
        // Put all entries above this line.
        COUNT
    };
}

namespace GameResource
{
    const char * getImageString( const int32_t id );

    const char * getAudioString( const int32_t id );
}
