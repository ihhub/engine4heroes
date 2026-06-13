/***************************************************************************
 *   engine4heroes: https://github.com/ihhub/engine4heroes                 *
 *   Copyright (C) 2026                                                    *
 *                                                                         *
 *   fheroes2: https://github.com/ihhub/fheroes2                           *
 *   Copyright (C) 2020 - 2025                                             *
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
#include <memory>
#include <utility>

#include "math_base.h"

namespace engine4heroes
{
    class Image
    {
    public:
        Image() = default;

        Image( const int32_t width, const int32_t height )
        {
            Image::resize( width, height );
        }

        Image( const Image & image )
        {
            copy( image );
        }

        Image( Image && image ) noexcept;

        virtual ~Image() = default;

        Image & operator=( const Image & image );
        Image & operator=( Image && image ) noexcept;

        virtual void resize( const int32_t width_, const int32_t height_ );

        void reset(); // makes image fully transparent (transform layer is set to 1)

        // It's safe to cast to uint32_t as width and height are always >= 0
        int32_t width() const
        {
            return _width;
        }

        int32_t height() const
        {
            return _height;
        }

        uint32_t * image();

        const uint32_t * image() const;

        bool empty() const
        {
            return !_data;
        }

        void fill( const uint32_t value );

        void clear(); // makes the image empty

    private:
        void copy( const Image & image );

        int32_t _width{ 0 };
        int32_t _height{ 0 };
        std::unique_ptr<uint32_t[]> _data;
    };

    class Sprite : public Image
    {
    public:
        Sprite() = default;
        Sprite( const int32_t width, const int32_t height, const int32_t x = 0, const int32_t y = 0 )
            : Image( width, height )
            , _x( x )
            , _y( y )
        {
            // Do nothing.
        }

        explicit Sprite( const Image & image, const int32_t x = 0, const int32_t y = 0 )
            : Image( image )
            , _x( x )
            , _y( y )
        {
            // Do nothing.
        }

        explicit Sprite( Image && image, const int32_t x = 0, const int32_t y = 0 )
            : Image( std::move( image ) )
            , _x( x )
            , _y( y )
        {
            // Do nothing.
        }

        Sprite( const Sprite & sprite ) = default;
        Sprite( Sprite && sprite ) noexcept;

        ~Sprite() override = default;

        Sprite & operator=( const Sprite & sprite );
        Sprite & operator=( Sprite && sprite ) noexcept;

        Sprite & operator=( Image && image ) noexcept;

        int32_t x() const
        {
            return _x;
        }

        int32_t y() const
        {
            return _y;
        }

        virtual void setPosition( const int32_t x_, const int32_t y_ );

    private:
        int32_t _x{ 0 };
        int32_t _y{ 0 };
    };

    // This class is used in situations when we draw a window within another window
    class ImageRestorer final
    {
    public:
        explicit ImageRestorer( Image & image );
        ImageRestorer( Image & image, const int32_t x_, const int32_t y_, const int32_t width, const int32_t height );

        ImageRestorer( const ImageRestorer & ) = delete;
        ImageRestorer & operator=( const ImageRestorer & ) = delete;

        // Restores the original image if necessary, see the implementation for details
        ~ImageRestorer()
        {
            if ( !_isRestored ) {
                restore();
            }
        }

        void update( const int32_t x_, const int32_t y_, const int32_t width, const int32_t height );

        int32_t x() const
        {
            return _x;
        }

        int32_t y() const
        {
            return _y;
        }

        int32_t width() const
        {
            return _width;
        }

        int32_t height() const
        {
            return _height;
        }

        Rect rect() const
        {
            return { _x, _y, _width, _height };
        }

        void restore();

        void reset()
        {
            _isRestored = true;
        }

    private:
        Image & _image;
        Image _copy;

        int32_t _x{ 0 };
        int32_t _y{ 0 };
        int32_t _width{ 0 };
        int32_t _height{ 0 };

        void _updateRoi();

        bool _isRestored{ false };
    };

    // Draw one image on another taking into account the transparency and shadows data in the transform layer.
    void Blit( const Image & in, Image & out, const bool flip = false );
    void Blit( const Image & in, Image & out, const Rect & outRoi, const bool flip = false );
    void Blit( const Image & in, Image & out, const int32_t outX, const int32_t outY, const bool flip = false );
    void Blit( const Image & in, int32_t inX, int32_t inY, Image & out, int32_t outX, int32_t outY, int32_t width, int32_t height, const bool flip = false );

    // inPos must contain non-negative values
    void Blit( const Image & in, const Point & inPos, Image & out, const Point & outPos, const Size & size, bool flip = false );

    void Copy( const Image & in, Image & out );
    void Copy( const Image & in, const int32_t inX, const int32_t inY, Image & out, const Rect & outRoi );
    void Copy( const Image & in, int32_t inX, int32_t inY, Image & out, int32_t outX, int32_t outY, int32_t width, int32_t height );

    Sprite Crop( const Image & image, int32_t x, int32_t y, int32_t width, int32_t height );

    void Fill( Image & image, int32_t x, int32_t y, int32_t width, int32_t height, const uint32_t color );
}
