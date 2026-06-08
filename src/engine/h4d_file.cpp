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

#include "h4d_file.h"

#include <cassert>
#include <cstring>
#include <initializer_list>
#include <string_view>
#include <utility>

#include "image_tool.h"
#include "serialize.h"

namespace
{
    constexpr std::string_view actorSequencePrefix{ "actor_sequence." };
    constexpr std::string_view layersPrefix{ "layers." };
    constexpr std::string_view bitmapRawPrefix{ "bitmap_raw." };
    constexpr std::string_view animationPrefix{ "animation." };

    constexpr std::string_view soundPrefix{ "sound." };
    constexpr std::string_view fontPrefix{ "font." };

    bool isPrefixOf( const std::string & path, const std::string_view prefix )
    {
        return path.size() >= prefix.size() && memcmp( path.data(), prefix.data(), prefix.size() ) == 0;
    }

    // const std::array<uint8_t, 2> actorSequenceMagicSequence{ 0x02, 0x00 };

    bool processImage( const std::string & entryName, ROStreamBuf & stream, engine4heroes::Sprite & output )
    {
        (void)entryName;

        std::string imageName;
        return engine4heroes::decodeH4DSprite( stream, output, imageName );
    }

    bool processLayersObject( const std::string & entryName, ROStreamBuf & stream, std::vector<engine4heroes::Sprite> & images )
    {
        // Get the number of images per object.
        const uint32_t imageCount = stream.getLE16();
        if ( imageCount == 0 ) {
            // Why do we even try to load an empty file?
            return false;
        }

        for ( uint32_t i = 0; i < imageCount; ++i ) {
            engine4heroes::Sprite image;
            if ( !processImage( entryName, stream, image ) ) {
                return false;
            }

            images.emplace_back( std::move( image ) );
        }

        return true;
    }

    bool processActorSequence( const std::string & entryName, ROStreamBuf & stream, std::vector<engine4heroes::Sprite> & images )
    {
        const uint16_t fileType = stream.getLE16();
        (void)fileType;
        const uint16_t fileFormat = stream.getLE16();

        std::vector<uint8_t> info;

        if ( fileFormat > 2 ) {
            const uint32_t infoLength = stream.getLE16() * 4;
            if ( infoLength > 0 ) {
                info = stream.getRaw( infoLength );
            }
        }

        if ( !processLayersObject( entryName, stream, images ) ) {
            return false;
        }

        assert( stream.size() >= 10 );

        std::vector<uint8_t> extraInfo;

        if ( stream.size() > 0 ) {
            extraInfo = stream.getRaw( stream.size() );
        }

        return true;
    }

    bool processRawBitmap( ROStreamBuf & stream, std::vector<engine4heroes::Sprite> & images )
    {
        // Raw bitmap literally contain raw bitmap 24-bit images.
        const uint16_t fileType = stream.getLE16();
        if ( fileType != 1 ) {
            // No idea how to handle this yet.
            assert( 0 );
            return false;
        }

        const uint16_t fileFormat = stream.getLE16();
        if ( fileFormat != 0 ) {
            // No idea how to handle this yet.
            assert( 0 );
            return false;
        }

        const uint32_t height = stream.getLE32();
        const uint32_t width = stream.getLE32();
        if ( width == 0 || height == 0 ) {
            // A malformed image?
            assert( 0 );
            return false;
        }

        const uint32_t imageSize = stream.getLE32();
        if ( imageSize != stream.size() ) {
            // A corrupted file?
            assert( 0 );
            return false;
        }

        engine4heroes::Sprite image;
        image.resize( static_cast<int32_t>( width ), static_cast<int32_t>( height ) );

        uint32_t * data = image.image();
        const uint32_t * dataEnd = data + width * height;

        for ( ; data != dataEnd; ++data ) {
            const uint8_t red{ stream.get() };
            const uint8_t green{ stream.get() };
            const uint8_t blue{ stream.get() };

            *data = red + ( green << 8U ) + ( blue << 16U );
        }

        images.emplace_back( std::move( image ) );

        return !stream.fail();
    }

