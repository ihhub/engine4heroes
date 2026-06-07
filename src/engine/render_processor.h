/***************************************************************************
 *   engine4heroes: https://github.com/ihhub/engine4heroes                 *
 *   Copyright (C) 2026                                                    *
 *                                                                         *
 *   fheroes2: https://github.com/ihhub/fheroes2                           *
 *   Copyright (C) 2023 - 2025                                             *
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

#include <cstdint>
#include <functional>
#include <vector>

#include "timing.h"

namespace engine4heroes
{
    class RenderProcessor
    {
    public:
        RenderProcessor( const RenderProcessor & ) = delete;

        ~RenderProcessor() = default;

        RenderProcessor & operator=( const RenderProcessor & ) = delete;

        static RenderProcessor & instance();

        void registerRenderers( const std::function<void()> & preRenderer, const std::function<void()> & postRenderer )
        {
            _preRenderer = preRenderer;
            _postRenderer = postRenderer;
        }

        void unregisterRenderers()
        {
            _preRenderer = {};
            _postRenderer = {};
        }

        void enableRenderers()
        {
            _enableRenderers = true;
        }

        void disableRenderers()
        {
            _enableRenderers = false;
        }

        bool preRenderAction();

        void postRenderAction() const;

    private:
        RenderProcessor() = default;

        std::function<void()> _preRenderer;
        std::function<void()> _postRenderer;

        bool _enableRenderers{ false };
    };
}
