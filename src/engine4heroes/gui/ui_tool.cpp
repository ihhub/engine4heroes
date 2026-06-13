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

#include "ui_tool.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <ctime>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "localevent.h"
#include "screen.h"
#include "system.h"
#include "tools.h"
#include "translations.h"
#include "ui_text.h"

namespace engine4heroes
{
    MovableSprite::MovableSprite()
        : _restorer( Display::instance(), 0, 0, 0, 0 )
    {
        // Do nothing.
    }

    MovableSprite::MovableSprite( const int32_t width, const int32_t height, const int32_t x, const int32_t y )
        : Sprite( width, height, x, y )
        , _restorer( Display::instance(), x, y, width, height )
        , _isHidden( height == 0 && width == 0 )
    {
        // Do nothing.
    }

    MovableSprite::MovableSprite( const Sprite & sprite )
        : Sprite( sprite )
        , _restorer( Display::instance(), 0, 0, 0, 0 )
        , _isHidden( false )
    {
        // Do nothing.
    }

    MovableSprite::~MovableSprite()
    {
        if ( _isHidden ) {
            _restorer.reset();
        }
    }

    MovableSprite & MovableSprite::operator=( const Sprite & sprite )
    {
        if ( this == &sprite ) {
            return *this;
        }

        Sprite::operator=( sprite );

        _restorer.update( x(), y(), width(), height() );

        return *this;
    }

    void MovableSprite::setPosition( const int32_t x, const int32_t y )
    {
        if ( _isHidden ) {
            Sprite::setPosition( x, y );
            return;
        }

        hide();
        Sprite::setPosition( x, y );
        show();
    }

    void MovableSprite::show()
    {
        if ( _isHidden ) {
            _restorer.update( x(), y(), width(), height() );
            Blit( *this, Display::instance(), x(), y() );
            _isHidden = false;
        }
    }

    void MovableText::drawInRoi( const int32_t x, const int32_t y, const Rect & roi )
    {
        hide();

        assert( _text != nullptr );

        Rect textArea = _text->area();
        textArea.x += x;
        textArea.y += y;

        // Not to cut off the top of diacritic signs in capital letters we shift the text down.
        const int32_t extraShiftY = textArea.y < roi.y ? roi.y - textArea.y : 0;
        textArea.height += extraShiftY;

        const Rect overlappedRoi = textArea ^ roi;

        _restorer.update( overlappedRoi.x, overlappedRoi.y, overlappedRoi.width, overlappedRoi.height );
        _text->drawInRoi( x, y + extraShiftY, _output, overlappedRoi );

        _isHidden = false;
    }

    TextInputField::TextInputField( const Rect & textArea, const bool isMultiLine, const bool isCenterAligned, Image & output,
                                    const std::optional<SupportedLanguage> language )
        : _output( output )
        , _text( FontType::normalWhite(), textArea.width, isMultiLine, language )
        , _cursor( getCursorSprite( FontType::normalWhite() ) )
        // We enlarge background to have space for cursor at text edges and space for diacritics.
        , _background( output, textArea.x - 1, textArea.y - 2, textArea.width + 2, textArea.height + 2 )
        , _textInputArea( textArea )
        , _isSingleLineTextCenterAligned( !isMultiLine && isCenterAligned )
    {
        // Do nothing.
    }

    bool TextInputField::eventProcessing()
    {
        // if ( !Game::validateAnimationDelay( Game::DelayType::CURSOR_BLINK_DELAY ) ) {
        //     return false;
        // }

        if ( _cursor.isHidden() ) {
            _cursor.show();
        }
        else {
            _cursor.hide();
        }

        return true;
    }

    void TextInputField::draw( const std::string & newText, const int32_t cursorPositionInText )
    {
        _cursor.hide();
        _background.restore();

        _text.set( newText, cursorPositionInText );

        // Multi-line text is currently always automatically center-aligned.
        const int32_t offsetX = _isSingleLineTextCenterAligned ? _textInputArea.x + ( _textInputArea.width - _text.width() ) / 2 : _textInputArea.x;
        const int32_t offsetY = _textInputArea.y + 2;

        _text.drawInRoi( offsetX, offsetY, _output, _background.rect() );

        _cursor.setPosition( _text.cursorArea().x + offsetX, _text.cursorArea().y + offsetY );
        _cursor.show();

        // Game::AnimateResetDelay( Game::DelayType::CURSOR_BLINK_DELAY );
    }

    SystemInfoRenderer::SystemInfoRenderer()
        : _startTime( std::chrono::steady_clock::now() )
        , _text( engine4heroes::Display::instance() )
    {}

    void SystemInfoRenderer::preRender()
    {
        const int32_t offsetX = 26;
        engine4heroes::Display & display = engine4heroes::Display::instance();
        const int32_t offsetY = display.height() - 30;

        const tm tmi = System::GetTM( std::time( nullptr ) );

        std::array<char, 9> mbstr{ 0 };
        std::strftime( mbstr.data(), mbstr.size(), "%H:%M:%S", &tmi );

        std::string info( mbstr.data() );

        const std::chrono::time_point<std::chrono::steady_clock> endTime = std::chrono::steady_clock::now();
        const std::chrono::duration<double> time = endTime - _startTime;
        _startTime = endTime;

        const double totalTime = time.count();

        _delays.push_front( totalTime );

        double allTime = 0;
        for ( size_t i = 0; i < _delays.size(); ++i ) {
            allTime += _delays[i];
            if ( allTime > 1.0 ) {
                // Remove all delays that exceed to one second time period.
                _delays.resize( i + 1 );
            }
        }

        const double averageFps = static_cast<double>( _delays.size() ) / allTime + 0.05;
        const int32_t integerFps = static_cast<int32_t>( averageFps );

        info += _( ", FPS: " );
        info += std::to_string( integerFps );
        if ( integerFps < 10 ) {
            info += '.';
            info += std::to_string( static_cast<int32_t>( ( averageFps - integerFps ) * 10 ) );
        }

        auto text = std::make_unique<engine4heroes::Text>( std::move( info ), engine4heroes::FontType::normalWhite() );

        engine4heroes::Rect fpsRoi( text->area() );
        fpsRoi.x += offsetX;
        fpsRoi.y += offsetY;

        _text.update( std::move( text ) );
        _text.draw( offsetX, offsetY );

        display.updateNextRenderRoi( fpsRoi );
    }

