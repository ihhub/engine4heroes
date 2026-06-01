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

#include "h4r_file.h"

#include "tools.h"

#include <array>
#include <cassert>
#include <cstddef>
#include <utility>

namespace
{
    const std::array<uint8_t, 4> magicWord{ 'H', '4', 'R', 0x05 };

    // The minimum file size if no entries are present.
    const size_t minFileSize{ 4 + 4 + 4 };
}

namespace File
{
    bool H4RFile::open( const std::string & fileName )
    {
        if ( !_stream.open( fileName, "rb" ) ) {
            return false;
        }

        const size_t fileSize = _stream.size();
        if ( fileSize < minFileSize ) {
            return false;
        }

        for ( const uint8_t value : magicWord ) {
            if ( _stream.get() != value ) {
                return false;
            }
        }

        const uint32_t fileTableOffset = _stream.getLE32();
        if ( fileTableOffset < 8 || fileTableOffset >= fileSize ) {
            return false;
        }

        _stream.seek( fileTableOffset );

        const uint32_t fileCount = _stream.getLE32();
        if ( fileCount == 0 ) {
            return false;
        }

        for ( uint32_t i = 0; i < fileCount; ++i ) {
            H4DFileEntry entry;

            entry.offset = _stream.getLE32();
            entry.size = _stream.getLE32();
            entry.compressedSize = _stream.getLE32();
            entry.timestamp = _stream.getLE32();

            const uint16_t nameLength = _stream.getLE16();
            if ( nameLength == 0 ) {
                return false;
            }

            // All names must be stored in a lower-case format.
            std::string name = StringLower( _stream.getString( nameLength ) );

            // TODO: we might not need to store additional ".h4d" part of the name since all files have this suffix.

            const uint16_t commentLength = _stream.getLE16();
            if ( commentLength > 0 ) {
#if !defined( NDEBUG )
                // We don't need this information while running a release version of the engine.
                // This is just waste of resources.
                entry.comment = _stream.getString( commentLength );
#else
                _stream.skip( commentLength );
#endif
            }

            const uint16_t referenceLength = _stream.getLE16();
            if ( referenceLength > 0 ) {
                entry.reference = StringLower( _stream.getString( referenceLength ) );

                // TODO: we might not need to store additional ".h4d" part of the reference since all files have this suffix.
            }

            entry.type = _stream.getLE32();

            if ( entry.type != 3 ) {
                entry.type = entry.type;
            }

            if ( entry.offset > 0 ) {
                assert( entry.size > 0 && entry.compressedSize > 0 );
                assert( entry.offset < fileSize );
                assert( entry.offset + entry.size <= fileSize );
                assert( entry.reference.empty() );
            }
            else {
                assert( !entry.reference.empty() );
            }

            if ( _stream.fail() ) {
                return false;
            }

            _files.emplace( std::move( name ), std::move( entry ) );
        }

        /*
#if !defined( NDEBUG )
        // Once all entries have been populated it is important to verify the correctness of the data.
        //
        // Check that all references refer to valid existing items and these items are not references.
        for ( const auto & [name, fileInfo] : _files ) {
            if ( !fileInfo.reference.empty() ) {
                const auto iter = _files.find( fileInfo.reference );
                assert( iter != _files.end() );
                // One reference can refer to another one.
            }
        }
#endif
        */

        return !_stream.fail();
    }

    std::vector<uint8_t> H4RFile::getFileEntryData( const std::string & fileName )
    {
        auto it = _files.find( fileName );
        if ( it == _files.end() ) {
            return {};
        }

        const auto & entry = it->second;

        if ( !entry.reference.empty() ) {
            // You didn't check that this entry is a reference!
            assert( 0 );
            return {};
        }

        if ( entry.size == 0 ) {
            return {};
        }

        const uint32_t fileOffset = entry.offset;
        _stream.seek( fileOffset );
        return _stream.getRaw( entry.size );
    }
}
