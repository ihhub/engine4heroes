/***************************************************************************
 *   engine4heroes: https://github.com/ihhub/engine4heroes                 *
 *   Copyright (C) 2026                                                    *
 *                                                                         *
 *   fheroes2: https://github.com/ihhub/fheroes2                           *
 *   Copyright (C) 2021 - 2025                                             *
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

#include "ui_language.h"

#include <algorithm>
#include <cassert>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <utility>

#include "configuration.h"
#include "tools.h"
#include "translations.h"

namespace
{
    // Strings in this map must in lower case and non translatable.
    const std::map<std::string, engine4heroes::SupportedLanguage, std::less<>> languageName
        = { { "pl", engine4heroes::SupportedLanguage::Polish },     { "polish", engine4heroes::SupportedLanguage::Polish },
            { "de", engine4heroes::SupportedLanguage::German },     { "german", engine4heroes::SupportedLanguage::German },
            { "fr", engine4heroes::SupportedLanguage::French },     { "french", engine4heroes::SupportedLanguage::French },
            { "ru", engine4heroes::SupportedLanguage::Russian },    { "russian", engine4heroes::SupportedLanguage::Russian },
            { "it", engine4heroes::SupportedLanguage::Italian },    { "italian", engine4heroes::SupportedLanguage::Italian },
            { "cs", engine4heroes::SupportedLanguage::Czech },      { "czech", engine4heroes::SupportedLanguage::Czech },
            { "nb", engine4heroes::SupportedLanguage::Norwegian },  { "norwegian", engine4heroes::SupportedLanguage::Norwegian },
            { "be", engine4heroes::SupportedLanguage::Belarusian }, { "belarusian", engine4heroes::SupportedLanguage::Belarusian },
            { "uk", engine4heroes::SupportedLanguage::Ukrainian },  { "ukrainian", engine4heroes::SupportedLanguage::Ukrainian },
            { "bg", engine4heroes::SupportedLanguage::Bulgarian },  { "bulgarian", engine4heroes::SupportedLanguage::Bulgarian },
            { "es", engine4heroes::SupportedLanguage::Spanish },    { "spanish", engine4heroes::SupportedLanguage::Spanish },
            { "pt", engine4heroes::SupportedLanguage::Portuguese }, { "portuguese", engine4heroes::SupportedLanguage::Portuguese },
            { "sv", engine4heroes::SupportedLanguage::Swedish },    { "swedish", engine4heroes::SupportedLanguage::Swedish },
            { "tr", engine4heroes::SupportedLanguage::Turkish },    { "turkish", engine4heroes::SupportedLanguage::Turkish },
            { "ro", engine4heroes::SupportedLanguage::Romanian },   { "romanian", engine4heroes::SupportedLanguage::Romanian },
            { "nl", engine4heroes::SupportedLanguage::Dutch },      { "dutch", engine4heroes::SupportedLanguage::Dutch },
            { "hu", engine4heroes::SupportedLanguage::Hungarian },  { "hungarian", engine4heroes::SupportedLanguage::Hungarian },
            { "da", engine4heroes::SupportedLanguage::Danish },     { "danish", engine4heroes::SupportedLanguage::Danish },
            { "sk", engine4heroes::SupportedLanguage::Slovak },     { "slovak", engine4heroes::SupportedLanguage::Slovak },
            { "vi", engine4heroes::SupportedLanguage::Vietnamese }, { "vietnamese", engine4heroes::SupportedLanguage::Vietnamese },
            { "el", engine4heroes::SupportedLanguage::Greek },      { "greek", engine4heroes::SupportedLanguage::Greek } };
}

namespace engine4heroes
{
    LanguageSwitcher::LanguageSwitcher( const SupportedLanguage language )
        : _currentLanguage( Configuration::instance().getGameLanguage() )
    {
        Configuration::instance().setGameLanguage( getLanguageAbbreviation( language ) );
    }

    LanguageSwitcher::~LanguageSwitcher()
    {
        Configuration::instance().setGameLanguage( _currentLanguage );
    }

    SupportedLanguage getResourceLanguage()
    {
        static std::optional<SupportedLanguage> language;
        if ( language.has_value() ) {
            return *language;
        }

        // TODO: load font resources and check their CRC32 value.
        language = SupportedLanguage::English;
        return *language;
    }

    std::vector<SupportedLanguage> getSupportedLanguages()
    {
        // We need to group languages by code pages to avoid recreating font related resources while switching languages.
        std::map<CodePage, std::vector<SupportedLanguage>> supportedLanguges;

        const SupportedLanguage resourceLanguage = getResourceLanguage();
        if ( resourceLanguage != SupportedLanguage::English ) {
            supportedLanguges[getCodePage( resourceLanguage )].emplace_back( resourceLanguage );
        }

        const std::set<SupportedLanguage> possibleLanguages{ SupportedLanguage::French,     SupportedLanguage::Polish,     SupportedLanguage::German,
                                                             SupportedLanguage::Russian,    SupportedLanguage::Italian,    SupportedLanguage::Norwegian,
                                                             SupportedLanguage::Belarusian, SupportedLanguage::Bulgarian,  SupportedLanguage::Ukrainian,
                                                             SupportedLanguage::Romanian,   SupportedLanguage::Spanish,    SupportedLanguage::Portuguese,
                                                             SupportedLanguage::Swedish,    SupportedLanguage::Turkish,    SupportedLanguage::Dutch,
                                                             SupportedLanguage::Hungarian,  SupportedLanguage::Czech,      SupportedLanguage::Danish,
                                                             SupportedLanguage::Slovak,     SupportedLanguage::Vietnamese, SupportedLanguage::Greek };

        for ( const SupportedLanguage language : possibleLanguages ) {
            if ( language != resourceLanguage /*&& isAlphabetSupported( language )*/ ) {
                supportedLanguges[getCodePage( language )].emplace_back( language );
            }
        }

        auto & conf = Configuration::instance();

        const engine4heroes::SupportedLanguage currentLanguage = engine4heroes::getLanguageFromAbbreviation( conf.getGameLanguage() );

        std::vector<engine4heroes::SupportedLanguage> validSupportedLanguages{ engine4heroes::SupportedLanguage::English };

        for ( const auto & [codePage, languages] : supportedLanguges ) {
            for ( const auto language : languages ) {
                // TODO: we shouldn't load all language resources just for the sake of verifying whether their translations exist.
                //       Find another way to avoid this heavy operation.
                if ( conf.setGameLanguage( engine4heroes::getLanguageAbbreviation( language ) ) ) {
                    validSupportedLanguages.emplace_back( language );
                }
            }
        }

        conf.setGameLanguage( engine4heroes::getLanguageAbbreviation( currentLanguage ) );

        assert( !validSupportedLanguages.empty() );

        return validSupportedLanguages;
    }

    const char * getLanguageName( const SupportedLanguage language )
    {
        switch ( language ) {
        case SupportedLanguage::English:
            return _( "English" );
        case SupportedLanguage::French:
            return _( "French" );
        case SupportedLanguage::Polish:
            return _( "Polish" );
        case SupportedLanguage::German:
            return _( "German" );
        case SupportedLanguage::Russian:
            return _( "Russian" );
        case SupportedLanguage::Italian:
            return _( "Italian" );
        case SupportedLanguage::Czech:
            return _( "Czech" );
        case SupportedLanguage::Norwegian:
            return _( "Norwegian" );
        case SupportedLanguage::Belarusian:
            return _( "Belarusian" );
        case SupportedLanguage::Bulgarian:
            return _( "Bulgarian" );
        case SupportedLanguage::Ukrainian:
            return _( "Ukrainian" );
        case SupportedLanguage::Romanian:
            return _( "Romanian" );
        case SupportedLanguage::Spanish:
            return _( "Spanish" );
        case SupportedLanguage::Swedish:
            return _( "Swedish" );
        case SupportedLanguage::Portuguese:
            return _( "Portuguese" );
        case SupportedLanguage::Turkish:
            return _( "Turkish" );
        case SupportedLanguage::Dutch:
            return _( "Dutch" );
        case SupportedLanguage::Hungarian:
            return _( "Hungarian" );
        case SupportedLanguage::Danish:
            return _( "Danish" );
        case SupportedLanguage::Slovak:
            return _( "Slovak" );
        case SupportedLanguage::Vietnamese:
            return _( "Vietnamese" );
        case SupportedLanguage::Greek:
            return _( "Greek" );
        default:
            // Did you add a new language? Please add the code to handle it.
            assert( 0 );
            return nullptr;
        }
    }

    const char * getLanguageAbbreviation( const SupportedLanguage language )
    {
        switch ( language ) {
        case SupportedLanguage::English:
            return ""; // English is a special case. It always returns an empty string as it's a default language.
        case SupportedLanguage::French:
            return "fr";
        case SupportedLanguage::Polish:
            return "pl";
        case SupportedLanguage::German:
            return "de";
        case SupportedLanguage::Russian:
            return "ru";
        case SupportedLanguage::Italian:
            return "it";
        case SupportedLanguage::Czech:
            return "cs";
        case SupportedLanguage::Norwegian:
            return "nb";
        case SupportedLanguage::Belarusian:
            return "be";
        case SupportedLanguage::Bulgarian:
            return "bg";
        case SupportedLanguage::Ukrainian:
            return "uk";
        case SupportedLanguage::Romanian:
            return "ro";
        case SupportedLanguage::Spanish:
            return "es";
        case SupportedLanguage::Swedish:
            return "sv";
        case SupportedLanguage::Portuguese:
            return "pt";
        case SupportedLanguage::Turkish:
            return "tr";
        case SupportedLanguage::Dutch:
            return "nl";
        case SupportedLanguage::Hungarian:
            return "hu";
        case SupportedLanguage::Danish:
            return "da";
        case SupportedLanguage::Slovak:
            return "sk";
        case SupportedLanguage::Vietnamese:
            return "vi";
        case SupportedLanguage::Greek:
            return "el";
        default:
            // Did you add a new language? Please add the code to handle it.
            assert( 0 );
            return nullptr;
        }
    }

    SupportedLanguage getLanguageFromAbbreviation( const std::string & abbreviation )
    {
        if ( abbreviation.empty() ) {
            return SupportedLanguage::English;
        }

        const std::string name( StringLower( abbreviation ) );

        auto iter = languageName.find( name );
        if ( iter == languageName.end() ) {
            // Unsupported language. Fallback to English.
            return SupportedLanguage::English;
        }

        return iter->second;
    }

    SupportedLanguage getCurrentLanguage()
    {
        return engine4heroes::getLanguageFromAbbreviation( Configuration::instance().getGameLanguage() );
    }

    CodePage getCodePage( const SupportedLanguage language )
    {
        switch ( language ) {
        case SupportedLanguage::English:
            return CodePage::ASCII;
        case SupportedLanguage::Czech:
        case SupportedLanguage::Hungarian:
        case SupportedLanguage::Polish:
        case SupportedLanguage::Slovak:
            return CodePage::CP1250;
        case SupportedLanguage::Belarusian:
        case SupportedLanguage::Bulgarian:
        case SupportedLanguage::Russian:
        case SupportedLanguage::Ukrainian:
            return CodePage::CP1251;
        case SupportedLanguage::Danish:
        case SupportedLanguage::Dutch:
        case SupportedLanguage::French:
        case SupportedLanguage::German:
        case SupportedLanguage::Italian:
        case SupportedLanguage::Norwegian:
        case SupportedLanguage::Portuguese:
        case SupportedLanguage::Spanish:
        case SupportedLanguage::Swedish:
            return CodePage::CP1252;
        case SupportedLanguage::Greek:
            return CodePage::CP1253;
        case SupportedLanguage::Turkish:
            return CodePage::CP1254;
        case SupportedLanguage::Vietnamese:
            return CodePage::CP1258;
        case SupportedLanguage::Romanian:
            return CodePage::ISO8859_16;
        default:
            // Add new language handling code!
            assert( 0 );
            break;
        }

        return CodePage::ASCII;
    }
}
