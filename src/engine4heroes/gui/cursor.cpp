/***************************************************************************
 *   engine4heroes: https://github.com/ihhub/engine4heroes                 *
 *   Copyright (C) 2026                                                    *
 *                                                                         *
 *   fheroes2: https://github.com/ihhub/fheroes2                           *
 *   Copyright (C) 2019 - 2026                                             *
 *                                                                         *
 *   Free Heroes2 Engine: http://sourceforge.net/projects/fheroes2         *
 *   Copyright (C) 2009 by Andrey Afletdinov <fheroes2@gmail.com>          *
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

#include "cursor.h"

#include <array>

#include "image.h"
#include "localevent.h"
#include "resource_id.h"
#include "resource_manager.h"

namespace
{
    constexpr std::array<int32_t, Cursor::COUNT> cursorResource{ { ImageId::NONE, ImageId::ANIMATION_SAMPLE_CURSOR } };
}

Cursor & Cursor::Get()
{
    static Cursor _cursor;
    return _cursor;
}

void Cursor::SetThemes( const int theme, const bool force )
{
    if ( _theme == theme && !force ) {
        return;
    }

    if ( theme <= CUSTOM || theme >= COUNT ) {
        return;
    }

    _theme = theme;

    const auto & image = GameResource::getImage( cursorResource[theme], 0 );

    SetOffset( theme, { ( image.width() - image.x() ) / 2, ( image.height() - image.y() ) / 2 } );

    engine4heroes::Cursor & cursor = engine4heroes::cursor();
    cursor.update( image, -_offset.x, -_offset.y );

    // Apply new offset.
    const engine4heroes::Point & currentPos = LocalEvent::Get().getMouseCursorPos();
    Move( currentPos.x, currentPos.y );
}

void Cursor::setCustomImage( const engine4heroes::Image & image, const engine4heroes::Point & offset )
{
    _theme = CUSTOM;

    engine4heroes::Cursor & cursor = engine4heroes::cursor();
    cursor.update( image, -offset.x, -offset.y );
    cursor.keepInScreenArea( false );

    // Immediately apply new mouse offset.
    const engine4heroes::Point & currentPos = LocalEvent::Get().getMouseCursorPos();
    _offset = offset;

    Move( currentPos.x, currentPos.y );
}

engine4heroes::Rect Cursor::updateCursorPosition( const int32_t x, const int32_t y )
{
    if ( engine4heroes::cursor().isSoftwareEmulation() ) {
        Cursor::Get().Move( x, y );
        if ( engine4heroes::cursor().isVisible() ) {
            return { x, y, 1, 1 };
        }
    }

    return {};
}

void Cursor::Move( int32_t x, int32_t y ) const
{
    engine4heroes::cursor().setPosition( x + _offset.x, y + _offset.y );
}

void Cursor::SetOffset( const int name, const engine4heroes::Point & defaultOffset )
{
    switch ( name ) {
    case Cursor::POINTER:
        _offset = { 0, 0 };
        break;
    default:
        _offset = { -defaultOffset.x, -defaultOffset.y };
        break;
    }
}

void Cursor::Refresh()
{
    Get().SetThemes( Get().Themes(), true );
}

CursorRestorer::CursorRestorer( const bool visible )
{
    engine4heroes::cursor().show( visible );
}

CursorRestorer::CursorRestorer( const bool visible, const int theme )
{
    Cursor::Get().SetThemes( theme );

    engine4heroes::cursor().show( visible );
}

CursorRestorer::~CursorRestorer()
{
    engine4heroes::Cursor & cursorRenderer = engine4heroes::cursor();

    const bool isShown = _visible && !cursorRenderer.isVisible();

    cursorRenderer.show( _visible );

    Cursor & cursor = Cursor::Get();

    const bool noThemeChange = ( cursor.Themes() == _theme );

    cursor.SetThemes( _theme );

    // In case of software emulated cursor when cursor theme is not changed and it is shown after it was hidden
    // we force render the cursor area. It is needed to reduce the cursor show delay.
    if ( isShown && noThemeChange && cursorRenderer.isSoftwareEmulation() ) {
        const engine4heroes::Point & pos = LocalEvent::Get().getMouseCursorPos();
        engine4heroes::Display::instance().render( { pos.x, pos.y, 1, 1 } );
    }
}
