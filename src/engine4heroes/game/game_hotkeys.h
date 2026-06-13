/***************************************************************************
 *   engine4heroes: https://github.com/ihhub/engine4heroes                 *
 *   Copyright (C) 2026                                                    *
 *                                                                         *
 *   fheroes2: https://github.com/ihhub/fheroes2                           *
 *   Copyright (C) 2022 - 2026                                             *
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
#include <string>
#include <utility>
#include <vector>

namespace engine4heroes
{
    enum class Key : int32_t;
}

namespace Game
{
    enum class HotKeyEvent : int32_t
    {
        NONE,

        DEFAULT_OKAY,
        DEFAULT_CANCEL,
        DEFAULT_LEFT,
        DEFAULT_RIGHT,
        DEFAULT_UP,
        DEFAULT_DOWN,

        GLOBAL_TOGGLE_FULLSCREEN,
        GLOBAL_TOGGLE_TEXT_SUPPORT_MODE,

        // WARNING! Put all new event only above this line. No adding in between.
        NO_EVENT,
    };

    enum class HotKeyCategory : uint8_t
    {
        DEFAULT,
        GLOBAL,
    };

    bool HotKeyPressEvent( const HotKeyEvent eventID );
    bool HotKeyHoldEvent( const HotKeyEvent eventID );

    engine4heroes::Key getHotKeyForEvent( const HotKeyEvent eventID );
    void setHotKeyForEvent( const HotKeyEvent eventID, const engine4heroes::Key key );

    inline bool HotKeyCloseWindow()
    {
        return HotKeyPressEvent( HotKeyEvent::DEFAULT_CANCEL ) || HotKeyPressEvent( HotKeyEvent::DEFAULT_OKAY );
    }

    std::string getHotKeyNameByEventId( const HotKeyEvent eventID );

    const char * getHotKeyEventNameByEventId( const HotKeyEvent eventID );

    std::vector<std::pair<HotKeyEvent, HotKeyCategory>> getAllHotKeyEvents();

    void globalKeyDownEvent( const engine4heroes::Key key, const int32_t modifier );

    void HotKeysLoad( const std::string & filename );

    void HotKeySave();

    const char * getHotKeyCategoryName( const HotKeyCategory category );
}
