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

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "serialize.h"

namespace File
{
    // Do not use this class directly.
    // Use H4RFileManager to access game resources.
    class H4RFile final
    {
    public:
        // !!! IMPORTANT !!!
        // This structure should not be used in the final engine
        // as it contains a lot of data not being used by the game.
        struct H4DFileEntry final
        {
            // Offset from the beginning of the file.
            uint32_t offset{ 0 };

            // Uncompressed size of the file.
            uint32_t size{ 0 };

            // Compressed file size stored inside h4r file.
            uint32_t compressedSize{ 0 };

            // The time when the file was created.
            // It is used to determine which resource to be used from different h4r files.
            uint32_t timestamp{ 0 };

            // The type of a file. Unknown use case.
            uint32_t type{ 0 };

#if !defined( NDEBUG )
            // Comment or path of the file.
            // Possibly used during the development of the game.
            //
            // As of now, this is only for debug purposes as we don't want to waste resources for nothing.
            std::string comment;
#endif

            // Reference to the actual file.
            // If reference is set the current file is empty and it points to another file.
            std::string reference;
        };

        bool isGood() const
        {
            return !_stream.fail() && !_files.empty();
        }

        bool open( const std::string & fileName );

        // Do not call this function without checking whether an entry is not a reference!
        std::vector<uint8_t> getFileEntryData( const std::string & fileName );

        const std::map<std::string, H4DFileEntry, std::less<>> & getFileEntries() const
        {
            return _files;
        }

    private:
        StreamFile _stream;
        std::map<std::string, H4DFileEntry, std::less<>> _files;
    };
}
