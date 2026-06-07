/***************************************************************************
 *   engine4heroes: https://github.com/ihhub/engine4heroes                 *
 *   Copyright (C) 2026                                                    *
 *                                                                         *
 *   engine4heroes: https://github.com/ihhub/engine4heroes                 *
 *   Copyright (C) 2026                                                    *
 *                                                                         *
 *   fheroes2: https://github.com/ihhub/fheroes2                           *
 *   Copyright (C) 2019 - 2026                                             *
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

#include <cassert>
#include <cstdint>

#include "math_base.h"
#include "screen.h"

namespace engine4heroes
{
    class Image;
}

class Cursor
{
public:
    enum CursorType : int
    {
        CUSTOM,
        POINTER,

        // WARNING!!!
        // Put all new entries above this line.
        COUNT,
    };

    Cursor( const Cursor & ) = delete;

    Cursor & operator=( const Cursor & ) = delete;

    static Cursor & Get();

    // Returns a non-empty area if it should be updated while rendering.
    static engine4heroes::Rect updateCursorPosition( const int32_t x, const int32_t y );

    static void Refresh();

    int Themes() const
    {
        return _theme;
    }

    void SetThemes( const int theme, const bool force = false );

    void setCustomImage( const engine4heroes::Image & image, const engine4heroes::Point & offset );

private:
    Cursor() = default;
    ~Cursor() = default;

    void SetOffset( const int name, const engine4heroes::Point & defaultOffset );
    void Move( int32_t x, int32_t y ) const;

    int _theme{ Cursor::CursorType::CUSTOM };

    engine4heroes::Point _offset;
};

class CursorRestorer
{
public:
    CursorRestorer() = default;
    // Use only to hide a cursor. Previous cursor visibility and theme will be restored when this object is destroyed.
    explicit CursorRestorer( const bool visible );
    // Use to set cursor visibility and theme of the cursor. Previous cursor visibility and theme will be restored when this object is destroyed.
    CursorRestorer( const bool visible, const int theme );
    CursorRestorer( const CursorRestorer & ) = delete;

    ~CursorRestorer();

    CursorRestorer & operator=( const CursorRestorer & ) = delete;

private:
    const int _theme{ Cursor::Get().Themes() };
    const bool _visible{ engine4heroes::cursor().isVisible() };
};
