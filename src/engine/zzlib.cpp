/***************************************************************************
 *   engine4heroes: https://github.com/ihhub/engine4heroes                 *
 *   Copyright (C) 2026                                                    *
 *                                                                         *
 *   fheroes2: https://github.com/ihhub/fheroes2                           *
 *   Copyright (C) 2019 - 2024                                             *
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

#include "zzlib.h"

#include <cstring>
#include <ostream>

#include <zconf.h>
#include <zlib.h>

#include "serialize.h"

namespace
{
    constexpr uint16_t FORMAT_VERSION_0 = 0;
}

std::vector<uint8_t> Compression::unzipData( const uint8_t * src, const size_t srcSize, size_t realSize /* = 0 */ )
{
    if ( src == nullptr || srcSize == 0 ) {
        return {};
    }

    const uLong srcSizeULong = static_cast<uLong>( srcSize );
    if ( srcSizeULong != srcSize ) {
        return {};
    }

    std::vector<uint8_t> res( realSize );

    if ( realSize == 0 ) {
        constexpr size_t sizeMultiplier = 7;

        if ( srcSize > res.max_size() / sizeMultiplier ) {
            // If the multiplicated size is too large, let's start with the original size and see how it goes
            realSize = srcSize;
        }
        else {
            realSize = srcSize * sizeMultiplier;
        }

        res.resize( realSize );
    }

    uLong dstSizeULong = static_cast<uLong>( res.size() );
    if ( dstSizeULong != res.size() ) {
        return {};
    }

    int ret = Z_BUF_ERROR;
    while ( Z_BUF_ERROR == ( ret = uncompress( res.data(), &dstSizeULong, src, srcSizeULong ) ) ) {
        constexpr size_t sizeMultiplier = 2;

        // Avoid infinite loop due to unsigned overflow on multiplication
        if ( res.size() > res.max_size() / sizeMultiplier ) {
            return {};
        }

        res.resize( res.size() * sizeMultiplier );

        dstSizeULong = static_cast<uLong>( res.size() );
        if ( dstSizeULong != res.size() ) {
            return {};
        }
    }

    if ( ret != Z_OK ) {
        return {};
    }

    res.resize( dstSizeULong );

    return res;
}

std::vector<uint8_t> Compression::zipData( const uint8_t * src, const size_t srcSize )
{
    if ( src == nullptr || srcSize == 0 ) {
        return {};
    }

    const uLong srcSizeULong = static_cast<uLong>( srcSize );
    if ( srcSizeULong != srcSize ) {
        return {};
    }

    std::vector<uint8_t> res( compressBound( srcSizeULong ) );

    uLong dstSizeULong = static_cast<uLong>( res.size() );
    if ( dstSizeULong != res.size() ) {
        return {};
    }

    const int ret = compress( res.data(), &dstSizeULong, src, srcSizeULong );

    if ( ret != Z_OK ) {
        return {};
    }

    res.resize( dstSizeULong );

    return res;
}

std::vector<uint8_t> Compression::unzipGzip( const std::vector<uint8_t> & gzipStream )
{
    if ( gzipStream.empty() ) {
        return {};
    }

    z_stream zStream{};
    zStream.next_in = const_cast<Bytef*>( gzipStream.data() );
    zStream.avail_in = static_cast<uInt>( gzipStream.size() );

    // 15 + 16 means "windowBits = 15" with gzip decoding enabled (16)
    if ( inflateInit2( &zStream, 15 + 16 ) != Z_OK ) {
        return {};
    }

    int ret{ Z_OK };

    constexpr size_t chunkSize{ 16 * 1024 };

    // To avoid populating a container while resizing (std::vector::resize() does it),
    // we dynamically allocate memory without any extra operations.
    // This approach saves some computational resources which we don't want to waste.
    std::unique_ptr<uint8_t[]> outputBuffer;
    outputBuffer.reset( new uint8_t[chunkSize] );

    std::vector<uint8_t> uncompressed;

    do {
        zStream.next_out = outputBuffer.get();
        zStream.avail_out = static_cast<uInt>( chunkSize );

        ret = inflate( &zStream, Z_NO_FLUSH );
        if ( ret != Z_OK && ret != Z_STREAM_END ) {
            inflateEnd( &zStream );
            return {};
        }

        const size_t decompressedLength = chunkSize - zStream.avail_out;
        uncompressed.insert( uncompressed.end(), outputBuffer.get(), outputBuffer.get() + decompressedLength );
    } while ( ret != Z_STREAM_END );

    if ( inflateEnd( &zStream ) != Z_OK ) {
        return {};
    }

    return uncompressed;
}
