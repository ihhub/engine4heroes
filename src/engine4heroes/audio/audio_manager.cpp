/***************************************************************************
 *   engine4heroes: https://github.com/ihhub/engine4heroes                 *
 *   Copyright (C) 2026                                                    *
 *                                                                         *
 *   fheroes2: https://github.com/ihhub/fheroes2                           *
 *   Copyright (C) 2022 - 2026                                             *
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

#include "audio_manager.h"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdlib>
#include <deque>
#include <mutex>
#include <optional>
#include <ostream>
#include <string>
#include <utility>

#include "audio.h"
#include "configuration.h"
#include "logging.h"
#include "music_info.h"
#include "resource_manager.h"
#include "sound_info.h"
#include "thread.h"

namespace
{
    std::map<int32_t, std::vector<uint8_t>> soundCache;
    std::map<int32_t, std::vector<uint8_t>> musicCache;

    struct ChannelAudioLoopEffectInfo : public AudioManager::AudioLoopEffectInfo
    {
        ChannelAudioLoopEffectInfo( const AudioLoopEffectInfo & info, const int chan )
            : AudioLoopEffectInfo( info )
            , channelId( chan )
        {
            // Do nothing.
        }

        bool operator==( const AudioManager::AudioLoopEffectInfo & other ) const
        {
            return AudioManager::AudioLoopEffectInfo::operator==( other );
        }

        int channelId{ -1 };
    };

    std::map<int, std::vector<uint8_t>> wavDataCache;

    const std::vector<uint8_t> & getSound( const int soundType )
    {
        std::vector<uint8_t> & sound = soundCache[soundType];
        if ( sound.empty() ) {
            sound = GameResource::getAudioStream( Sound::getSoundString( soundType ) );
        }

        return sound;
    }

    const std::vector<uint8_t> & getMusic( const int trackId )
    {
        std::vector<uint8_t> & music = musicCache[trackId];
        if ( music.empty() ) {
            music = GameResource::getAudioStream( Music::getMusicTrackString( trackId ) );
        }

        return music;
    }

    // Returns the ID of the channel occupied by the sound being played, or a negative value (-1) in case of failure.
    int PlaySoundImpl( const int soundType );
    void PlayMusicImpl( const int trackId, const Music::PlaybackMode playbackMode );
    void playLoopSoundsImpl( std::map<int, std::vector<AudioManager::AudioLoopEffectInfo>> soundEffects, const bool is3DAudioEnabled );

    // SDL MIDI player is a single threaded library which requires a lot of time to start playing some long midi compositions.
    // This leads to a situation of a short application freeze while a hero crosses terrains or ending a battle.
    // The only way to avoid this is to fire MIDI requests asynchronously and synchronize them if needed.
    class AsyncSoundManager final : public MultiThreading::AsyncManager
    {
    public:
        void pushMusic( const int musicId, const Music::PlaybackMode playbackMode )
        {
            createWorker();

            const std::scoped_lock<std::mutex> lock( _mutex );

            _musicTask.emplace( musicId, playbackMode );

            notifyWorker();
        }

        void pushSound( const int soundType )
        {
            createWorker();

            const std::scoped_lock<std::mutex> lock( _mutex );

            _soundTasks.emplace_back( soundType );

            notifyWorker();
        }

        void pushLoopSound( std::map<int, std::vector<AudioManager::AudioLoopEffectInfo>> effects, const bool is3DAudioEnabled )
        {
            createWorker();

            const std::scoped_lock<std::mutex> lock( _mutex );

            _loopSoundTask.emplace( std::move( effects ), is3DAudioEnabled );

            notifyWorker();
        }

        void removeMusicTask()
        {
            const std::scoped_lock<std::mutex> lock( _mutex );

            _musicTask.reset();

            if ( _taskToExecute == TaskType::PlayMusic ) {
                _taskToExecute = TaskType::None;
            }
        }

        void removeSoundTasks()
        {
            const std::scoped_lock<std::mutex> lock( _mutex );

            _soundTasks.clear();

            if ( _taskToExecute == TaskType::PlaySound ) {
                _taskToExecute = TaskType::None;
            }
        }

        void removeAllSoundTasks()
        {
            const std::scoped_lock<std::mutex> lock( _mutex );

            _soundTasks.clear();
            _loopSoundTask.reset();

            switch ( _taskToExecute ) {
            case TaskType::PlaySound:
            case TaskType::PlayLoopSound:
                _taskToExecute = TaskType::None;
                break;
            default:
                break;
            }
        }

        void removeAllTasks()
        {
            const std::scoped_lock<std::mutex> lock( _mutex );

            _musicTask.reset();
            _soundTasks.clear();
            _loopSoundTask.reset();

            _taskToExecute = TaskType::None;
        }

        // This mutex protects operations with AudioManager's resources, such as AGG files, data caches, etc
        std::recursive_mutex & resourceMutex()
        {
            return _resourceMutex;
        }

    private:
        enum class TaskType : int
        {
            None,
            PlayMusic,
            PlaySound,
            PlayLoopSound
        };

        struct MusicTask
        {
            MusicTask() = default;

            MusicTask( const int id, const Music::PlaybackMode mode )
                : musicId( id )
                , playbackMode( mode )
            {
                // Do nothing.
            }

            int musicId{ 0 };
            Music::PlaybackMode playbackMode{ Music::PlaybackMode::PLAY_ONCE };
        };

        struct SoundTask
        {
            SoundTask() = default;

            explicit SoundTask( const int type )
                : soundType( type )
            {
                // Do nothing.
            }

            int soundType{ Sound::UNKNOWN };
        };

        struct LoopSoundTask
        {
            LoopSoundTask() = default;

            LoopSoundTask( std::map<int, std::vector<AudioManager::AudioLoopEffectInfo>> effects, const bool is3DAudioOn )
                : soundEffects( std::move( effects ) )
                , is3DAudioEnabled( is3DAudioOn )
            {
                // Do nothing.
            }

            std::map<int, std::vector<AudioManager::AudioLoopEffectInfo>> soundEffects;
            bool is3DAudioEnabled{ false };
        };

        std::optional<MusicTask> _musicTask;
        std::deque<SoundTask> _soundTasks;
        std::optional<LoopSoundTask> _loopSoundTask;

        MusicTask _currentMusicTask;
        SoundTask _currentSoundTask;
        LoopSoundTask _currentLoopSoundTask;

        std::atomic<TaskType> _taskToExecute{ TaskType::None };

        std::recursive_mutex _resourceMutex;

        // This method is called by the worker thread and is protected by _mutex
        bool prepareTask() override
        {
            if ( _musicTask ) {
                std::swap( _currentMusicTask, *_musicTask );
                _musicTask.reset();

                _taskToExecute = TaskType::PlayMusic;

                return true;
            }

            if ( !_soundTasks.empty() ) {
                std::swap( _currentSoundTask, _soundTasks.front() );
                _soundTasks.pop_front();

                _taskToExecute = TaskType::PlaySound;

                return true;
            }

            if ( _loopSoundTask ) {
                std::swap( _currentLoopSoundTask, *_loopSoundTask );
                _loopSoundTask.reset();

                _taskToExecute = TaskType::PlayLoopSound;

                return true;
            }

            _taskToExecute = TaskType::None;

            return false;
        }

        // This method is called by the worker thread, but is not protected by _mutex
        void executeTask() override
        {
            // Do not allow the main thread to acquire this mutex in the interval between the
            // _taskToExecute was checked and the task was started executing. Release it only
            // when the task is fully completed.
            const std::scoped_lock<std::recursive_mutex> lock( _resourceMutex );

            switch ( _taskToExecute ) {
            case TaskType::None:
                // Nothing to do.
                return;
            case TaskType::PlayMusic:
                PlayMusicImpl( _currentMusicTask.musicId, _currentMusicTask.playbackMode );
                return;
            case TaskType::PlaySound:
                PlaySoundImpl( _currentSoundTask.soundType );
                return;
            case TaskType::PlayLoopSound:
                playLoopSoundsImpl( std::move( _currentLoopSoundTask.soundEffects ), _currentLoopSoundTask.is3DAudioEnabled );
                return;
            default:
                // How is it even possible? Did you add a new task?
                assert( 0 );
                break;
            }
        }
    };

    std::map<int, std::vector<ChannelAudioLoopEffectInfo>> currentAudioLoopEffects;
    bool is3DAudioLoopEffectsEnabled{ false };

    // The music track last requested to be played
    int lastRequestedMusicTrackId{ Music::UNKNOWN };
    // The music track that is currently being played
    int currentMusicTrackId{ Music::UNKNOWN };

    // engine4heroes::AGGFile g_midiHeroes2AGG;
    // engine4heroes::AGGFile g_midiHeroes2xAGG;

    AsyncSoundManager g_asyncSoundManager;

    int PlaySoundImpl( const int soundType )
    {
        const std::scoped_lock<std::recursive_mutex> lock( g_asyncSoundManager.resourceMutex() );

        DEBUG_LOG( DBG_GAME, DBG_TRACE, "Try to play sound " << Sound::getSoundString( soundType ) )

        const std::vector<uint8_t> & v = getSound( soundType );
        if ( v.empty() ) {
            return -1;
        }

        return Mixer::Play( v.data(), static_cast<uint32_t>( v.size() ), false );
    }

    void PlayMusicImpl( const int trackId, const Music::PlaybackMode playbackMode )
    {
        // Make sure that the music track is valid.
        assert( trackId > Music::UNKNOWN && trackId < Music::COUNT );

        const std::scoped_lock<std::recursive_mutex> lock( g_asyncSoundManager.resourceMutex() );

        if ( currentMusicTrackId == trackId && Music::isPlaying() ) {
            return;
        }

        // Check if the music track is already available in the music database.
        if ( Music::Play( trackId, playbackMode ) ) {
            DEBUG_LOG( DBG_GAME, DBG_TRACE, "Play music track " << trackId )

            currentMusicTrackId = trackId;

            return;
        }

        Music::Play( trackId, getMusic( trackId ), playbackMode );

        currentMusicTrackId = trackId;
    }

    std::pair<size_t, size_t> findPairOfClosestSoundEffects( const std::vector<AudioManager::AudioLoopEffectInfo> & effectsToAdd,
                                                             const std::vector<ChannelAudioLoopEffectInfo> & effectsToReplace )
    {
        assert( !effectsToAdd.empty() && !effectsToReplace.empty() );

        std::pair<size_t, size_t> result{ 0, 0 };

        int bestAngleDiff = INT_MAX;
        int bestDistanceDiff = INT_MAX;

        for ( size_t effectToAddId = 0; effectToAddId < effectsToAdd.size(); ++effectToAddId ) {
            const AudioManager::AudioLoopEffectInfo & effectToAdd = effectsToAdd[effectToAddId];

            for ( size_t effectToReplaceId = 0; effectToReplaceId < effectsToReplace.size(); ++effectToReplaceId ) {
                const ChannelAudioLoopEffectInfo & effectToReplace = effectsToReplace[effectToReplaceId];

                const int angleDiff = std::abs( effectToAdd.angle - effectToReplace.angle );
                const int distanceDiff = std::abs( static_cast<int>( effectToAdd.distance ) - static_cast<int>( effectToReplace.distance ) );

                if ( bestAngleDiff < angleDiff ) {
                    continue;
                }

                if ( bestAngleDiff == angleDiff && bestDistanceDiff < distanceDiff ) {
                    continue;
                }

                bestAngleDiff = angleDiff;
                bestDistanceDiff = distanceDiff;

                result = { effectToAddId, effectToReplaceId };
            }
        }

        return result;
    }

    void clearAllAudioLoopEffects()
    {
        for ( const auto & audioEffectPair : currentAudioLoopEffects ) {
            const std::vector<ChannelAudioLoopEffectInfo> & existingEffects = audioEffectPair.second;

            for ( const ChannelAudioLoopEffectInfo & info : existingEffects ) {
                if ( Mixer::isPlaying( info.channelId ) ) {
                    Mixer::Stop( info.channelId );
                }
            }
        }

        currentAudioLoopEffects.clear();
    }

    void playLoopSoundsImpl( std::map<int, std::vector<AudioManager::AudioLoopEffectInfo>> soundEffects, const bool is3DAudioEnabled )
    {
        const std::scoped_lock<std::recursive_mutex> lock( g_asyncSoundManager.resourceMutex() );

        if ( is3DAudioLoopEffectsEnabled != is3DAudioEnabled ) {
            is3DAudioLoopEffectsEnabled = is3DAudioEnabled;
            clearAllAudioLoopEffects();
        }

        std::map<int, std::vector<ChannelAudioLoopEffectInfo>> tempAudioLoopEffects;
        std::swap( tempAudioLoopEffects, currentAudioLoopEffects );

        // Remove all sounds which aren't currently played anymore. This may be the case if the sound playback was forcibly stopped.
        for ( auto iter = tempAudioLoopEffects.begin(); iter != tempAudioLoopEffects.end(); ) {
            auto & [dummy, effects] = *iter;

            effects.erase( std::remove_if( effects.begin(), effects.end(),
                                           []( const ChannelAudioLoopEffectInfo & effectInfo ) { return !Mixer::isPlaying( effectInfo.channelId ); } ),
                           effects.end() );

            if ( effects.empty() ) {
                iter = tempAudioLoopEffects.erase( iter );
            }
            else {
                ++iter;
            }
        }

        // Try to reuse existing sounds.
        for ( auto iter = soundEffects.begin(); iter != soundEffects.end(); ) {
            auto & [soundType, effectsToAdd] = *iter;
            assert( !effectsToAdd.empty() );

            auto soundTypeIter = tempAudioLoopEffects.find( soundType );
            if ( soundTypeIter == tempAudioLoopEffects.end() ) {
                // There is no such sound, nothing to reuse.
                ++iter;
                continue;
            }

            std::vector<ChannelAudioLoopEffectInfo> & effectsToReplace = soundTypeIter->second;

            // Find existing sounds that have exactly the same distance and angle as the ones that should be added, and reuse these sounds as is.
            for ( auto effectToAddIter = effectsToAdd.begin(); effectToAddIter != effectsToAdd.end(); ) {
                auto effectToReplaceIter = std::find( effectsToReplace.begin(), effectsToReplace.end(), *effectToAddIter );
                if ( effectToReplaceIter == effectsToReplace.end() ) {
                    ++effectToAddIter;
                    continue;
                }

                currentAudioLoopEffects[soundType].emplace_back( *effectToReplaceIter );

                effectsToReplace.erase( effectToReplaceIter );
                effectToAddIter = effectsToAdd.erase( effectToAddIter );
            }

            // Find the existing sounds closest to those that should be added and reuse these sounds by adjusting their positions.
            {
                size_t effectsToReplaceCount = std::min( effectsToAdd.size(), effectsToReplace.size() );

                while ( effectsToReplaceCount > 0 ) {
                    --effectsToReplaceCount;

                    {
                        const auto [effectToAddId, effectToReplaceId] = findPairOfClosestSoundEffects( effectsToAdd, effectsToReplace );
                        const int channelId = effectsToReplace[effectToReplaceId].channelId;

                        currentAudioLoopEffects[soundType].emplace_back( effectsToAdd[effectToAddId], channelId );

                        effectsToReplace.erase( effectsToReplace.begin() + static_cast<ptrdiff_t>( effectToReplaceId ) );
                        effectsToAdd.erase( effectsToAdd.begin() + static_cast<ptrdiff_t>( effectToAddId ) );
                    }

                    const ChannelAudioLoopEffectInfo & effectInfo = currentAudioLoopEffects[soundType].back();

                    assert( is3DAudioEnabled || effectInfo.angle == 0 );

                    Mixer::setPosition( effectInfo.channelId, effectInfo.angle, effectInfo.distance );
                }
            }

            if ( effectsToReplace.empty() ) {
                tempAudioLoopEffects.erase( soundTypeIter );
            }

            if ( effectsToAdd.empty() ) {
                iter = soundEffects.erase( iter );
            }
            else {
                ++iter;
            }
        }

        // Channels with sounds that could not be reused because such sounds are missing from the list of added ones should be stopped.
        {
            for ( const auto & [dummy, effects] : tempAudioLoopEffects ) {
                for ( const ChannelAudioLoopEffectInfo & effectInfo : effects ) {
                    Mixer::Stop( effectInfo.channelId );
                }
            }

            tempAudioLoopEffects.clear();
        }

        // Add new sound effects.
        for ( const auto & [soundType, effects] : soundEffects ) {
            assert( !effects.empty() );

            for ( const AudioManager::AudioLoopEffectInfo & effectInfo : effects ) {
                // This is a new sound effect. Register and play it.
                const std::vector<uint8_t> & audioData = getSound( soundType );
                if ( audioData.empty() ) {
                    // Looks like nothing to play. Ignore it.
                    continue;
                }

                assert( is3DAudioEnabled || effectInfo.angle == 0 );

                const int channelId
                    = Mixer::Play( audioData.data(), static_cast<uint32_t>( audioData.size() ), true, std::pair{ effectInfo.angle, effectInfo.distance } );
                if ( channelId < 0 ) {
                    // Unable to play this sound.
                    continue;
                }

                currentAudioLoopEffects[soundType].emplace_back( effectInfo, channelId );

                DEBUG_LOG( DBG_GAME, DBG_TRACE, "Playing sound " << Sound::getSoundString( soundType ) )
            }
        }
    }
}

namespace AudioManager
{
    AudioInitializer::AudioInitializer()
    {
        if ( Audio::isValid() ) {
            Mixer::SetChannels( 32 );

            Mixer::setVolume( 100 * Configuration::instance().getSoundVolume() / 10 );

            Music::setVolume( 100 * Configuration::instance().getMusicVolume() / 10 );
            Music::SetFadeInMs( 900 );
        }
    }

    AudioInitializer::~AudioInitializer()
    {
        g_asyncSoundManager.removeAllTasks();
        g_asyncSoundManager.stopWorker();

        wavDataCache.clear();
        currentAudioLoopEffects.clear();
    }

    MusicRestorer::MusicRestorer()
        : _music( lastRequestedMusicTrackId )
    {
        // Do nothing.
    }

    MusicRestorer::~MusicRestorer()
    {
        // It is assumed that the previous track was looped and should be resumed
        PlayMusicAsync( _music, Music::PlaybackMode::RESUME_AND_PLAY_INFINITE );
    }

    void playLoopSoundsAsync( std::map<int, std::vector<AudioLoopEffectInfo>> soundEffects )
    {
        if ( !Audio::isValid() ) {
            return;
        }

        g_asyncSoundManager.pushLoopSound( std::move( soundEffects ), Configuration::instance().is3DAudioEnabled() );
    }

    int PlaySound( const int soundType )
    {
        if ( soundType == Sound::UNKNOWN ) {
            return -1;
        }

        if ( !Audio::isValid() ) {
            return -1;
        }

        g_asyncSoundManager.removeSoundTasks();

        return PlaySoundImpl( soundType );
    }

    void PlaySoundAsync( const int soundType )
    {
        if ( soundType == Sound::UNKNOWN ) {
            return;
        }

        if ( !Audio::isValid() ) {
            return;
        }

        g_asyncSoundManager.pushSound( soundType );
    }

    void PlayMusic( const int trackId, const Music::PlaybackMode playbackMode )
    {
        if ( !Audio::isValid() ) {
            return;
        }

        lastRequestedMusicTrackId = trackId;

        if ( trackId == Music::UNKNOWN ) {
            return;
        }

        g_asyncSoundManager.removeMusicTask();

        PlayMusicImpl( trackId, playbackMode );
    }

    void PlayMusicAsync( const int trackId, const Music::PlaybackMode playbackMode )
    {
        if ( !Audio::isValid() ) {
            return;
        }

        lastRequestedMusicTrackId = trackId;

        if ( trackId == Music::UNKNOWN ) {
            return;
        }

        g_asyncSoundManager.pushMusic( trackId, playbackMode );
    }

    void PlayCurrentMusic()
    {
        if ( !Audio::isValid() ) {
            return;
        }

        g_asyncSoundManager.removeMusicTask();

        const std::scoped_lock<std::recursive_mutex> lock( g_asyncSoundManager.resourceMutex() );

        if ( currentMusicTrackId == Music::UNKNOWN ) {
            return;
        }

        const int trackId = std::exchange( currentMusicTrackId, Music::UNKNOWN );

        PlayMusicImpl( trackId, Music::PlaybackMode::RESUME_AND_PLAY_INFINITE );
    }

    void stopSounds()
    {
        if ( !Audio::isValid() ) {
            return;
        }

        g_asyncSoundManager.removeAllSoundTasks();

        const std::scoped_lock<std::recursive_mutex> lock( g_asyncSoundManager.resourceMutex() );

        clearAllAudioLoopEffects();

        Mixer::Stop();
    }

    void ResetAudio()
    {
        if ( !Audio::isValid() ) {
            return;
        }

        g_asyncSoundManager.removeAllTasks();

        const std::scoped_lock<std::recursive_mutex> lock( g_asyncSoundManager.resourceMutex() );

        clearAllAudioLoopEffects();

        Music::Stop();
        Mixer::Stop();

        lastRequestedMusicTrackId = Music::UNKNOWN;
        currentMusicTrackId = Music::UNKNOWN;
    }
}
