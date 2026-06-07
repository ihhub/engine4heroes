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

#include "h4d_image.h"

#include <cstring>

namespace engine4heroes
{
    PackedImage::PackedImage( PackedImage && image ) noexcept
        : _data( std::move( image._data ) )
    {
        std::swap( _width, image._width );
        std::swap( _height, image._height );
    }

    PackedImage & PackedImage::operator=( const PackedImage & image )
    {
        if ( this == &image ) {
            return *this;
        }

        copy( image );

        return *this;
    }

    PackedImage & PackedImage::operator=( PackedImage && image ) noexcept
    {
        if ( this == &image ) {
            return *this;
        }

        std::swap( _width, image._width );
        std::swap( _height, image._height );
        std::swap( _data, image._data );

        return *this;
    }

    uint8_t * PackedImage::image()
    {
        return _data.get();
    }

    const uint8_t * PackedImage::image() const
    {
        return _data.get();
    }

    void PackedImage::clear()
    {
        _data.reset();

        _width = 0;
        _height = 0;
    }

    void PackedImage::fill( const uint8_t value )
    {
        if ( !empty() ) {
            const size_t totalSize = static_cast<size_t>( _width ) * _height;
            memset( image(), value, totalSize );
        }
    }

    void PackedImage::resize( const int32_t width_, const int32_t height_ )
    {
        if ( width_ == _width && height_ == _height ) {
            return;
        }

        if ( width_ <= 0 || height_ <= 0 ) {
            clear();

            return;
        }

        const size_t size = static_cast<size_t>( width_ ) * height_;

        _data.reset( new uint8_t[size] );

        _width = width_;
        _height = height_;
    }

    void PackedImage::reset()
    {
        if ( !empty() ) {
            const size_t totalSize = static_cast<size_t>( _width ) * _height;
            memset( image(), static_cast<uint8_t>( 0 ), totalSize );
        }
    }

    void PackedImage::copy( const PackedImage & image )
    {
        if ( !image._data ) {
            clear();

            return;
        }

        const size_t size = static_cast<size_t>( image._width ) * image._height;

        if ( image._width != _width || image._height != _height ) {
            _data.reset( new uint8_t[size] );

            _width = image._width;
            _height = image._height;
        }

        memcpy( _data.get(), image._data.get(), size );
    }

    PackedSprite::PackedSprite( PackedSprite && sprite ) noexcept
        : PackedImage( std::move( sprite ) )
    {
        std::swap( _x, sprite._x );
        std::swap( _y, sprite._y );
    }

    PackedSprite & PackedSprite::operator=( const PackedSprite & sprite )
    {
        if ( this == &sprite ) {
            return *this;
        }

        PackedImage::operator=( sprite );

        _x = sprite._x;
        _y = sprite._y;

        return *this;
    }

    PackedSprite & PackedSprite::operator=( PackedSprite && sprite ) noexcept
    {
        if ( this == &sprite ) {
            return *this;
        }

        PackedImage::operator=( std::move( sprite ) );

        std::swap( _x, sprite._x );
        std::swap( _y, sprite._y );

        return *this;
    }

    PackedSprite & PackedSprite::operator=( PackedImage && image ) noexcept
    {
        PackedImage::operator=( std::move( image ) );

        _x = 0;
        _y = 0;

        return *this;
    }

    void PackedSprite::setPosition( const int32_t x_, const int32_t y_ )
    {
        _x = x_;
        _y = y_;
    }
}
