/***************************************************************************
 *   engine4heroes: https://github.com/ihhub/engine4heroes                 *
 *   Copyright (C) 2026                                                    *
 *                                                                         *
 *   fheroes2: https://github.com/ihhub/fheroes2                           *
 *   Copyright (C) 2021 - 2022                                             *
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

#include "math_base.h"

namespace engine4heroes
{
    // Action-event class to communicate between sender and receiver (only for user actions)
    class ActionObject
    {
    public:
        ActionObject() = default;
        virtual ~ActionObject() = default;

        void subscribe( ActionObject * receiver )
        {
            _receiver = receiver;
        }

        void unsubscribe()
        {
            _receiver = nullptr;
        }

    protected:
        void notifySubscriber()
        {
            if ( _receiver != nullptr ) {
                _receiver->senderUpdate( this );
            }
        }

        void preRenderNotification( const Rect & area ) const
        {
            if ( _receiver != nullptr ) {
                _receiver->preRenderUpdate( this, area );
            }
        }

        void postRenderNotification( const Rect & area ) const
        {
            if ( _receiver != nullptr ) {
                _receiver->postRenderUpdate( this, area );
            }
        }

        virtual void senderUpdate( const ActionObject * /*object*/ )
        {
            // Do nothing.
        }

        virtual void preRenderUpdate( const ActionObject * /*object*/, const Rect & /* renderArea*/ )
        {
            // Do nothing.
        }

        virtual void postRenderUpdate( const ActionObject * /*object*/, const Rect & /* renderArea*/ )
        {
            // Do nothing.
        }

    private:
        ActionObject * _receiver{ nullptr };
    };
}