    void TimedEventValidator::senderUpdate( const ActionObject * sender )
    {
        if ( sender == nullptr ) {
            return;
        }
        _delayBeforeFirstUpdateMs.reset();
        _delayBetweenUpdateMs.reset();
    }

    std::optional<int32_t> processIntegerValueTyping( const int32_t min, const int32_t max, std::string & valueBuf )
    {
        assert( min <= max );

        const LocalEvent & le = LocalEvent::Get();

        if ( !le.isAnyKeyPressed() ) {
            return {};
        }

        const int32_t zeroBufValue = std::clamp<int32_t>( 0, min, max );

        if ( le.isKeyPressed( engine4heroes::Key::KEY_BACKSPACE ) || le.isKeyPressed( engine4heroes::Key::KEY_DELETE ) ) {
            valueBuf.clear();

            return zeroBufValue;
        }

        if ( le.isKeyPressed( engine4heroes::Key::KEY_MINUS ) || le.isKeyPressed( engine4heroes::Key::KEY_KP_MINUS ) ) {
            if ( min >= 0 ) {
                return {};
            }

            if ( !std::all_of( valueBuf.begin(), valueBuf.end(), []( const char ch ) { return ( ch == '0' ); } ) ) {
                return {};
            }

            valueBuf = "-";

            return zeroBufValue;
        }

        if ( const std::optional<char> newDigit = [&le]() -> std::optional<char> {
                 using KeyUnderlyingType = std::underlying_type_t<engine4heroes::Key>;

                 const engine4heroes::Key keyValue = le.getPressedKeyValue();

                 if ( keyValue >= engine4heroes::Key::KEY_0 && keyValue <= engine4heroes::Key::KEY_9 ) {
                     return engine4heroes::checkedCast<char>( static_cast<KeyUnderlyingType>( keyValue ) - static_cast<KeyUnderlyingType>( engine4heroes::Key::KEY_0 )
                                                              + '0' );
                 }

                 if ( keyValue >= engine4heroes::Key::KEY_KP_0 && keyValue <= engine4heroes::Key::KEY_KP_9 ) {
                     return engine4heroes::checkedCast<char>( static_cast<KeyUnderlyingType>( keyValue ) - static_cast<KeyUnderlyingType>( engine4heroes::Key::KEY_KP_0 )
                                                              + '0' );
                 }

                 return {};
             }();
             newDigit ) {
            valueBuf.push_back( *newDigit );

            const std::optional<int32_t> value = [&valueBuf = std::as_const( valueBuf )]() -> std::optional<int32_t> {
                try {
                    return std::stoi( valueBuf );
                }
                catch ( std::out_of_range & ) {
                    return {};
                }
            }();

            if ( !value || ( min <= 0 && value < min ) || ( max >= 0 && value > max ) ) {
                valueBuf.pop_back();

                return {};
            }

            if ( value == std::clamp( *value, min, max ) ) {
                return value;
            }

            return {};
        }

        return {};
    }

    std::vector<LocalizedString> getLocalizedStrings( std::string text, const SupportedLanguage currentLanguage, const std::string_view toReplace,
                                                      std::string_view replacement, const SupportedLanguage replacementLanguage )
    {
        if ( currentLanguage == replacementLanguage ) {
            StringReplace( text, toReplace.data(), replacement );
            return { { std::move( text ), currentLanguage } };
        }

        // Check whether the replacement text even exists.
        const std::string::size_type pos = text.find( toReplace );
        if ( pos == std::string::npos ) {
            return { { std::move( text ), currentLanguage } };
        }

        std::vector<LocalizedString> strings;

        strings.emplace_back( text.substr( 0, pos ), currentLanguage );
        strings.emplace_back( std::string( replacement ), replacementLanguage );
        strings.emplace_back( text.substr( pos + toReplace.size() ), currentLanguage );

        return strings;
    }

    std::unique_ptr<TextBase> getLocalizedText( std::vector<LocalizedString> texts, const FontType font )
    {
        if ( texts.empty() ) {
            return {};
        }

        if ( texts.size() == 1 ) {
            return std::make_unique<Text>( std::move( texts.front().text ), font, texts.front().language );
        }

        auto multiFontText = std::make_unique<MultiFontText>();
        for ( auto & text : texts ) {
            multiFontText->add( Text( std::move( text.text ), font, text.language ) );
        }

        return multiFontText;
    }

    std::unique_ptr<TextBase> getLocalizedText( std::vector<std::pair<LocalizedString, FontType>> texts )
    {
        if ( texts.empty() ) {
            return {};
        }

        if ( texts.size() == 1 ) {
            auto & [text, font] = texts.front();
            return std::make_unique<Text>( std::move( text.text ), font, text.language );
        }

        auto multiFontText = std::make_unique<MultiFontText>();
        for ( auto & [text, font] : texts ) {
            multiFontText->add( Text( std::move( text.text ), font, text.language ) );
        }

        return multiFontText;
    }
}
