/***************************************************************************
 *   engine4heroes: https://github.com/ihhub/engine4heroes                 *
 *   Copyright (C) 2026                                                    *
 *                                                                         *
 *   fheroes2: https://github.com/ihhub/fheroes2                           *
 *   Copyright (C) 2021 - 2022                                             *
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

#include "configuration.h"

#include <algorithm>
#include <cstdlib>
#include <sstream>

#if defined( MACOS_APP_BUNDLE )
#include <CoreFoundation/CoreFoundation.h>
#endif

#include "logging.h"
#include "render_processor.h"
#include "screen.h"
#include "serialize.h"
#include "system.h"
#include "tinyconfig.h"
#include "version.h"

#define STRINGIFY( DEF ) #DEF
#define EXPANDDEF( DEF ) STRINGIFY( DEF )

namespace
{
    const std::string optionResolution{ "resolution" };
    const std::string optionFullscreen{ "fullscreen" };
    const std::string optionDebug{ "debug" };
    const std::string optionLanguage{ "lang" };
    const std::string optionSystemInfo{ "system info" };
    const std::string option3DAudio{ "3d audio" };
    const std::string optionMusicVolume{ "music volume" };
    const std::string optionSoundVolume{ "sound volume" };
    const std::string optionTextSupportMode{ "text support mode" };
    const std::string optionCursorSoftwareRendering{ "cursor soft rendering" };
    const std::string optionScreenScalingType{ "screen scaling type" };
}

Configuration & Configuration::instance()
{
    static Configuration conf;
    return conf;
}

std::string Configuration::getVersion()
{
    return std::to_string( MAJOR_VERSION ) + '.' + std::to_string( MINOR_VERSION ) + '.' + std::to_string( INTERMEDIATE_VERSION );
}

const std::vector<std::string> & Configuration::getRootDirs()
{
    static const std::vector<std::string> rootDirs = []() {
        std::vector<std::string> result;

#ifdef ENGINE4HEROES_DATA
        // Macro-defined path.
        result.emplace_back( EXPANDDEF( ENGINE4HEROES_DATA ) );
#endif

#if defined( __IPHONEOS__ )
        // IOS application should have all resources within the application folder.
        result.emplace_back( "." );
#endif

        // Environment variable.
        const char * dataEnvPath = getenv( "ENGINE4HEROES_DATA" );
        if ( dataEnvPath != nullptr && std::find( result.begin(), result.end(), dataEnvPath ) == result.end() ) {
            result.emplace_back( dataEnvPath );
        }

        // The location of the application.
        std::string appPath = System::GetParentDirectory( Configuration::instance()._programPath );
        if ( std::find( result.begin(), result.end(), appPath ) == result.end() ) {
            result.emplace_back( std::move( appPath ) );
        }

#if defined( MACOS_APP_BUNDLE )
        // macOS app bundle Resources directory
        char resourcePath[PATH_MAX];

        CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL( CFBundleGetMainBundle() );
        if ( CFURLGetFileSystemRepresentation( resourcesURL, TRUE, reinterpret_cast<UInt8 *>( resourcePath ), PATH_MAX )
             && std::find( result.begin(), result.end(), resourcePath ) == result.end() ) {
            result.emplace_back( resourcePath );
        }
        else {
            ERROR_LOG( "Unable to get app bundle path" )
        }
        CFRelease( resourcesURL );
#endif

        // User config directory.
        std::string configPath = System::GetConfigDirectory( "engine4heroes" );
        if ( std::find( result.begin(), result.end(), configPath ) == result.end() ) {
            result.emplace_back( std::move( configPath ) );
        }

        // User data directory.
        std::string dataPath = System::GetDataDirectory( "engine4heroes" );
        if ( std::find( result.begin(), result.end(), dataPath ) == result.end() ) {
            result.emplace_back( std::move( dataPath ) );
        }

        // Remove all paths that are not directories. Empty path should not be removed because it in fact means the current directory.
        result.erase( std::remove_if( result.begin(), result.end(), []( const std::string & path ) { return !path.empty() && !System::IsDirectory( path ); } ),
                      result.end() );

        return result;
    }();

    return rootDirs;
}

ListFiles Configuration::findFiles( const std::string & prefixDir, const std::string & fileNameFilter, const bool exactMatch )
{
    ListFiles res;

    for ( const std::string & dir : getRootDirs() ) {
        const std::string path = !prefixDir.empty() ? System::concatPath( dir, prefixDir ) : dir;

        if ( System::IsDirectory( path ) ) {
            if ( exactMatch ) {
                res.FindFileInDir( path, fileNameFilter );
            }
            else {
                res.ReadDir( path, fileNameFilter );
            }
        }
    }

    return res;
}

std::string Configuration::getLastFoundFile( const std::string & prefix, const std::string & name )
{
    const ListFiles & files = findFiles( prefix, name, true );
    return files.empty() ? name : files.back();
}

bool Configuration::save( const std::string_view fileName ) const
{
    if ( fileName.empty() ) {
        return false;
    }

    StreamFile fileStream;
    if ( !fileStream.open( System::concatPath( System::GetConfigDirectory( "engine4heroes" ), fileName ), "w" ) ) {
        return false;
    }

    const std::string data = generateConfigFile();

    fileStream.putRaw( data.data(), data.size() );

    return true;
}

bool Configuration::load( const std::string_view fileName )
{
    TinyConfig config( '=', '#' );

    if ( !config.Load( std::string( fileName ) ) ) {
        return false;
    }

    if ( config.Exists( optionFullscreen ) ) {
        setFullScreen( config.IntParams( optionFullscreen ) );
    }

    if ( config.Exists( optionSystemInfo ) ) {
        setSystemInfo( config.StrParams( optionSystemInfo ) == "on" );
    }

    if ( config.Exists( optionCursorSoftwareRendering ) ) {
        _isCursorSoftwareRenderingEnabled = ( config.StrParams( optionCursorSoftwareRendering ) == "on" );
        engine4heroes::cursor().enableSoftwareEmulation( _isCursorSoftwareRenderingEnabled );
    }

    return true;
}

void Configuration::setFullScreen( const bool enable )
{
    _isFullScreen = !_isFullScreen;

    if ( enable != engine4heroes::engine().isFullScreen() ) {
        engine4heroes::engine().toggleFullScreen();
        engine4heroes::Display::instance().render();
    }
}

void Configuration::setTextSupportMode( const bool enable )
{
    _isTextSupportModeEnabled = enable;
    // TODO: do something.
}

void Configuration::setSystemInfo( const bool enable )
{
    _isSystemInfoEnabled = enable;
    if ( _isSystemInfoEnabled ) {
        engine4heroes::RenderProcessor::instance().enableRenderers();
    }
    else {
        engine4heroes::RenderProcessor::instance().disableRenderers();
    }
}

void Configuration::setNearestScreenScaling( const bool enable )
{
    _isScreenScalingNearest = enable;
    // TODO: do something.
}

std::string Configuration::generateConfigFile() const
{
    std::ostringstream os;

    os << "# engine4heroes configuration file (saved by version " << getVersion() << ")" << std::endl;
    os << std::endl
       << "# !!! WARNING !!!" << std::endl
       << "# Only modify this file if you are absolutely sure of what you are doing!" << std::endl
       << "# !!! WARNING !!!" << std::endl;

    const engine4heroes::Display & display = engine4heroes::Display::instance();

    os << std::endl << "# Resolution: in-game width x height : on-screen width x height" << std::endl;
    os << optionResolution << " = " << display.width() << "x" << display.height() << ":" << display.screenSize().width << "x" << display.screenSize().height << std::endl;

    os << std::endl << "# Sound volume: 0 - 10" << std::endl;
    os << optionSoundVolume << " = " << _soundVolume << std::endl;

    os << std::endl << "# Music volume: 0 - 10" << std::endl;
    os << optionMusicVolume << " = " << _musicVolume << std::endl;

    os << std::endl << "# Toggle fullscreen mode: on/off" << std::endl;
    os << optionFullscreen << " = " << ( _isFullScreen ? "on" : "off" ) << std::endl;

    os << std::endl << "# Print debug messages (only for development, see src/engine/logging.h for possible values)" << std::endl;
    os << optionDebug << " = " << Logging::getDebugLevel() << std::endl;

    os << std::endl << "# Game language (an empty value means English)" << std::endl;
    os << optionLanguage << " = " << _gameLanguage << std::endl;

    os << std::endl << "# Enable text support mode that outputs extra information in console window: on/off" << std::endl;
    os << optionTextSupportMode << " = " << ( _isTextSupportModeEnabled ? "on" : "off" ) << std::endl;

    os << std::endl << "# Enable 3D audio for objects on Adventure Map: on/off" << std::endl;
    os << option3DAudio << " = " << ( _is3DAudioEnabled ? "on" : "off" ) << std::endl;

    os << std::endl << "# Display system information: on/off" << std::endl;
    os << optionSystemInfo << " = " << ( _isTextSupportModeEnabled ? "on" : "off" ) << std::endl;

    os << std::endl << "# Enable cursor software rendering: on/off" << std::endl;
    os << optionCursorSoftwareRendering << " = " << ( _isCursorSoftwareRenderingEnabled ? "on" : "off" ) << std::endl;

    os << std::endl << "# Screen scaling type: nearest or linear" << std::endl;
    os << optionScreenScalingType << " = " << ( _isScreenScalingNearest ? "nearest" : "linear" ) << std::endl;

    return os.str();
}
