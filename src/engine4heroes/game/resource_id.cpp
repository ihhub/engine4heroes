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

#include "resource_id.h"

#include <array>

namespace
{
    struct ResourceInfo final
    {
        int32_t id{ 0 };
        const char * string{ nullptr };
    };

    const std::array<ResourceInfo, ImageId::COUNT> imageNames{ { { ImageId::NONE, "" },
                                                                 { ImageId::LAYERS_MENU_MAIN_0800, "layers.menu.main.0800.h4d" },
                                                                 { ImageId::LAYERS_MENU_MAIN_1024, "layers.menu.main.1024.h4d" },
                                                                 { ImageId::LAYERS_MENU_MAIN_1280, "layers.menu.main.1280.h4d" },
                                                                 { ImageId::BITMAP_RAW_MENU_MAIN_0800, "bitmap_raw.menu.main.0800.h4d" },
                                                                 { ImageId::BITMAP_RAW_MENU_MAIN_1024, "bitmap_raw.menu.main.1024.h4d" },
                                                                 { ImageId::BITMAP_RAW_MENU_MAIN_1280, "bitmap_raw.menu.main.1280.h4d" },
                                                                 { ImageId::ANIMATION_SAMPLE_CURSOR, "animation.sample_cursor.h4d" },
                                                                 { ImageId::FONT_PROSE_ANTIQUE_10, "font.prose_antique.10.h4d" },
                                                                 { ImageId::FONT_PROSE_ANTIQUE_12, "font.prose_antique.12.h4d" },
                                                                 { ImageId::FONT_PROSE_ANTIQUE_14, "font.prose_antique.14.h4d" },
                                                                 { ImageId::FONT_PROSE_ANTIQUE_16, "font.prose_antique.16.h4d" },
                                                                 { ImageId::FONT_PROSE_ANTIQUE_18, "font.prose_antique.18.h4d" },
                                                                 { ImageId::FONT_PROSE_ANTIQUE_20, "font.prose_antique.20.h4d" },
                                                                 { ImageId::FONT_PROSE_ANTIQUE_22, "font.prose_antique.22.h4d" },
                                                                 { ImageId::FONT_PROSE_ANTIQUE_24, "font.prose_antique.24.h4d" },
                                                                 { ImageId::FONT_PROSE_ANTIQUE_26, "font.prose_antique.26.h4d" } } };

    const std::array<ResourceInfo, AudioId::COUNT> audioNames{ {
        { AudioId::NONE, "" },
        { AudioId::MAIN_MENU, "sound.main_menu.h4d" },
        { AudioId::MISCELLANEOUS_BUTTON, "sound.miscellaneous.button.h4d" },
    } };
}

namespace GameResource
{
    const char * getImageString( const int32_t id )
    {
        if ( id >= ImageId::COUNT || id <= ImageId::NONE ) {
            return "";
        }

        return imageNames[id].string;
    }

    const char * getAudioString( const int32_t id )
    {
        if ( id >= AudioId::COUNT || id <= AudioId::NONE ) {
            return "";
        }

        return audioNames[id].string;
    }
}
