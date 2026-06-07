/***************************************************************************
 *   engine4heroes: https://github.com/ihhub/engine4heroes                 *
 *   Copyright (C) 2026                                                    *
 *                                                                         *
 *   fheroes2: https://github.com/ihhub/fheroes2                           *
 *   Copyright (C) 2019 - 2026                                             *
 *                                                                         *
 *   Free Heroes2 Engine: http://sourceforge.net/projects/fheroes2         *
 *   Copyright (C) 2010 by Andrey Afletdinov <fheroes2@gmail.com>          *
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

#include "game_hotkeys.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstring>
#include <functional>
#include <map>
#include <set>
#include <sstream>
#include <string_view>
#include <type_traits>
#include <utility>

#include "configuration.h"
#include "localevent.h"
#include "logging.h"
#include "serialize.h"
#include "system.h"
#include "tinyconfig.h"
#include "tools.h"
#include "translations.h"
#include "ui_language.h"

namespace
{
    struct HotKeyEventInfo
    {
        HotKeyEventInfo() = default;

        HotKeyEventInfo( const Game::HotKeyCategory category_, const char * name_, const engine4heroes::Key key_ )
            : category( category_ )
            , name( name_ )
            , key( key_ )
        {
            // Do nothing.
        }

        HotKeyEventInfo( const HotKeyEventInfo & ) = default;
        HotKeyEventInfo( HotKeyEventInfo && ) = default;
        ~HotKeyEventInfo() = default;

        HotKeyEventInfo & operator=( const HotKeyEventInfo & ) = default;
        HotKeyEventInfo & operator=( HotKeyEventInfo && ) = default;

        Game::HotKeyCategory category = Game::HotKeyCategory::DEFAULT;

        const char * name = "";

        engine4heroes::Key key = engine4heroes::Key::NONE;
    };

    constexpr typename std::underlying_type<Game::HotKeyEvent>::type hotKeyEventToInt( Game::HotKeyEvent value )
    {
        return static_cast<typename std::underlying_type<Game::HotKeyEvent>::type>( value );
    }

    std::array<HotKeyEventInfo, hotKeyEventToInt( Game::HotKeyEvent::NO_EVENT )> hotKeyEventInfo;

    void initializeHotKeyEvents()
    {
        // Make sure that event name is unique!
        hotKeyEventInfo[hotKeyEventToInt( Game::HotKeyEvent::DEFAULT_OKAY )]
            = { Game::HotKeyCategory::DEFAULT, gettext_noop( "hotkey|default okay event" ), engine4heroes::Key::KEY_ENTER };
        hotKeyEventInfo[hotKeyEventToInt( Game::HotKeyEvent::DEFAULT_CANCEL )]
            = { Game::HotKeyCategory::DEFAULT, gettext_noop( "hotkey|default cancel event" ), engine4heroes::Key::KEY_ESCAPE };
        hotKeyEventInfo[hotKeyEventToInt( Game::HotKeyEvent::DEFAULT_LEFT )]
            = { Game::HotKeyCategory::DEFAULT, gettext_noop( "hotkey|default left" ), engine4heroes::Key::KEY_LEFT };
        hotKeyEventInfo[hotKeyEventToInt( Game::HotKeyEvent::DEFAULT_RIGHT )]
            = { Game::HotKeyCategory::DEFAULT, gettext_noop( "hotkey|default right" ), engine4heroes::Key::KEY_RIGHT };
        hotKeyEventInfo[hotKeyEventToInt( Game::HotKeyEvent::DEFAULT_UP )]
            = { Game::HotKeyCategory::DEFAULT, gettext_noop( "hotkey|default up" ), engine4heroes::Key::KEY_UP };
        hotKeyEventInfo[hotKeyEventToInt( Game::HotKeyEvent::DEFAULT_DOWN )]
            = { Game::HotKeyCategory::DEFAULT, gettext_noop( "hotkey|default down" ), engine4heroes::Key::KEY_DOWN };

        hotKeyEventInfo[hotKeyEventToInt( Game::HotKeyEvent::GLOBAL_TOGGLE_FULLSCREEN )]
            = { Game::HotKeyCategory::GLOBAL, gettext_noop( "hotkey|toggle fullscreen" ), engine4heroes::Key::KEY_F4 };
        hotKeyEventInfo[hotKeyEventToInt( Game::HotKeyEvent::GLOBAL_TOGGLE_TEXT_SUPPORT_MODE )]
            = { Game::HotKeyCategory::GLOBAL, gettext_noop( "hotkey|toggle text support mode" ), engine4heroes::Key::KEY_F10 };
    }

    std::string getHotKeyFileContent()
    {
        std::ostringstream os;
        os << "# engine4heroes hotkey file (saved by version " << Configuration::getVersion() << ")" << std::endl;
        os << std::endl;

        Game::HotKeyCategory currentCategory = hotKeyEventInfo[hotKeyEventToInt( Game::HotKeyEvent::NONE ) + 1].category;
        os << "# " << getHotKeyCategoryName( currentCategory ) << ':' << std::endl;

#if defined( WITH_DEBUG )
        std::set<const char *> duplicationStringVerifier;
#endif

        const engine4heroes::LanguageSwitcher languageSwitcher( engine4heroes::SupportedLanguage::English );

        for ( int32_t eventId = hotKeyEventToInt( Game::HotKeyEvent::NONE ) + 1; eventId < hotKeyEventToInt( Game::HotKeyEvent::NO_EVENT ); ++eventId ) {
            if ( currentCategory != hotKeyEventInfo[eventId].category ) {
                currentCategory = hotKeyEventInfo[eventId].category;
                os << std::endl;
                os << "# " << getHotKeyCategoryName( currentCategory ) << ':' << std::endl;
            }

            const char * eventName = _( hotKeyEventInfo[eventId].name );
            assert( strlen( eventName ) > 0 );
#if defined( WITH_DEBUG )
            const bool isUnique = duplicationStringVerifier.emplace( eventName ).second;
            assert( isUnique );
#endif

            os << eventName << " = " << StringUpper( KeySymGetName( hotKeyEventInfo[eventId].key ) ) << std::endl;
        }

        return os.str();
    }
}

bool Game::HotKeyPressEvent( const HotKeyEvent eventID )
{
    const LocalEvent & le = LocalEvent::Get();
    return le.isAnyKeyPressed() && le.getPressedKeyValue() == hotKeyEventInfo[hotKeyEventToInt( eventID )].key;
}

bool Game::HotKeyHoldEvent( const HotKeyEvent eventID )
{
    const LocalEvent & le = LocalEvent::Get();
    return le.isKeyBeingHold() && le.getPressedKeyValue() == hotKeyEventInfo[hotKeyEventToInt( eventID )].key;
}

engine4heroes::Key Game::getHotKeyForEvent( const HotKeyEvent eventID )
{
    return hotKeyEventInfo[hotKeyEventToInt( eventID )].key;
}

void Game::setHotKeyForEvent( const HotKeyEvent eventID, const engine4heroes::Key key )
{
    hotKeyEventInfo[hotKeyEventToInt( eventID )].key = key;
}

std::string Game::getHotKeyNameByEventId( const HotKeyEvent eventID )
{
    return StringUpper( KeySymGetName( hotKeyEventInfo[hotKeyEventToInt( eventID )].key ) );
}

const char * Game::getHotKeyEventNameByEventId( const HotKeyEvent eventID )
{
    return hotKeyEventInfo[hotKeyEventToInt( eventID )].name;
}

std::vector<std::pair<Game::HotKeyEvent, Game::HotKeyCategory>> Game::getAllHotKeyEvents()
{
    std::vector<std::pair<Game::HotKeyEvent, Game::HotKeyCategory>> events;
    events.reserve( hotKeyEventInfo.size() - 1 );

    for ( size_t i = 1; i < hotKeyEventInfo.size(); ++i ) {
        events.emplace_back( static_cast<Game::HotKeyEvent>( i ), hotKeyEventInfo[i].category );
    }

    return events;
}

void Game::HotKeysLoad( const std::string & filename )
{
    initializeHotKeyEvents();

    bool isFilePresent = System::IsFile( filename );
    if ( isFilePresent ) {
        TinyConfig config( '=', '#' );
        isFilePresent = config.Load( filename );

        if ( isFilePresent ) {
            std::map<std::string, engine4heroes::Key, std::less<>> nameToKey;
            for ( int32_t i = static_cast<int32_t>( engine4heroes::Key::NONE ); i < static_cast<int32_t>( engine4heroes::Key::LAST_KEY ); ++i ) {
                const engine4heroes::Key key = static_cast<engine4heroes::Key>( i );
                nameToKey.try_emplace( StringUpper( KeySymGetName( key ) ), key );
            }

            const engine4heroes::LanguageSwitcher languageSwitcher( engine4heroes::SupportedLanguage::English );

            for ( int eventId = hotKeyEventToInt( HotKeyEvent::NONE ) + 1; eventId < hotKeyEventToInt( HotKeyEvent::NO_EVENT ); ++eventId ) {
                const char * eventName = _( hotKeyEventInfo[eventId].name );
                std::string value = config.StrParams( eventName );
                if ( value.empty() ) {
                    continue;
                }

                value = StringUpper( value );
                auto foundKey = nameToKey.find( value );
                if ( foundKey == nameToKey.end() ) {
                    continue;
                }

                hotKeyEventInfo[eventId].key = foundKey->second;
                DEBUG_LOG( DBG_GAME, DBG_INFO, "Event '" << eventName << "' has key '" << value << "'" )
            }
        }
    }

    HotKeySave();
}

void Game::HotKeySave()
{
    const std::string filename = System::concatPath( System::GetConfigDirectory( "engine4heroes" ), "engine4heroes.key" );

    StreamFile fileStream;
    if ( !fileStream.open( filename, "w" ) ) {
        ERROR_LOG( "Unable to open the hotkey settings file " << filename )
        return;
    }

    const std::string data = getHotKeyFileContent();

    fileStream.putRaw( data.data(), data.size() );
}

void Game::globalKeyDownEvent( const engine4heroes::Key key, const int32_t modifier )
{
    if ( ( modifier & engine4heroes::KeyModifier::KEY_MODIFIER_ALT ) || ( modifier & engine4heroes::KeyModifier::KEY_MODIFIER_CTRL ) ) {
        return;
    }

    auto & conf = Configuration::instance();

    if ( key == hotKeyEventInfo[hotKeyEventToInt( HotKeyEvent::GLOBAL_TOGGLE_FULLSCREEN )].key ) {
        conf.setFullScreen( !conf.isFullScreen() );
        conf.save();
    }
    else if ( key == hotKeyEventInfo[hotKeyEventToInt( HotKeyEvent::GLOBAL_TOGGLE_TEXT_SUPPORT_MODE )].key ) {
        conf.setTextSupportMode( !conf.isTextSupportModeEnabled() );
        conf.save();
    }
}

const char * Game::getHotKeyCategoryName( const HotKeyCategory category )
{
    switch ( category ) {
    case HotKeyCategory::DEFAULT:
        return gettext_noop( "Default Actions" );
    case HotKeyCategory::GLOBAL:
        return gettext_noop( "Global Actions" );
    default:
        // Did you add a new category? Add the logic above!
        assert( 0 );
        break;
    }

    return "";
}