    bool processAnimation( const std::string & entryName, ROStreamBuf & stream, std::vector<engine4heroes::Sprite> & images )
    {
        return processLayersObject( entryName, stream, images );

        // TODO: get extra information about the animation object.
    }

    bool processSound( ROStreamBuf & stream, std::vector<uint8_t> & sound )
    {
        const uint16_t soundType = stream.getLE16();
        if ( soundType == 1 ) {
            // It is an MP3 audio track.

            // TODO: identify what exactly all these values for MP3.
            const uint16_t chunkSize = stream.getLE16();
            (void)chunkSize;
            const uint8_t channelCount = stream.get();
            (void)channelCount;
            const uint32_t sampleRateInHz = stream.getLE32();
            (void)sampleRateInHz;
            const uint32_t dataLengthBytes = stream.getLE32();
            (void)dataLengthBytes;
            const uint16_t blockAlign = stream.getLE16();
            (void)blockAlign;
            const uint32_t totalFileSize = stream.getLE32();
            (void)totalFileSize;

            sound = stream.getRaw( 0 );
        }
        else {
            // It is a WAV audio track.
            assert( soundType == 0 );

            const uint16_t chunkSize = stream.getLE16();
            (void)chunkSize;
            const uint8_t channelCount = stream.get();
            const uint32_t sampleRateInHz = stream.getLE32();
            const uint32_t dataLengthBytes = stream.getLE32();
            (void)dataLengthBytes;
            const uint16_t blockAlign = stream.getLE16();

            RWStreamBuf wavHeader( 44 );
            wavHeader.putLE32( 0x46464952 ); // RIFF marker ("RIFF")
            wavHeader.putLE32( static_cast<uint32_t>( stream.size() ) + 0x24 ); // Total size minus the size of this and previous fields
            wavHeader.putLE32( 0x45564157 ); // File type header ("WAVE")
            wavHeader.putLE32( 0x20746D66 ); // Format sub-chunk marker ("fmt ")
            wavHeader.putLE32( 0x10 ); // Size of the format sub-chunk
            wavHeader.putLE16( 0x01 ); // Audio format (1 for PCM)
            wavHeader.putLE16( channelCount ); // Number of channels
            wavHeader.putLE32( sampleRateInHz ); // Sample rate
            wavHeader.putLE32( sampleRateInHz ); // Byte rate (SampleRate * BitsPerSample * NumberOfChannels) / 8
            wavHeader.putLE16( blockAlign ); // Block align (BitsPerSample * NumberOfChannels) / 8
            wavHeader.putLE16( 0x08 ); // Bits per sample
            wavHeader.putLE32( 0x61746164 ); // Data sub-chunk marker ("data")
            wavHeader.putLE32( static_cast<uint32_t>( stream.size() ) ); // Size of the data sub-chunk

            sound = {};

            sound.reserve( stream.size() + 44 );
            sound.assign( wavHeader.data(), wavHeader.data() + 44 );

            const auto & [data, dataSize] = stream.getRawView();

            sound.insert( sound.begin() + 44, data, data + dataSize );
        }

        return !stream.fail();
    }

    bool readFontCharacter( ROStreamBuf & stream, engine4heroes::Sprite & image )
    {
        const uint32_t width = stream.getLE32();
        const uint32_t height = stream.getLE32();

        image.resize( width, height );
        image.fill( 0 );

        const size_t size = width * height;

        const int32_t offsetX = stream.getLE32();
        const int32_t offsetY = stream.getLE32();
        image.setPosition( offsetX, offsetY );

        auto [data, realSize] = stream.getRawView( width * height );
        if ( realSize != size ) {
            return false;
        }

        uint32_t * out = image.image();
        const uint32_t * outEnd = out + size;

        for ( ; out != outEnd; ++out, ++data ) {
            // TODO: transform data into a proper image format.
            if ( *data == 0 ) {
                *out = 0;
            }
            else {
                *out = ( *data << 24U );
            }
        }

        return !stream.fail();
    }

