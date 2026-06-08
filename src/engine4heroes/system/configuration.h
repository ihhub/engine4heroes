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

#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "dir.h"
#include "screen.h"

class Configuration
{
public:
    static constexpr const char * configFileName = "engine4heroes.cfg";

    static Configuration & instance();

    static std::string getVersion();

    static const std::vector<std::string> & getRootDirs();

    static ListFiles findFiles( const std::string & prefixDir, const std::string & fileNameFilter, const bool exactMatch );

    static std::string getLastFoundFile( const std::string & prefix, const std::string & name );

    bool save() const
    {
        return save( configFileName );
    }

    bool save( const std::string_view fileName ) const;

    bool load()
    {
        return load( configFileName );
    }

    bool load( const std::string_view fileName );

    void setProgramPath( const char * path )
    {
        if ( path == nullptr ) {
            return;
        }

        _programPath = path;
    }

    int32_t getSoundVolume() const
    {
        return _soundVolume;
    }

    void setSoundVolume( const int32_t volume )
    {
        _soundVolume = std::clamp( volume, 0, 10 );
    }

    int32_t getMusicVolume() const
    {
        return _musicVolume;
    }

    void setMusicVolume( const int32_t volume )
    {
        _musicVolume = std::clamp( volume, 0, 10 );
    }

    bool is3DAudioEnabled() const
    {
        return _is3DAudioEnabled;
    }

    void set3DAudio( const bool enable )
    {
        _is3DAudioEnabled = enable;
    }

    const std::string & getGameLanguage() const
    {
        return _gameLanguage;
    }

    bool setGameLanguage( std::string language )
    {
        _gameLanguage = std::move( language );

        // TODO: load a translation for this language.
        return true;
    }

    bool isFullScreen() const
    {
        return _isFullScreen;
    }

    void setFullScreen( const bool enable );

    void setTextSupportMode( const bool enable );

    bool isTextSupportModeEnabled() const
    {
        return _isTextSupportModeEnabled;
    }

    void setSystemInfo( const bool enable );

    bool isSystemInfoEnabled() const
    {
        return _isSystemInfoEnabled;
    }

    bool isCursorSoftwareRenderingEnabled() const
    {
        return _isCursorSoftwareRenderingEnabled;
    }

    void setNearestScreenScaling( const bool enable );

    bool isScreenScalingNearest() const
    {
        return _isScreenScalingNearest;
    }

    const engine4heroes::ResolutionInfo & getResolutionInfo() const
    {
        return _resolutionInfo;
    }

    bool isFirstGameRun() const
    {
        return _isFirstGameRun;
    }

private:
    std::string _programPath;

    std::string _gameLanguage;

    int32_t _soundVolume{ 5 };
    int32_t _musicVolume{ 5 };

    engine4heroes::ResolutionInfo _resolutionInfo{ engine4heroes::Display::DEFAULT_WIDTH, engine4heroes::Display::DEFAULT_HEIGHT };

    bool _is3DAudioEnabled{ false };
    bool _isFullScreen{ false };
    bool _isTextSupportModeEnabled{ false };
    bool _isSystemInfoEnabled{ false };
    bool _isCursorSoftwareRenderingEnabled{ false };
    bool _isScreenScalingNearest{ false };
    bool _isFirstGameRun{ true };

    Configuration() = default;
    ~Configuration() = default;

    std::string generateConfigFile() const;
};
