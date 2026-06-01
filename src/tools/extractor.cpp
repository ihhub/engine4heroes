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

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "h4r_file.h"

int main( int argc, char ** argv )
{
    if ( argc < 3 ) {
        std::cerr << argv[0] << " extracts the contents of the specified H4R file." << std::endl
                  << "Syntax: " << argv[0] << " input_file.h4r dst_dir" << std::endl;
        return EXIT_FAILURE;
    }

    const std::string inputFileName{ argv[1] };
    const std::string dstDir{ argv[2] };

    File::H4RFile fileReader;
    fileReader.open( inputFileName );
    if ( !fileReader.isGood() ) {
        std::cerr << "File " << inputFileName << " is invalid." << std::endl;
        return EXIT_FAILURE;
    }

    const std::filesystem::path prefixPath = std::filesystem::path( dstDir ) / std::filesystem::path( inputFileName ).stem();

    std::error_code ec;

    // Using the non-throwing overloads
    if ( !std::filesystem::exists( prefixPath, ec ) && !std::filesystem::create_directories( prefixPath, ec ) ) {
        std::cerr << "Cannot create directory " << prefixPath << std::endl;
        return EXIT_FAILURE;
    }

    uint32_t itemsExtracted = 0;
    uint32_t itemsFailed = 0;

    for ( const auto & [fileName, fileInfo] : fileReader.getFileEntries() ) {
        if ( fileInfo.reference.empty() ) {
            // This is a real file. Save it.
            const auto & buf = fileReader.getFileEntryData( fileName );
            if ( buf.size() != fileInfo.size ) {
                ++itemsFailed;

                std::cerr << inputFileName << ": item " << fileName << " has an invalid size of " << fileInfo.size << std::endl;
                continue;
            }

            const std::filesystem::path outputFilePath = prefixPath / std::filesystem::path( fileName );

            std::ofstream outputStream( outputFilePath, std::ios_base::binary | std::ios_base::trunc );
            if ( !outputStream ) {
                std::cerr << "Cannot open file " << outputFilePath << std::endl;
                return EXIT_FAILURE;
            }

            {

                outputStream.write( reinterpret_cast<const char *>( buf.data() ), buf.size() );
            }

            if ( !outputStream ) {
                std::cerr << "Error writing to file " << outputFilePath << std::endl;
                return EXIT_FAILURE;
            }

            ++itemsExtracted;
        }
        else {
            const std::filesystem::path outputFilePath = prefixPath / std::filesystem::path( fileName + ".ref" );
            std::ofstream outputStream( outputFilePath, std::ios_base::binary | std::ios_base::trunc );
            if ( !outputStream ) {
                std::cerr << "Cannot open file " << outputFilePath << std::endl;
                return EXIT_FAILURE;
            }

            {
                outputStream << fileInfo.reference;
            }

            if ( !outputStream ) {
                std::cerr << "Error writing to file " << outputFilePath << std::endl;
                return EXIT_FAILURE;
            }

            ++itemsExtracted;
        }
    }

    std::cout << "Total extracted items: " << itemsExtracted << ", failed items: " << itemsFailed << std::endl;

    return ( itemsFailed == 0 ) ? EXIT_SUCCESS : EXIT_FAILURE;
}