    bool processFont( ROStreamBuf & stream, std::vector<engine4heroes::Sprite> & images )
    {
        const uint16_t type = stream.getLE16();
        if ( type != 1 ) {
            // As of now, only this type should be present in game resources.
            assert( 0 );
            return false;
        }

        // This byte can be 0x1F or 0x20.
        // Since we don't need to display all possible symbols as text, the first N characters are excluded from the font.
        const uint8_t firstCharacter = stream.get();

        // As simple as it states: this is a font size (height). Not very useful as we know font size from a name.
        const uint8_t fontSize = stream.get();
        (void)fontSize;
        // Maximum allowed character height.
        const uint8_t fontHeight = stream.get();
        (void)fontHeight;

        // Maximum allowed character width.
        const uint8_t fontWidth = stream.get();
        (void)fontWidth;

        // The number of characters present in the font.
        const uint8_t characterCount = stream.get();

        if ( firstCharacter > 0 ) {
            images.resize( firstCharacter );
        }

        for ( size_t i = 0; i < characterCount; ++i ) {
            engine4heroes::Sprite image;
            if ( !readFontCharacter( stream, image ) ) {
                return false;
            }

            images.emplace_back( std::move( image ) );
        }

        const size_t leftoverDataLength = stream.size();

        if ( leftoverDataLength != 0 ) {
            // It seems that we don't handle the format properly or the file is corrupted.
            return false;
        }

        return !stream.fail();
    }
}

namespace File
{
    std::vector<engine4heroes::Sprite> getImages( const std::string & entryName, const std::vector<uint8_t> & data )
    {
        assert( !entryName.empty() );

        if ( data.empty() ) {
            return {};
        }

        ROStreamBuf stream( data );
        std::vector<engine4heroes::Sprite> images;

        if ( isPrefixOf( entryName, bitmapRawPrefix ) ) {
            if ( !processRawBitmap( stream, images ) ) {
                return {};
            }

            return images;
        }

        if ( isPrefixOf( entryName, layersPrefix ) ) {
            if ( !processLayersObject( entryName, stream, images ) ) {
                return {};
            }

            return images;
        }

        if ( isPrefixOf( entryName, animationPrefix ) ) {
            if ( !processAnimation( entryName, stream, images ) ) {
                return {};
            }

            return images;
        }

        if ( isPrefixOf( entryName, fontPrefix ) ) {
            if ( !processFont( stream, images ) ) {
                return {};
            }

            return images;
        }

        return {};
    }

    std::vector<uint8_t> getAudioStream( const std::string & entryName, const std::vector<uint8_t> & data )
    {
        assert( !entryName.empty() );

        if ( data.empty() ) {
            return {};
        }

        ROStreamBuf stream( data );

        if ( isPrefixOf( entryName, soundPrefix ) ) {
            std::vector<uint8_t> sound;
            if ( !processSound( stream, sound ) ) {
                return {};
            }

            return sound;
        }

        return {};
    }

    bool H4DFile::process( const std::string & entryName, const std::vector<uint8_t> & data )
    {
        if ( data.empty() ) {
            return true;
        }

        ROStreamBuf stream( data );

        if ( isPrefixOf( entryName, layersPrefix ) ) {
            std::vector<engine4heroes::Sprite> images;
            if ( !processLayersObject( entryName, stream, images ) ) {
                return false;
            }

#if defined( WITH_DEBUG )
            const size_t leftDataSize = stream.size();
            assert( leftDataSize == 0 );
#endif
            return true;
        }

        if ( isPrefixOf( entryName, actorSequencePrefix ) ) {
            std::vector<engine4heroes::Sprite> images;
            if ( !processActorSequence( entryName, stream, images ) ) {
                return false;
            }

#if defined( WITH_DEBUG )
            const size_t leftDataSize = stream.size();
            assert( leftDataSize == 0 );
#endif
            return true;
        }

        return true;

        /*
        const uint32_t dataLength = stream.getLE32();
        if ( dataLength >= data.size() - 4 ) {
            return;
        }

        const uint32_t itemCount = stream.getLE32();
        if ( itemCount == 0 ) {
            // Well, what do we expect from an empty file?
            return;
        }
        */
    }
}
