/***************************************************************************
 *   engine4heroes: https://github.com/ihhub/engine4heroes                 *
 *   Copyright (C) 2026                                                    *
 *                                                                         *
 *   fheroes2: https://github.com/ihhub/fheroes2                           *
 *   Copyright (C) 2019 - 2026                                             *
 *                                                                         *
 *   fheroes2: https://github.com/ihhub/fheroes2                           *
 *   Copyright (C) 2019 - 2025                                             *
 *                                                                         *
 *   Free Heroes2 Engine: http://sourceforge.net/projects/fheroes2         *
 *   Copyright (C) 2009 by Andrey Afletdinov <fheroes2@gmail.com>          *
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
#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <set>
#include <string>

// Managing compiler warnings for SDL headers
#if defined( __GNUC__ )
#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wdouble-promotion"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wswitch-default"
#endif

#include <SDL_error.h>
#include <SDL_events.h>
#include <SDL_main.h> // IWYU pragma: keep
#include <SDL_mouse.h>

// Managing compiler warnings for SDL headers
#if defined( __GNUC__ )
#pragma GCC diagnostic pop
#endif

#if defined( _WIN32 )
#include <cassert>
#endif

#include "audio_manager.h"
#include "configuration.h"
#include "core.h"
#include "cursor.h"
#include "exception.h"
#include "game_hotkeys.h"
#include "game_mainmenu.h"
#include "localevent.h"
#include "logging.h"
#include "render_processor.h"
#include "resource_manager.h"
#include "screen.h"
#include "system.h"
#include "ui_tool.h"

namespace
{
    std::string GetCaption()
    {
        return std::string( "engine4heroes engine, version: " + Configuration::getVersion() );
    }

    void InitConfigDir()
    {
        const std::string configDir = System::GetConfigDirectory( "engine4heroes" );

        System::MakeDirectory( configDir );
    }

    void ReadConfigs()
    {
        const std::string configurationFileName( Configuration::configFileName );
        const std::string confFile = Configuration::getLastFoundFile( "", configurationFileName );

        auto & conf = Configuration::instance();
        if ( !System::IsFile( confFile ) || !conf.load() ) {
            conf.save();

            // Fullscreen mode can be enabled by default for some devices, we need to forcibly
            // synchronize reality with the default config if config file was not read
            conf.setFullScreen( conf.isFullScreen() );
        }
    }

    class DisplayInitializer final
    {
    public:
        DisplayInitializer()
        {
            // TODO: load resolution from the configuration.

            auto & display = engine4heroes::Display::instance();
            const engine4heroes::ResolutionInfo bestResolution{ engine4heroes::Display::DEFAULT_WIDTH,
                                                                engine4heroes::Display::DEFAULT_HEIGHT }; // conf.currentResolutionInfo() };

            // display.setWindowPos( conf.getSavedWindowPos() );
            display.setResolution( bestResolution );

            engine4heroes::engine().setTitle( GetCaption() );

            // Hide system cursor.
            const int returnValue = SDL_ShowCursor( SDL_DISABLE );
            if ( returnValue < 0 ) {
                ERROR_LOG( "Failed to hide system cursor. Error description: " << SDL_GetError() )
            }

            auto & renderProcessor = engine4heroes::RenderProcessor::instance();

            display.subscribe( [&renderProcessor]() { return renderProcessor.preRenderAction(); }, [&renderProcessor]() { renderProcessor.postRenderAction(); } );

            // Initialize system info renderer.
            _systemInfoRenderer = std::make_unique<engine4heroes::SystemInfoRenderer>();

            renderProcessor.registerRenderers( [sysInfoRenderer = _systemInfoRenderer.get()]() { sysInfoRenderer->preRender(); },
                                               [sysInfoRenderer = _systemInfoRenderer.get()]() { sysInfoRenderer->postRender(); } );

            renderProcessor.enableRenderers();

            // Update mouse cursor when switching between software emulation and OS mouse modes.
            engine4heroes::cursor().registerUpdater( Cursor::Refresh );

            // TODO: fix this.
            // #if !defined( MACOS_APP_BUNDLE )
            //             const engine4heroes::Image & appIcon = Compression::CreateImageFromZlib( 32, 32, iconImage, sizeof( iconImage ), true );
            //             engine4heroes::engine().setIcon( appIcon );
            // #endif
        }

        DisplayInitializer( const DisplayInitializer & ) = delete;
        DisplayInitializer & operator=( const DisplayInitializer & ) = delete;

        ~DisplayInitializer()
        {
            engine4heroes::RenderProcessor::instance().unregisterRenderers();

            auto & display = engine4heroes::Display::instance();
            display.subscribe( {}, {} );
            display.release();
        }

    private:
        // This member must not be initialized before Display.
        std::unique_ptr<engine4heroes::SystemInfoRenderer> _systemInfoRenderer;
    };
}

int main( int argc, char ** argv )
{
// SDL2main.lib converts argv to UTF-8, but this application expects ANSI, use the original argv
#if defined( _WIN32 )
    assert( argc == __argc );

    argv = __argv;
#else
    (void)argc;
#endif

    try {
        auto & conf = Configuration::instance();
        conf.setProgramPath( argv[0] );

        InitConfigDir();
        ReadConfigs();

        const engine4heroes::HardwareInitializer hardwareInitializer;
        Logging::InitLog();

        COUT( GetCaption() )

        const std::set<engine4heroes::SystemInitializationComponent> coreComponents{ engine4heroes::SystemInitializationComponent::Audio,
                                                                                     engine4heroes::SystemInitializationComponent::Video };

        const engine4heroes::CoreInitializer coreInitializer( coreComponents );

        const DisplayInitializer displayInitializer;

        if ( !GameResource::initializeResources() ) {
            return EXIT_FAILURE;
        }

        const AudioManager::AudioInitializer audioInitializer;

        const CursorRestorer cursorRestorer( true, Cursor::POINTER );

        engine4heroes::cursor().enableSoftwareEmulation( true );

        LocalEvent & eventHandler = LocalEvent::Get();
        eventHandler.setGlobalMouseMotionEventHook( Cursor::updateCursorPosition );
        eventHandler.setGlobalKeyDownEventHook( Game::globalKeyDownEvent );

        Game::HotKeysLoad( Configuration::getLastFoundFile( "", "engine4heroes.key" ) );

        try {
            Game::openMainMenu();
        }
        catch ( const engine4heroes::InvalidDataResources & ex ) {
            ERROR_LOG( ex.what() )
            // displayMissingResourceWindow();
            return EXIT_FAILURE;
        }
    }
    catch ( const std::exception & ex ) {
        ERROR_LOG( "Exception '" << ex.what() << "' occurred during application runtime." )
        return EXIT_FAILURE;
    }
    catch ( ... ) {
        ERROR_LOG( "An unknown exception occurred during application runtime." )
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
