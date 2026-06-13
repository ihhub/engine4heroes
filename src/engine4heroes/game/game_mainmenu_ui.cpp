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

#include "game_mainmenu_ui.h"

#include <cassert>
#include <string>

#include "image.h"
#include "resource_id.h"
#include "resource_manager.h"
#include "screen.h"

namespace engine4heroes
{
    void drawMainMenuScreen()
    {
        Display & display = Display::instance();

        if ( display.width() == 800 && display.height() == 600 ) {
            const auto & mainMenu = GameResource::getImage( ImageId::BITMAP_RAW_MENU_MAIN_0800, 0 );
            engine4heroes::Copy( mainMenu, display );
        }
        else if ( display.width() == 1024 && display.height() == 768 ) {
            const auto & mainMenu = GameResource::getImage( ImageId::BITMAP_RAW_MENU_MAIN_1024, 0 );
            engine4heroes::Copy( mainMenu, display );
        }
        else if ( display.width() == 1280 && display.height() == 1024 ) {
            const auto & mainMenu = GameResource::getImage( ImageId::BITMAP_RAW_MENU_MAIN_1280, 0 );
            engine4heroes::Copy( mainMenu, display );
        }
        else {
            // Do something about it!
            assert( 0 );
        }
    }
}
