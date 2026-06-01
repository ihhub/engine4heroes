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

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "serialize.h"
#include "zzlib.h"

int main( int argc, char ** argv )
{
    if ( argc < 2 ) {
        return EXIT_FAILURE;
    }

    StreamFile fileStream;
    fileStream.setBigendian( true );

    std::string filename = argv[1];

    if ( !fileStream.open( filename, "rb" ) ) {
        std::cout << "Failed to open " << filename << std::endl;
        return EXIT_FAILURE;
    }

    RWStreamBuf dataStream;
    dataStream.setBigendian( true );

    const auto data = fileStream.getRaw( 0 );
    const auto unpacked = Compression::unzipGzip( data );

    filename = std::string( argv[2] ) + "\\" + filename + ".unpacked";

    std::ofstream outputStream( filename, std::ios_base::binary | std::ios_base::trunc );
    outputStream.write( reinterpret_cast<const char *>( unpacked.data() ), unpacked.size() );

    return EXIT_SUCCESS;
}
