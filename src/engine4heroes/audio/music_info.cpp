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

#include "music_info.h"

#include <array>
#include <cassert>
#include <string_view>

namespace
{
    constexpr std::array<std::string_view, Music::MusicTrack::COUNT> trackString{ "", "sound.main_menu.h4d" };
}

namespace Music
{
    std::string getMusicTrackString( const int32_t track )
    {
        if ( track <= UNKNOWN || track >= COUNT ) {
            assert( 0 );
            return {};
        }

        return std::string( trackString[track] );
    }
}
