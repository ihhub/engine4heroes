/***************************************************************************
 *   engine4heroes: https://github.com/ihhub/engine4heroes                 *
 *   Copyright (C) 2026                                                    *
 *                                                                         *
 *   fheroes2: https://github.com/ihhub/fheroes2                           *
 *   Copyright (C) 2020 - 2026                                             *
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

#include "ui_button.h"

#include <cassert>
#include <cstddef>
#include <memory>

#include "image.h"
#include "math_tools.h"
#include "resource_manager.h"

namespace engine4heroes
{
    bool ButtonBase::draw( Image & output ) const
    {
        if ( !isVisible() ) {
            return false;
        }

        const auto commonArea{ getBoundaryRect( _areaPressed, _areaReleased ) };
        preRenderNotification( commonArea );

        if ( isPressed() ) {
            // button can't be disabled and pressed
            const Sprite & sprite = _getPressed();
            Blit( sprite, output, _areaPressed );
        }
        else {
            const Sprite & sprite = isEnabled() ? _getReleased() : _getDisabled();
            Blit( sprite, output, _areaReleased );
        }

        postRenderNotification( commonArea );

        return true;
    }

    bool ButtonBase::drawOnPress( Display & output /* = Display::instance() */ )
    {
        if ( isPressed() ) {
            return false;
        }

        if ( !press() ) {
            return false;
        }

        if ( isVisible() ) {
            draw( output );

            output.render( area() );
        }
        return true;
    }

    bool ButtonBase::drawOnRelease( Display & output /* = Display::instance() */ )
    {
        if ( !isPressed() ) {
            return false;
        }

        if ( !release() ) {
            return false;
        }

        if ( isVisible() ) {
            draw( output );

            output.render( area() );
        }
        return true;
    }

    const Sprite & ButtonBase::_getDisabled() const
    {
        const Sprite & sprite = _getReleased();
        if ( !_disabledSprite || ( _releasedSprite != &sprite ) ) {
            _releasedSprite = &sprite;
            _disabledSprite = std::make_unique<Sprite>( sprite );
            // ApplyPalette( *_disabledSprite, PAL::GetPalette( PAL::PaletteType::DARKENING ) );
        }

        return *_disabledSprite;
    }

    Button::Button( const int32_t resourceId, const uint32_t releasedIndex, const uint32_t pressedIndex )
        : _resourceId( resourceId )
        , _releasedIndex( releasedIndex )
        , _pressedIndex( pressedIndex )
    {
        // const auto & releasedButton = _getReleased();
        //
        // setPosition( releasedButton.x(), releasedButton.y() );

        _updateButtonAreas();
    }

    const Sprite & Button::_getPressed() const
    {
        return GameResource::getImage( _resourceId, _pressedIndex );
    }

    const Sprite & Button::_getReleased() const
    {
        return GameResource::getImage( _resourceId, _releasedIndex );
    }

    const Sprite & ButtonSprite::_getPressed() const
    {
        return _pressed;
    }

    const Sprite & ButtonSprite::_getReleased() const
    {
        return _released;
    }

    const Sprite & ButtonSprite::_getDisabled() const
    {
        if ( _disabled.empty() ) {
            return ButtonBase::_getDisabled();
        }

        return _disabled;
    }

    ButtonRestorer::ButtonRestorer( ButtonBase & button )
        : _button( button )
        , _isEnabled( button.isEnabled() )
    {
        if ( _isEnabled ) {
            Display & display = Display::instance();

            _button.disable();
            _button.draw( display );
            display.updateNextRenderRoi( _button.area() );
        }
    }

    ButtonRestorer::~ButtonRestorer()
    {
        if ( _isEnabled ) {
            Display & display = Display::instance();

            _button.enable();
            _button.draw( display );
            display.updateNextRenderRoi( _button.area() );
        }
    }

    void OptionButtonGroup::addButton( ButtonBase * button )
    {
        if ( button == nullptr ) {
            return;
        }

        _button.push_back( button );
        button->subscribe( this );
    }

    void OptionButtonGroup::senderUpdate( const ActionObject * sender )
    {
        if ( sender == nullptr ) {
            // How is it even possible?
            assert( 0 );

            return;
        }

        for ( size_t i = 0; i < _button.size(); ++i ) {
            if ( sender == _button[i] ) {
                const ButtonBase * button = _button[i];
                if ( button->isPressed() ) {
                    unsubscribeAll();

                    for ( size_t buttonId = 0; buttonId < _button.size(); ++buttonId ) {
                        if ( i != buttonId ) {
                            _button[buttonId]->release();
                        }
                    }

                    subscribeAll();
                }
            }
        }
    }

    void OptionButtonGroup::subscribeAll()
    {
        for ( ButtonBase * button : _button ) {
            button->subscribe( this );
        }
    }

    void OptionButtonGroup::unsubscribeAll() const
    {
        for ( ButtonBase * button : _button ) {
            button->unsubscribe();
        }
    }
}
