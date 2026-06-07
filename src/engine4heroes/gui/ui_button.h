/***************************************************************************
 *   engine4heroes: https://github.com/ihhub/engine4heroes                 *
 *   Copyright (C) 2026                                                    *
 *                                                                         *
 *   fheroes2: https://github.com/ihhub/fheroes2                           *
 *   Copyright (C) 2020 - 2025                                             *
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

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "image.h"
#include "math_base.h"
#include "screen.h"
#include "ui_base.h"

namespace engine4heroes
{
    // An abstract class for button usage
    class ButtonBase : public ActionObject
    {
    public:
        ButtonBase() = default;
        ButtonBase( const int32_t offsetX, const int32_t offsetY )
            : _offsetX( offsetX )
            , _offsetY( offsetY )
        {
            // Do nothing.
        }

        ButtonBase( const ButtonBase & ) = delete;
        ButtonBase( ButtonBase && ) noexcept = default;

        ~ButtonBase() override = default;

        ButtonBase & operator=( const ButtonBase & button ) = delete;
        ButtonBase & operator=( ButtonBase && ) noexcept = default;

        bool isEnabled() const
        {
            return _isEnabled;
        }

        bool isDisabled() const
        {
            return !_isEnabled;
        }

        bool isPressed() const
        {
            return _isPressed;
        }

        bool isReleased() const
        {
            return !_isPressed;
        }

        bool isVisible() const
        {
            return _isVisible;
        }

        bool isHidden() const
        {
            return !_isVisible;
        }

        bool press()
        {
            if ( !isEnabled() ) {
                return false;
            }

            _isPressed = true;
            notifySubscriber();
            return true;
        }

        bool release()
        {
            if ( !isEnabled() ) {
                return false;
            }

            _isPressed = false;
            notifySubscriber();
            return true;
        }

        void enable()
        {
            _isEnabled = true;
            notifySubscriber();

            _updateReleasedArea();
        }

        // Button becomes disabled and released
        void disable()
        {
            _isEnabled = false;
            _isPressed = false; // button can't be disabled and pressed
            notifySubscriber();

            _updateReleasedArea();
        }

        // This method doesn't call draw()
        void show()
        {
            _isVisible = true;
            notifySubscriber();
        }

        // This method doesn't call draw()
        void hide()
        {
            _isVisible = false;
            notifySubscriber();
        }

        void setPosition( const int32_t offsetX, const int32_t offsetY )
        {
            _areaPressed.x += offsetX - _offsetX;
            _areaReleased.x += offsetX - _offsetX;
            _areaPressed.y += offsetY - _offsetY;
            _areaReleased.y += offsetY - _offsetY;

            _offsetX = offsetX;
            _offsetY = offsetY;
        }

        // Will draw on screen by default
        bool draw( Image & output = Display::instance() ) const;

        // Will draw and render on screen by default. Returns true in case of state change. This method calls render() internally.
        bool drawOnPress( Display & output = Display::instance() );

        // Will draw and render on screen by default. Returns true in case of state change. This method calls render() internally.
        bool drawOnRelease( Display & output = Display::instance() );

        // Will draw and render on screen by default. Returns true in case of state change. This method calls render() internally.
        bool drawOnState( const bool isPressedState, Display & output = Display::instance() )
        {
            if ( isPressedState ) {
                return drawOnPress( output );
            }

            return drawOnRelease( output );
        }

        const Rect & area() const
        {
            return isPressed() ? _areaPressed : _areaReleased;
        }

    protected:
        virtual const Sprite & _getPressed() const = 0;
        virtual const Sprite & _getReleased() const = 0;
        virtual const Sprite & _getDisabled() const;

        void _updateButtonAreas()
        {
            _updatePressedArea();
            _updateReleasedArea();
        }

    private:
        int32_t _offsetX{ 0 };
        int32_t _offsetY{ 0 };

        Rect _areaPressed{ _offsetX, _offsetY, 0, 0 };
        Rect _areaReleased{ _offsetX, _offsetY, 0, 0 };

        bool _isPressed{ false };
        bool _isEnabled{ true };
        bool _isVisible{ true };

        void _updatePressedArea()
        {
            const Sprite & pressed = _getPressed();
            _areaPressed = { _offsetX + pressed.x(), _offsetY + pressed.y(), pressed.width(), pressed.height() };
        }

        void _updateReleasedArea()
        {
            const Sprite & released = isEnabled() ? _getReleased() : _getDisabled();
            _areaReleased = { _offsetX + released.x(), _offsetY + released.y(), released.width(), released.height() };
        }

        mutable const Sprite * _releasedSprite = nullptr;
        mutable std::unique_ptr<Sprite> _disabledSprite;
    };

    class Button final : public ButtonBase
    {
    public:
        explicit Button( const int32_t offsetX = 0, const int32_t offsetY = 0 )
            : ButtonBase( offsetX, offsetY )
        {
            // Do nothing.
        }

        Button( std::string resourceName, const uint32_t releasedIndex, const uint32_t pressedIndex );

        ~Button() override = default;

    protected:
        const Sprite & _getPressed() const override;
        const Sprite & _getReleased() const override;

    private:
        std::string _resourceName{};
        uint32_t _releasedIndex{ 0 };
        uint32_t _pressedIndex{ 0 };
    };

    // This button class is used for custom Sprites
    class ButtonSprite final : public ButtonBase
    {
    public:
        explicit ButtonSprite( const int32_t offsetX = 0, const int32_t offsetY = 0 )
            : ButtonBase( offsetX, offsetY )
        {
            // Do nothing.
        }

        ButtonSprite( const int32_t offsetX, const int32_t offsetY, Sprite released, Sprite pressed, Sprite disabled = {} )
            : ButtonBase( offsetX, offsetY )
            , _released( std::move( released ) )
            , _pressed( std::move( pressed ) )
            , _disabled( std::move( disabled ) )
        {
            _updateButtonAreas();
        }

        ButtonSprite( const ButtonSprite & ) = delete;
        ButtonSprite( ButtonSprite && ) noexcept = default;

        ~ButtonSprite() override = default;

        ButtonSprite & operator=( const ButtonSprite & ) = delete;
        ButtonSprite & operator=( ButtonSprite && ) noexcept = default;

        void setSprite( const Sprite & released, const Sprite & pressed, const Sprite & disabled = {} )
        {
            _released = released;
            _pressed = pressed;
            _disabled = disabled;

            _updateButtonAreas();
        }

    protected:
        const Sprite & _getPressed() const override;
        const Sprite & _getReleased() const override;
        const Sprite & _getDisabled() const override;

    private:
        Sprite _released;
        Sprite _pressed;
        Sprite _disabled;
    };

    // This class is used for a situations when we need to disable a button for certain action
    // and restore it within the scope of code. The changed button is immediately rendered on display.
    class ButtonRestorer final
    {
    public:
        explicit ButtonRestorer( ButtonBase & button );
        ButtonRestorer( const ButtonRestorer & ) = delete;

        ~ButtonRestorer();

        ButtonRestorer & operator=( const ButtonRestorer & ) = delete;

    private:
        ButtonBase & _button;
        const bool _isEnabled;
    };

    class OptionButtonGroup final : public ActionObject
    {
    public:
        void addButton( ButtonBase * button );

        void draw( Image & output ) const
        {
            for ( const ButtonBase * button : _button ) {
                button->draw( output );
            }
        }

    protected:
        void senderUpdate( const ActionObject * sender ) override;

    private:
        std::vector<ButtonBase *> _button;

        void subscribeAll();
        void unsubscribeAll() const;
    };
}
