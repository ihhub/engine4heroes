/***************************************************************************
 *   engine4heroes: https://github.com/ihhub/engine4heroes                 *
 *   Copyright (C) 2026                                                    *
 *                                                                         *
 *   fheroes2: https://github.com/ihhub/fheroes2                           *
 *   Copyright (C) 2021 - 2024                                             *
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

#include "core.h"

#include <cassert>
#include <cstdint>
#include <stdexcept>

// Managing compiler warnings for SDL headers
#if defined( __GNUC__ )
#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wdouble-promotion"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wswitch-default"
#endif

#include <SDL.h>
#include <SDL_error.h>

// Managing compiler warnings for SDL headers
#if defined( __GNUC__ )
#pragma GCC diagnostic pop
#endif

#include "audio.h"
#include "localevent.h"
#include "logging.h"

namespace
{
    void initHardwareInternally()
    {
        // Do nothing.
    }

    void freeHardwareInternally()
    {
        // Do nothing.
    }

    uint32_t convertToSDLFlag( const engine4heroes::SystemInitializationComponent component )
    {
        switch ( component ) {
        case engine4heroes::SystemInitializationComponent::Audio:
            return SDL_INIT_AUDIO;
        case engine4heroes::SystemInitializationComponent::Video:
            return SDL_INIT_VIDEO;
        case engine4heroes::SystemInitializationComponent::GameController:
            return SDL_INIT_GAMECONTROLLER;
        default:
            // Did you add a new component?
            assert( 0 );
            break;
        }

        return 0;
    }

    uint32_t getSDLInitFlags( const std::set<engine4heroes::SystemInitializationComponent> & components )
    {
        uint32_t flags = 0;
        for ( const engine4heroes::SystemInitializationComponent component : components ) {
            flags |= convertToSDLFlag( component );
        }
        return flags;
    }

    // For now only SDL library is supported.
    bool initCoreInternally( const std::set<engine4heroes::SystemInitializationComponent> & components )
    {
        const uint32_t sdlFlags = getSDLInitFlags( components );

        if ( SDL_Init( sdlFlags ) < 0 ) {
            ERROR_LOG( SDL_GetError() )
            return false;
        }

        if ( components.count( engine4heroes::SystemInitializationComponent::Audio ) > 0 ) {
            Audio::Init();
        }

        if ( components.count( engine4heroes::SystemInitializationComponent::GameController ) > 0 ) {
            LocalEvent::Get().initController();
        }

        LocalEvent::initEventEngine();

        return true;
    }

    void freeCoreInternally()
    {
        if ( engine4heroes::isComponentInitialized( engine4heroes::SystemInitializationComponent::GameController ) ) {
            LocalEvent::Get().CloseController();
        }

        if ( engine4heroes::isComponentInitialized( engine4heroes::SystemInitializationComponent::Audio ) ) {
            Audio::Quit();
        }

        SDL_Quit();
    }

    bool isComponentInitializedInternally( const engine4heroes::SystemInitializationComponent component )
    {
        const uint32_t sdlFlag = convertToSDLFlag( component );
        assert( sdlFlag != 0 );

        return SDL_WasInit( sdlFlag ) != 0;
    }
}

namespace engine4heroes
{
    HardwareInitializer::HardwareInitializer()
    {
        initHardwareInternally();
    }

    HardwareInitializer::~HardwareInitializer()
    {
        freeHardwareInternally();
    }

    CoreInitializer::CoreInitializer( const std::set<SystemInitializationComponent> & components )
    {
        if ( !initCoreInternally( components ) ) {
            throw std::logic_error( "Core module initialization failed." );
        }
    }

    CoreInitializer::~CoreInitializer()
    {
        freeCoreInternally();
    }

    bool isComponentInitialized( const SystemInitializationComponent component )
    {
        return isComponentInitializedInternally( component );
    }
}
