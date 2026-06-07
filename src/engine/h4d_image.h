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

#include <array>
#include <cstdint>
#include <memory>
#include <utility>

namespace engine4heroes
{
    struct Color final
    {
        uint8_t red{ 0 };
        uint8_t green{ 0 };
        uint8_t blue{ 0 };
    };

    class PackedImage
    {
    public:
        PackedImage() = default;

        PackedImage( const int32_t width, const int32_t height )
        {
            PackedImage::resize( width, height );
        }

        PackedImage( const PackedImage & image )
        {
            copy( image );
        }

        PackedImage( PackedImage && image ) noexcept;

        virtual ~PackedImage() = default;

        PackedImage & operator=( const PackedImage & image );
        PackedImage & operator=( PackedImage && image ) noexcept;

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

        virtual uint8_t * image();

        virtual const uint8_t * image() const;

        bool empty() const
        {
            return !_data;
        }

        void clear(); // makes the image empty

        // Fill 'image' layer with given value, setting 'transform' layer to 0.
        void fill( const uint8_t value );

        void setPalette( std::array<Color, 256> palette )
        {
            _palette = std::move( palette );
        }

        const std::array<Color, 256> & palette() const
        {
            return _palette;
        }

    private:
        void copy( const PackedImage & image );

        int32_t _width{ 0 };
        int32_t _height{ 0 };
        std::unique_ptr<uint8_t[]> _data;
        std::array<Color, 256> _palette;
    };

    class PackedSprite : public PackedImage
    {
    public:
        PackedSprite() = default;
        PackedSprite( const int32_t width, const int32_t height, const int32_t x = 0, const int32_t y = 0 )
            : PackedImage( width, height )
            , _x( x )
            , _y( y )
        {
            // Do nothing.
        }

        explicit PackedSprite( const PackedImage & image, const int32_t x = 0, const int32_t y = 0 )
            : PackedImage( image )
            , _x( x )
            , _y( y )
        {
            // Do nothing.
        }

        explicit PackedSprite( PackedImage && image, const int32_t x = 0, const int32_t y = 0 )
            : PackedImage( std::move( image ) )
            , _x( x )
            , _y( y )
        {
            // Do nothing.
        }

        PackedSprite( const PackedSprite & sprite ) = default;
        PackedSprite( PackedSprite && sprite ) noexcept;

        ~PackedSprite() override = default;

        PackedSprite & operator=( const PackedSprite & sprite );
        PackedSprite & operator=( PackedSprite && sprite ) noexcept;

        PackedSprite & operator=( PackedImage && image ) noexcept;

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
}
