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

#include "resource_manager.h"

#include <map>
#include <vector>

#include "configuration.h"
#include "dir.h"
#include "h4d_file.h"
#include "h4r_file.h"
#include "tools.h"
#include "zzlib.h"

namespace
{
    std::map<std::string, std::vector<engine4heroes::Sprite>> imageCache;
    std::map<std::string, std::vector<uint8_t>> audioCache;

    const engine4heroes::Sprite emptyImage;
    const std::vector<uint8_t> emptyAudioTrack;

    File::H4RFile heroes4;
    File::H4RFile musicData;

    std::string getValidH4RFileName( const std::string & filename, const ListFiles & candidates )
    {
        for ( const std::string & path : candidates ) {
            if ( path.size() < filename.size() ) {
                // Obviously this is not a correct file.
                continue;
            }

            const std::string tempPath = StringLower( path );

            if ( tempPath.compare( tempPath.size() - filename.size(), filename.size(), filename ) == 0 ) {
                return path;
            }
        }

        return {};
    }
}

namespace GameResource
{
    bool initializeResources()
    {
        const ListFiles fileCandidates = Configuration::findFiles( "data", ".h4r", false );
        if ( fileCandidates.empty() ) {
            return false;
        }

        const std::string heroes4Filename = getValidH4RFileName( "heroes4.h4r", fileCandidates );
        if ( !heroes4.open( heroes4Filename ) ) {
            return false;
        }

        const std::string musicFilename = getValidH4RFileName( "music.h4r", fileCandidates );
        return musicData.open( musicFilename );
    }

    const engine4heroes::Sprite & getImage( const std::string & imagePath, const uint32_t imageIndex )
    {
        const auto imageIter = imageCache.find( imagePath );
        if ( imageIter != imageCache.end() ) {
            if ( imageIndex >= imageIter->second.size() ) {
                return emptyImage;
            }

            return imageIter->second[imageIndex];
        }

        const auto iter = heroes4.getFileEntries().find( imagePath );
        if ( iter == heroes4.getFileEntries().end() ) {
            return emptyImage;
        }

        const auto data = heroes4.getFileEntryData( imagePath );
        if ( data.empty() ) {
            return emptyImage;
        }

        const auto unpacked = Compression::unzipGzip( data );
        if ( unpacked.empty() ) {
            assert( 0 );
            return emptyImage;
        }

        const auto [newDataIter, isSuccess] = imageCache.emplace( imagePath, File::getImages( imagePath, unpacked ) );
        assert( isSuccess );

        if ( imageIndex >= newDataIter->second.size() ) {
            return emptyImage;
        }

        return newDataIter->second[imageIndex];
    }

    const std::vector<uint8_t> & getAudioStream( const std::string & audioPath )
    {
        const auto audioIter = audioCache.find( audioPath );
        if ( audioIter != audioCache.end() ) {
            return audioIter->second;
        }

        std::vector<uint8_t> data;

        auto iter = heroes4.getFileEntries().find( audioPath );
        if ( iter != heroes4.getFileEntries().end() ) {
            data = heroes4.getFileEntryData( audioPath );
        }
        else {
            iter = musicData.getFileEntries().find( audioPath );
            if ( iter != musicData.getFileEntries().end() ) {
                data = musicData.getFileEntryData( audioPath );
            }
        }

        if ( data.empty() ) {
            return emptyAudioTrack;
        }

        const auto unpacked = Compression::unzipGzip( data );
        if ( unpacked.empty() ) {
            assert( 0 );
            return emptyAudioTrack;
        }

        const auto [newDataIter, isSuccess] = audioCache.emplace( audioPath, File::getAudioStream( audioPath, unpacked ) );
        assert( isSuccess );

        return newDataIter->second;
    }
}
