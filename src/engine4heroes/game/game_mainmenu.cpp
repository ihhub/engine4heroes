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

#include "game_mainmenu.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "cursor.h"
#include "game_mainmenu_ui.h"
#include "localevent.h"
#include "screen.h"
#include "ui_button.h"
#include "ui_text.h"

#include "audio.h"
#include "audio_manager.h"
#include "music_info.h"
#include "sound_info.h"

#include "resource_manager.h"
#include "image.h"

namespace
{
    class BackgroundButtonUpdater final : public engine4heroes::ActionObject
    {
    public:
        explicit BackgroundButtonUpdater( const engine4heroes::Image & background, const engine4heroes::Rect textArea, std::string text )
            : _background{ background }
            , _textArea{ textArea }
            , _text{ std::move( text ) }
        {
            // Do nothing.
        }

    protected:
        void preRenderUpdate( const ActionObject * object, const engine4heroes::Rect & renderArea ) override
        {
            assert( object != nullptr );

            auto & display = engine4heroes::Display::instance();
            engine4heroes::Copy( _background, renderArea.x, renderArea.y, display, renderArea.x, renderArea.y, renderArea.width, renderArea.height );
        }

        void postRenderUpdate( const ActionObject * object, const engine4heroes::Rect & /*renderArea*/ ) override
        {
            assert( object != nullptr );

            auto & display = engine4heroes::Display::instance();
            const engine4heroes::Text text{ _text, engine4heroes::FontType::largeWhite() };
            text.draw( _textArea.x, _textArea.y, _textArea.width, display );
        }

    private:
        const engine4heroes::Image & _background;
        const engine4heroes::Rect _textArea;
        const std::string _text;
    };

    engine4heroes::Rect getImageRect( const engine4heroes::Sprite & image )
    {
        return { image.x(), image.y(), image.width(), image.height() };
    }
}

namespace Game
{
    void openMainMenu()
    {
        engine4heroes::drawMainMenuScreen();

        auto & display = engine4heroes::Display::instance();

        engine4heroes::Image background;
        engine4heroes::Copy( display, background );

        engine4heroes::Button newGameButton{ "layers.menu.main.0800.h4d", 13, 12 };
        engine4heroes::Button loadGameButton{ "layers.menu.main.0800.h4d", 5, 4 };
        engine4heroes::Button optionsButton{ "layers.menu.main.0800.h4d", 17, 16 };
        engine4heroes::Button multiplayerButton{ "layers.menu.main.0800.h4d", 9, 8 };
        engine4heroes::Button quitButton{ "layers.menu.main.0800.h4d", 21, 20 };

        const engine4heroes::Rect loadGameTextArea{ getImageRect( GameResource::getImage( "layers.menu.main.0800.h4d", 6 ) ) };
        const engine4heroes::Rect networkTextArea{ getImageRect( GameResource::getImage( "layers.menu.main.0800.h4d", 10 ) ) };
        const engine4heroes::Rect newGameTextArea{ getImageRect( GameResource::getImage( "layers.menu.main.0800.h4d", 14 ) ) };
        const engine4heroes::Rect optionsTextArea{ getImageRect( GameResource::getImage( "layers.menu.main.0800.h4d", 18 ) ) };
        const engine4heroes::Rect quitTextArea{ getImageRect( GameResource::getImage( "layers.menu.main.0800.h4d", 22 ) ) };

        BackgroundButtonUpdater loadGameUpdater( background, loadGameTextArea, "Load Game" );
        BackgroundButtonUpdater networkUpdater( background, networkTextArea, "Multiplayer" );
        BackgroundButtonUpdater newGameUpdater( background, newGameTextArea, "New Game" );
        BackgroundButtonUpdater optionsUpdater( background, optionsTextArea, "Options" );
        BackgroundButtonUpdater quitUpdater( background, quitTextArea, "Quit" );

        newGameButton.subscribe( &newGameUpdater );
        loadGameButton.subscribe( &loadGameUpdater );
        optionsButton.subscribe( &optionsUpdater );
        multiplayerButton.subscribe( &networkUpdater );
        quitButton.subscribe( &quitUpdater );

        newGameButton.draw();
        loadGameButton.draw();
        optionsButton.draw();
        multiplayerButton.draw();
        quitButton.draw();

        display.render();

        AudioManager::PlayMusicAsync( Music::MAIN_MENU, Music::PlaybackMode::REWIND_AND_PLAY_INFINITE );

        engine4heroes::Text text{ "This is a test text.", engine4heroes::FontType::normalWhite() };
        text.draw( 100, 550, display );

        auto & eventHandler = LocalEvent::Get();
        while ( eventHandler.HandleEvents( true, true ) ) {
            newGameButton.drawOnState( eventHandler.isMouseLeftButtonPressedAndHeldInArea( newGameButton.area() ) );
            loadGameButton.drawOnState( eventHandler.isMouseLeftButtonPressedAndHeldInArea( loadGameButton.area() ) );
            optionsButton.drawOnState( eventHandler.isMouseLeftButtonPressedAndHeldInArea( optionsButton.area() ) );
            multiplayerButton.drawOnState( eventHandler.isMouseLeftButtonPressedAndHeldInArea( multiplayerButton.area() ) );
            quitButton.drawOnState( eventHandler.isMouseLeftButtonPressedAndHeldInArea( quitButton.area() ) );

            if ( eventHandler.MouseClickLeft( newGameButton.area() ) ) {
                AudioManager::PlaySound( Sound::BUTTON_CLICK );
            }
            else if ( eventHandler.MouseClickLeft( loadGameButton.area() ) ) {
                AudioManager::PlaySound( Sound::BUTTON_CLICK );
            }
            else if ( eventHandler.MouseClickLeft( optionsButton.area() ) ) {
                AudioManager::PlaySound( Sound::BUTTON_CLICK );
            }
            else if ( eventHandler.MouseClickLeft( multiplayerButton.area() ) ) {
                AudioManager::PlaySound( Sound::BUTTON_CLICK );
            }
            else if ( eventHandler.MouseClickLeft( quitButton.area() ) ) {
                AudioManager::PlaySound( Sound::BUTTON_CLICK );
            }
        }
    }
}
