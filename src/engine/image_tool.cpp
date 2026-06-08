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

#include "image_tool.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <memory>
#include <ostream>
#include <vector>

#include <SDL_surface.h>

#include "image.h"
#include "serialize.h"

namespace
{
    struct Color final
    {
        uint8_t red{ 0 };
        uint8_t green{ 0 };
        uint8_t blue{ 0 };
    };
}

namespace engine4heroes
{
    bool decodeH4DSprite( ROStreamBuf & stream, Sprite & sprite, std::string & name )
    {
        if ( stream.fail() ) {
            return false;
        }

        // Extract palette information.
        const uint32_t paletteSize = stream.getLE16();

        // As of current knowledge the size of palette is expected to be no more than 256.
        assert( paletteSize <= 256 );

        if ( paletteSize == 0 ) {
            // How is it possible to make an image with no palette?
            return false;
        }

        if ( paletteSize > 256 ) {
            // It is assumed that an image should have not more than 256 colors.
            assert( 0 );
            return false;
        }

        // Some colors at the beginning of the palette are transparent.
        // Find the first non-transparent color.
        const uint32_t nonTransparentFirstColorIndex = stream.getLE16();
        if ( nonTransparentFirstColorIndex >= paletteSize ) {
            // What's the point of having this image if all colors are transparent?
            return false;
        }

        // engine4heroes::PackedSprite tempSprite;
        std::array<Color, 256> palette;
        for ( uint32_t i = 0; i < std::min( paletteSize, 256U ); ++i ) {
            palette[i].red = stream.get();
            palette[i].green = stream.get();
            palette[i].blue = stream.get();
        }

        // tempSprite.setPalette( std::move( palette ) );

        const uint32_t imageNameLength = stream.getLE16();
        if ( imageNameLength > 0 ) {
            const auto temp = stream.getRaw( imageNameLength );
            name.assign( temp.cbegin(), temp.cend() );
        }
        else {
            // Is it okay to have an image with no name?
            name = {};
        }

        // Known values of image types:
        // 0 - all pixels are opaque
        // 1 - some pixels should be skipped
        // 4 - some pixels should be skipped and the image has a packed alpha channel
        //
        // There could be other image types so this code should be modified if needed to handle them.
        const uint8_t imageType = stream.get();

        // Image area, used for rendering on other images.
        const int32_t startX = stream.getLE32();
        const int32_t startY = stream.getLE32();
        const int32_t endX = stream.getLE32();
        const int32_t endY = stream.getLE32();

        if ( startX == 0 && startY == 0 && endX == 0 && endY == 0 ) {
            // This is a proper empty image.
            return true;
        }

        sprite.setPosition( startX, startY );

        const int32_t width = endX - startX;
        const int32_t height = endY - startY;
        if ( height < 1 ) {
            // This is an empty image.
            return false;
        }

        // For images with transparency every line of image has non-zero offset.
        // This is a good approach to keep compressed images.
        std::vector<ImageLineInfo> lineInfo;
        lineInfo.resize( height );
        for ( auto & info : lineInfo ) {
            info.startX = stream.getLE16();
            info.endX = stream.getLE16();
            info.offset = stream.getLE32();
        }

        assert( lineInfo[0].offset == 0 );

        for ( size_t i = 1; i < lineInfo.size(); ++i ) {
            assert( lineInfo[i].offset == ( lineInfo[i - 1].endX - lineInfo[i - 1].startX + lineInfo[i - 1].offset ) );
        }

        sprite.resize( width, height );
        uint32_t * imageY = sprite.image();

        for ( const auto & info : lineInfo ) {
            uint32_t * imageX = imageY;

            if ( info.endX == info.startX ) {
                // Skip the whole line.
                const uint32_t * imageXEnd = imageX + width;
                for ( ; imageX != imageXEnd; ++imageX ) {
                    *imageX = 0;
                }

                imageY += width;
                continue;
            }

            const auto temp = stream.getRaw( info.endX - info.startX );
            assert( info.startX >= 0 );
            assert( info.endX <= width );

            if ( info.startX > 0 ) {
                const uint32_t * imageXEnd = imageX + info.startX;
                for ( ; imageX != imageXEnd; ++imageX ) {
                    *imageX = 0;
                }
            }

            for ( size_t x = 0; x < temp.size(); ++x, ++imageX ) {
                if ( temp[x] < nonTransparentFirstColorIndex ) {
                    *imageX = 0;
                }
                else {
                    const Color & color = palette[temp[x]];
                    *imageX = color.red + ( color.green << 8U ) + ( color.blue << 16U ) + ( 255 << 24U );
                }
            }

            const uint32_t * imageXEnd = imageY + width;
            for ( ; imageX != imageXEnd; ++imageX ) {
                *imageX = 0;
            }

            imageY += width;
        }

        // TODO: handle all image types.
        if ( imageType == 1 ) {
            // TODO: should we read extra info for the image?
        }
        else if ( imageType == 4 ) {
            const int32_t overallPixels = lineInfo.back().offset + lineInfo.back().endX - lineInfo.back().startX;

            // Somehow the alpha channel is stored in a compressed format.
            // This information is only for pixels present in the image.
            const int32_t alphaLength = ( overallPixels + 1 ) / 2;
            std::vector<uint8_t> alphaMask = stream.getRaw( alphaLength );
            std::vector<uint32_t> expandedAlphaMask;
            expandedAlphaMask.resize( overallPixels + 1 );

            for ( size_t i = 0; i < alphaMask.size(); ++i ) {
                const uint32_t lower = ( alphaMask[i] & 0x0F ) * 17;
                const uint32_t upper = ( ( alphaMask[i] & 0xF0 ) >> 4 ) * 17;

                expandedAlphaMask[2 * i] = lower;
                expandedAlphaMask[2 * i + 1] = upper;
            }

            imageY = sprite.image();

            const uint32_t * mask = expandedAlphaMask.data();

            for ( const auto & info : lineInfo ) {
                uint32_t * imageX = imageY;

                if ( info.endX == info.startX ) {
                    // Skip the whole line.
                    imageY += width;
                    continue;
                }

                assert( info.startX >= 0 );
                assert( info.endX <= width );

                imageX += info.startX;

                for ( int32_t x = 0; x < info.endX - info.startX; ++x, ++imageX, ++mask ) {
                    *imageX = ( *imageX & 0xFFFFFF ) + ( *mask << 24U );
                }

                imageY += width;
            }

            if ( nonTransparentFirstColorIndex > 0 ) {
                const int32_t maskLength = ( overallPixels + 131 ) / 128;
                stream.skip( maskLength );
            }
        }
        else {
            // A new image type?
            assert( imageType == 0 );
        }

        return !stream.fail();
    }

    bool save( const Image & image, const std::string & path )
    {
        if ( image.empty() ) {
            return false;
        }

        const std::unique_ptr<SDL_Surface, void ( * )( SDL_Surface * )> surface( SDL_CreateRGBSurface( 0, image.width(), image.height(), 32, 0xFF, 0xFF00, 0xFF0000,
                                                                                                       0xFF000000 ),
                                                                                 SDL_FreeSurface );
        if ( !surface ) {
            return false;
        }

        if ( surface->pitch != image.width() * sizeof( uint32_t ) ) {
            const uint32_t * imageIn = image.image();

            for ( int32_t i = 0; i < image.height(); ++i ) {
                memcpy( static_cast<uint32_t *>( surface->pixels ) + surface->pitch * i, imageIn + image.width() * i,
                        static_cast<size_t>( image.width() ) * sizeof( uint32_t ) );
            }
        }
        else {
            memcpy( surface->pixels, image.image(), static_cast<size_t>( image.width() * image.height() ) * sizeof( uint32_t ) );
        }

        const int res = SDL_SaveBMP( surface.get(), path.c_str() );
        return ( res == 0 );
    }
}
