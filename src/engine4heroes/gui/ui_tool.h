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

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "game_string.h"
#include "image.h"
#include "math_base.h"
#include "timing.h"
#include "ui_base.h"
#include "ui_text.h"

enum class InterfaceType : uint8_t;

namespace engine4heroes
{
    enum class SupportedLanguage : uint8_t;

    class MovableSprite : public Sprite
    {
    public:
        MovableSprite();
        MovableSprite( const int32_t width, const int32_t height, const int32_t x, const int32_t y );
        explicit MovableSprite( const Sprite & sprite );

        MovableSprite( const MovableSprite & ) = delete;

        ~MovableSprite() override;

        MovableSprite & operator=( const MovableSprite & ) = delete;

        MovableSprite & operator=( const Sprite & sprite );

        void show();

        void hide()
        {
            if ( !_isHidden ) {
                _restorer.restore();
                _isHidden = true;
            }
        }

        // In case if Display has changed.
        void redraw()
        {
            hide();
            show();
        }

        bool isHidden() const
        {
            return _isHidden;
        }

        Rect getArea() const
        {
            return { x(), y(), width(), height() };
        }

        void setPosition( const int32_t x, const int32_t y ) override;

    protected:
        void _resetRestorer()
        {
            _restorer.reset();
        }

    private:
        ImageRestorer _restorer;
        bool _isHidden{ true };
    };

    class MovableText
    {
    public:
        explicit MovableText( Image & output )
            : _output( output )
            , _restorer( output, 0, 0, 0, 0 )
        {
            // Do nothing.
        }

        MovableText( const MovableText & ) = delete;

        ~MovableText() = default;

        MovableText & operator=( const MovableText & ) = delete;

        void update( std::unique_ptr<TextBase> text )
        {
            _text = std::move( text );
        }

        void draw( const int32_t x, const int32_t y )
        {
            drawInRoi( x, y, { 0, 0, _output.width(), _output.height() } );
        }

        // Draw text within a specified ROI (Region of Interest) that acts as a bounding box
        void drawInRoi( const int32_t x, const int32_t y, const Rect & roi );

        void hide()
        {
            if ( !_isHidden ) {
                _restorer.restore();
                _isHidden = true;
            }
        }

    private:
        Image & _output;
        ImageRestorer _restorer;
        std::unique_ptr<TextBase> _text;
        bool _isHidden{ true };
    };

    class TextInputField final
    {
    public:
        TextInputField( const Rect & textArea, const bool isMultiLine, const bool isCenterAligned, Image & output, const std::optional<SupportedLanguage> language = {} );

        // Returns `true` when rendering of this UI element is needed.
        bool eventProcessing();

        // TODO: Process text input from keyboard and other cursor-related operations to avoid use of `_cursorPosition` outside if this class.

        Rect getCursorArea() const
        {
            return _cursor.getArea();
        }

        Rect getOverallArea() const
        {
            return _background.rect();
        }

        void draw( const std::string & newText, const int32_t cursorPositionInText );

        void set( std::string text, const int32_t cursorPosition )
        {
            _text.set( std::move( text ), cursorPosition );
        }

        size_t getCursorInTextPosition( const Point & pos ) const
        {
            return _text.getCursorPosition( pos, _textInputArea, _isSingleLineTextCenterAligned );
        }

        size_t getCursorPositionInAdjacentLine( const size_t currentPos, const bool moveUp )
        {
            return _text.getCursorPositionInAdjacentLine( currentPos, _textInputArea.width, moveUp );
        }

        int32_t height( const std::string & text ) const
        {
            TextInput tmp{ _text };
            tmp.set( text, 0 );
            return tmp.height( _textInputArea.width );
        }

    private:
        Image & _output;
        TextInput _text;
        MovableSprite _cursor;
        ImageRestorer _background;
        Rect _textInputArea;
        bool _isSingleLineTextCenterAligned{ false };
    };

    // Renderer of current time and FPS on screen
    class SystemInfoRenderer
    {
    public:
        SystemInfoRenderer();

        SystemInfoRenderer( const SystemInfoRenderer & ) = delete;

        ~SystemInfoRenderer() = default;

        SystemInfoRenderer & operator=( const SystemInfoRenderer & ) = delete;

        void preRender();

        void postRender()
        {
            _text.hide();
        }

    private:
        std::chrono::time_point<std::chrono::steady_clock> _startTime;
        engine4heroes::MovableText _text;
        std::deque<double> _delays;
    };

    class TimedEventValidator final : public ActionObject
    {
    public:
        explicit TimedEventValidator( std::function<bool()> verification, const uint64_t delayBeforeFirstUpdateMs = 500, const uint64_t delayBetweenUpdateMs = 100 )
            : _verification( std::move( verification ) )
            , _delayBetweenUpdateMs( delayBetweenUpdateMs )
            , _delayBeforeFirstUpdateMs( delayBeforeFirstUpdateMs )
        {
            // Do nothing.
        }

        TimedEventValidator( const TimedEventValidator & ) = delete;

        ~TimedEventValidator() override = default;

        TimedEventValidator & operator=( const TimedEventValidator & ) = delete;

        bool isDelayPassed()
        {
            if ( _delayBeforeFirstUpdateMs.isPassed() && _delayBetweenUpdateMs.isPassed() && _verification() ) {
                _delayBetweenUpdateMs.reset();
                return true;
            }
            return false;
        }

    private:
        void senderUpdate( const ActionObject * sender ) override;

        std::function<bool()> _verification;
        engine4heroes::TimeDelay _delayBetweenUpdateMs;
        engine4heroes::TimeDelay _delayBeforeFirstUpdateMs;
    };

    // Updates `valueBuf` based on keyboard input relevant to modifying an integer. Returns a non-empty `std::optional` instance containing the entered value
    // if this value has been both changed and is within the range [`min`, `max`], otherwise returns an empty instance. See the implementation for details.
    std::optional<int32_t> processIntegerValueTyping( const int32_t min, const int32_t max, std::string & valueBuf );

    std::vector<LocalizedString> getLocalizedStrings( std::string text, const SupportedLanguage currentLanguage, const std::string_view toReplace,
                                                      std::string_view replacement, const SupportedLanguage replacementLanguage );

    std::unique_ptr<TextBase> getLocalizedText( std::vector<LocalizedString> texts, const FontType font );

    std::unique_ptr<TextBase> getLocalizedText( std::vector<std::pair<LocalizedString, FontType>> texts );
}
