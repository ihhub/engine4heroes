/***************************************************************************
 *   engine4heroes: https://github.com/ihhub/engine4heroes                 *
 *   Copyright (C) 2026                                                    *
 *                                                                         *
 *   fheroes2: https://github.com/ihhub/fheroes2                           *
 *   Copyright (C) 2020 - 2026                                             *
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

#include "image.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>

namespace
{
    bool Validate( const engine4heroes::Image & image, const int32_t x, const int32_t y, const int32_t width, const int32_t height )
    {
        if ( image.empty() || width <= 0 || height <= 0 ) {
            // What's the reason to work with empty images?
            return false;
        }

        if ( x < 0 || y < 0 || x + width > image.width() || y + height > image.height() ) {
            return false;
        }

        return true;
    }

    bool Verify( const engine4heroes::Image & image, int32_t & x, int32_t & y, int32_t & width, int32_t & height )
    {
        if ( image.empty() || width <= 0 || height <= 0 ) {
            // What's the reason to work with empty images?
            return false;
        }

        const int32_t widthOut = image.width();
        const int32_t heightOut = image.height();

        if ( x < 0 ) {
            const int32_t offsetX = -x;
            if ( offsetX >= width ) {
                return false;
            }

            x = 0;
            width -= offsetX;
        }

        if ( y < 0 ) {
            const int32_t offsetY = -y;
            if ( offsetY >= height ) {
                return false;
            }

            y = 0;
            height -= offsetY;
        }

        if ( x > widthOut || y > heightOut ) {
            return false;
        }

        if ( x + width > widthOut ) {
            const int32_t offsetX = x + width - widthOut;
            if ( offsetX >= width ) {
                return false;
            }
            width -= offsetX;
        }

        if ( y + height > heightOut ) {
            const int32_t offsetY = y + height - heightOut;
            if ( offsetY >= height ) {
                return false;
            }
            height -= offsetY;
        }

        return true;
    }

    bool Verify( int32_t & inX, int32_t & inY, int32_t & outX, int32_t & outY, int32_t & width, int32_t & height, const int32_t widthIn, const int32_t heightIn,
                 const int32_t widthOut, const int32_t heightOut )
    {
        if ( widthIn <= 0 || heightIn <= 0 || widthOut <= 0 || heightOut <= 0 || width <= 0 || height <= 0 ) {
            // What's the reason to work with empty images?
            return false;
        }

        if ( inX < 0 || inY < 0 || inX > widthIn || inY > heightIn ) {
            return false;
        }

        if ( outX < 0 ) {
            const int32_t offsetX = -outX;
            if ( offsetX >= width ) {
                return false;
            }

            inX += offsetX;
            outX = 0;
            width -= offsetX;
        }

        if ( outY < 0 ) {
            const int32_t offsetY = -outY;
            if ( offsetY >= height ) {
                return false;
            }

            inY += offsetY;
            outY = 0;
            height -= offsetY;
        }

        if ( outX > widthOut || outY > heightOut ) {
            return false;
        }

        if ( inX + width > widthIn ) {
            const int32_t offsetX = inX + width - widthIn;
            if ( offsetX >= width ) {
                return false;
            }
            width -= offsetX;
        }

        if ( inY + height > heightIn ) {
            const int32_t offsetY = inY + height - heightIn;
            if ( offsetY >= height ) {
                return false;
            }
            height -= offsetY;
        }

        if ( outX + width > widthOut ) {
            const int32_t offsetX = outX + width - widthOut;
            if ( offsetX >= width ) {
                return false;
            }
            width -= offsetX;
        }

        if ( outY + height > heightOut ) {
            const int32_t offsetY = outY + height - heightOut;
            if ( offsetY >= height ) {
                return false;
            }
            height -= offsetY;
        }

        return true;
    }

    bool Verify( const engine4heroes::Image & in, int32_t & inX, int32_t & inY, const engine4heroes::Image & out, int32_t & outX, int32_t & outY, int32_t & width,
                 int32_t & height )
    {
        return Verify( inX, inY, outX, outY, width, height, in.width(), in.height(), out.width(), out.height() );
    }
}

namespace engine4heroes
{
    Image::Image( Image && image ) noexcept
        : _data( std::move( image._data ) )
    {
        std::swap( _width, image._width );
        std::swap( _height, image._height );
    }

    Image & Image::operator=( const Image & image )
    {
        if ( this == &image ) {
            return *this;
        }

        copy( image );

        return *this;
    }

    Image & Image::operator=( Image && image ) noexcept
    {
        if ( this == &image ) {
            return *this;
        }

        std::swap( _width, image._width );
        std::swap( _height, image._height );
        std::swap( _data, image._data );

        return *this;
    }

    uint32_t * Image::image()
    {
        return _data.get();
    }

    const uint32_t * Image::image() const
    {
        return _data.get();
    }

    void Image::clear()
    {
        _data.reset();

        _width = 0;
        _height = 0;
    }


    void Image::resize( const int32_t width_, const int32_t height_ )
    {
        if ( width_ == _width && height_ == _height ) {
            return;
        }

        if ( width_ <= 0 || height_ <= 0 ) {
            clear();

            return;
        }

        const size_t size = static_cast<size_t>( width_ ) * height_;

        _data.reset( new uint32_t[size] );

        _width = width_;
        _height = height_;
    }

    void Image::fill( const uint32_t value )
    {
        if ( !empty() ) {
            uint32_t * data = image();
            const size_t totalSize = static_cast<size_t>( _width ) * _height;
            const uint32_t * dataEnd = data + totalSize;
            for ( ; data != dataEnd; ++data ) {
                *data = value;
            }
        }
    }

    void Image::reset()
    {
        if ( !empty() ) {
            uint32_t * data = image();
            const size_t totalSize = static_cast<size_t>( _width ) * _height;
            const uint32_t * dataEnd = data + totalSize;
            for ( ; data != dataEnd; ++data ) {
                *data = 0;
            }
        }
    }

    void Image::copy( const Image & image )
    {
        if ( !image._data ) {
            clear();

            return;
        }

        const size_t size = static_cast<size_t>( image._width ) * image._height;

        if ( image._width != _width || image._height != _height ) {
            _data.reset( new uint32_t[size] );

            _width = image._width;
            _height = image._height;
        }

        memcpy( _data.get(), image._data.get(), size * sizeof( uint32_t ) );
    }

    Sprite::Sprite( Sprite && sprite ) noexcept
        : Image( std::move( sprite ) )
    {
        std::swap( _x, sprite._x );
        std::swap( _y, sprite._y );
    }

    Sprite & Sprite::operator=( const Sprite & sprite )
    {
        if ( this == &sprite ) {
            return *this;
        }

        Image::operator=( sprite );

        _x = sprite._x;
        _y = sprite._y;

        return *this;
    }

    Sprite & Sprite::operator=( Sprite && sprite ) noexcept
    {
        if ( this == &sprite ) {
            return *this;
        }

        Image::operator=( std::move( sprite ) );

        std::swap( _x, sprite._x );
        std::swap( _y, sprite._y );

        return *this;
    }

    Sprite & Sprite::operator=( Image && image ) noexcept
    {
        Image::operator=( std::move( image ) );

        _x = 0;
        _y = 0;

        return *this;
    }

    void Sprite::setPosition( const int32_t x_, const int32_t y_ )
    {
        _x = x_;
        _y = y_;
    }

    ImageRestorer::ImageRestorer( Image & image )
        : _image( image )
        , _width( image.width() )
        , _height( image.height() )
    {
        _updateRoi();

        _copy.resize( _width, _height );

        Copy( _image, 0, 0, _copy, 0, 0, _width, _height );
    }

    ImageRestorer::ImageRestorer( Image & image, const int32_t x_, const int32_t y_, const int32_t width, const int32_t height )
        : _image( image )
        , _x( x_ )
        , _y( y_ )
        , _width( width )
        , _height( height )
    {
        _updateRoi();

        _copy.resize( _width, _height );

        Copy( _image, _x, _y, _copy, 0, 0, _width, _height );
    }

    void ImageRestorer::update( const int32_t x_, const int32_t y_, const int32_t width, const int32_t height )
    {
        _isRestored = false;
        _x = x_;
        _y = y_;
        _width = width;
        _height = height;
        _updateRoi();

        _copy.resize( _width, _height );
        Copy( _image, _x, _y, _copy, 0, 0, _width, _height );
    }

    void ImageRestorer::restore()
    {
        _isRestored = true;
        Copy( _copy, 0, 0, _image, _x, _y, _width, _height );
    }

    void ImageRestorer::_updateRoi()
    {
        if ( _width < 0 ) {
            _width = 0;
        }

        if ( _height < 0 ) {
            _height = 0;
        }

        if ( _x < 0 ) {
            const int32_t offset = -_x;
            _x = 0;
            _width = _width < offset ? 0 : _width - offset;
        }

        if ( _y < 0 ) {
            const int32_t offset = -_y;
            _y = 0;
            _height = _height < offset ? 0 : _height - offset;
        }

        if ( _x >= _image.width() || _y >= _image.height() ) {
            _x = 0;
            _y = 0;
            _width = 0;
            _height = 0;
            return;
        }

        if ( _x + _width > _image.width() ) {
            const int32_t offsetX = _x + _width - _image.width();
            if ( offsetX >= _width ) {
                _x = 0;
                _y = 0;
                _width = 0;
                _height = 0;
                return;
            }
            _width -= offsetX;
        }

        if ( _y + _height > _image.height() ) {
            const int32_t offsetY = _y + _height - _image.height();
            if ( offsetY >= _height ) {
                _x = 0;
                _y = 0;
                _width = 0;
                _height = 0;
                return;
            }
            _height -= offsetY;
        }
    }

    void Blit( const Image & in, Image & out, const bool flip /* = false */ )
    {
        Blit( in, 0, 0, out, 0, 0, in.width(), in.height(), flip );
    }

    void Blit( const Image & in, Image & out, const Rect & outRoi, const bool flip /* = false */ )
    {
        Blit( in, 0, 0, out, outRoi.x, outRoi.y, outRoi.width, outRoi.height, flip );
    }

    void Blit( const Image & in, Image & out, const int32_t outX, const int32_t outY, const bool flip /* = false */ )
    {
        Blit( in, 0, 0, out, outX, outY, in.width(), in.height(), flip );
    }

    void Blit( const Image & in, const Point & inPos, Image & out, const Point & outPos, const Size & size, const bool flip /* = false */ )
    {
        Blit( in, inPos.x, inPos.y, out, outPos.x, outPos.y, size.width, size.height, flip );
    }

    void Blit( const Image & in, int32_t inX, int32_t inY, Image & out, int32_t outX, int32_t outY, int32_t width, int32_t height, const bool flip /* = false */ )
    {
        // TODO: optimize for images that don't have alpha channel.
        if ( !Verify( in, inX, inY, out, outX, outY, width, height ) ) {
            return;
        }

        const int32_t widthIn = in.width();
        const int32_t widthOut = out.width();

        if ( flip ) {
            // TODO: implement it!
            assert( 0 );
        }
        else {
            const int32_t offsetInY = inY * widthIn + inX;
            const uint32_t * imageInY = in.image() + offsetInY;

            const int32_t offsetOutY = outY * widthOut + outX;
            uint32_t * imageOutY = out.image() + offsetOutY;
            const uint32_t * imageInYEnd = imageInY + height * widthIn;

            for ( ; imageInY != imageInYEnd; imageInY += widthIn, imageOutY += widthOut ) {
                const uint32_t * imageInX = imageInY;
                uint32_t * imageOutX = imageOutY;
                const uint32_t * imageInXEnd = imageInX + width;

                for ( ; imageInX != imageInXEnd; ++imageInX, ++imageOutX ) {
                    const uint32_t alpha = ( *imageInX ) >> 24;
                    if ( alpha == 0 ) {
                        // Skip the pixel as it is empty.
                    }
                    else if ( alpha == 255 ) {
                        // Copy pixel.
                        *imageOutX = *imageInX;
                    }
                    else {
                        // We need to blend pixels.
                        const uint32_t inRed{ *imageInX & 0xFF };
                        const uint32_t inGreen{ ( ( *imageInX ) >> 8U ) & 0xFF };
                        const uint32_t inBlue{ ( ( *imageInX ) >> 16U ) & 0xFF };

                        const uint32_t outRed{ *imageOutX & 0xFF };
                        const uint32_t outGreen{ ( ( *imageOutX ) >> 8U ) & 0xFF };
                        const uint32_t outBlue{ ( ( *imageOutX ) >> 16U ) & 0xFF };

                        const uint8_t red = static_cast<uint8_t>( ( outRed * ( 255 - alpha ) + inRed * alpha ) / 255U );
                        const uint8_t green = static_cast<uint8_t>( ( outGreen * ( 255 - alpha ) + inGreen * alpha ) / 255U );
                        const uint8_t blue = static_cast<uint8_t>( ( outBlue * ( 255 - alpha ) + inBlue * alpha ) / 255U );

                        *imageOutX = red + ( green << 8 ) + ( blue << 16 ) + ( 255U << 24U );
                    }
                }
            }
        }
    }

    void Copy( const Image & in, Image & out )
    {
        out = in;
    }

    void Copy( const Image & in, const int32_t inX, const int32_t inY, Image & out, const Rect & outRoi )
    {
        Copy( in, inX, inY, out, outRoi.x, outRoi.y, outRoi.width, outRoi.height );
    }

    void Copy( const Image & in, int32_t inX, int32_t inY, Image & out, int32_t outX, int32_t outY, int32_t width, int32_t height )
    {
        if ( !Verify( in, inX, inY, out, outX, outY, width, height ) ) {
            return;
        }

        const int32_t widthIn = in.width();
        const int32_t widthOut = out.width();

        if ( inX == 0 && inY == 0 && outX == 0 && outY == 0 && width == widthIn && width == widthOut && height == in.height() && height == out.height() ) {
            // Both images have identical width and height and a full copy is requested.
            Copy( in, out );
            return;
        }

        const int32_t offsetInY = inY * widthIn + inX;
        const uint32_t * imageInY = in.image() + offsetInY;

        const int32_t offsetOutY = outY * widthOut + outX;
        uint32_t * imageOutY = out.image() + offsetOutY;
        const uint32_t * imageOutYEnd = imageOutY + height * widthOut;

        for ( ; imageOutY != imageOutYEnd; imageInY += widthIn, imageOutY += widthOut ) {
            memcpy( imageOutY, imageInY, static_cast<size_t>( width ) * sizeof( uint32_t ) );
        }
    }

    Sprite Crop( const Image & image, int32_t x, int32_t y, int32_t width, int32_t height )
    {
        if ( image.empty() || width <= 0 || height <= 0 ) {
            return {};
        }

        if ( x < 0 ) {
            const int32_t offsetX = -x;
            if ( offsetX >= width ) {
                return {};
            }

            x = 0;
            width -= offsetX;
        }

        if ( y < 0 ) {
            const int32_t offsetY = -y;
            if ( offsetY >= height ) {
                return {};
            }

            y = 0;
            height -= offsetY;
        }

        if ( x > image.width() || y > image.height() ) {
            return {};
        }

        if ( x + width > image.width() ) {
            const int32_t offsetX = x + width - image.width();
            width -= offsetX;
        }

        if ( y + height > image.height() ) {
            const int32_t offsetY = y + height - image.height();
            height -= offsetY;
        }

        Sprite out;
        out.resize( width, height );

        Copy( image, x, y, out, 0, 0, width, height );
        out.setPosition( x, y );
        return out;
    }

    void Fill( Image & image, int32_t x, int32_t y, int32_t width, int32_t height, const uint32_t color )
    {
        if ( !Verify( image, x, y, width, height ) ) {
            return;
        }

        if ( image.width() == width && image.height() == height ) {
            // We fill the whole image.
            image.fill( color );
            return;
        }

        const int32_t imageWidth = image.width();

        uint32_t * imageY = image.image() + y * imageWidth + x;
        const uint32_t * imageYEnd = imageY + height * imageWidth;

        for ( ; imageY != imageYEnd; imageY += imageWidth ) {
            uint32_t * imageX = imageY;
            const uint32_t * imageXEnd = imageX + width;
            for ( ; imageX != imageXEnd; ++imageX ) {
                *imageX = color;
            }
        }
    }
}
