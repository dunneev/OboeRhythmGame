/*
 * Copyright 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <utils/logging.h>
#include <thread>
#include <cinttypes>

#include "Game.h"

Game::Game(AAssetManager &assetManager): mAssetManager(assetManager) {
    // Add your code here

}

/*
 * Before the game can be played there's a couple of things that must happen:
 *
 * The audio stream must be opened using openStream.
 * Any MP3 files used by the game need to be decoded and loaded into memory using setupAudioSources.
 *
 * These operations are blocking, and depending on the size of the MP3 files and the speed of the
 * decoder they might take several seconds to complete. We should avoid performing these operations
 * on the main thread otherwise we might get a dreaded ANR(Application Not Responding).
 *
 * Another thing which must happen before the game can be played is starting the audio stream.
 * It makes sense to do this after the other loading operations have completed.
 *
 * load() will be called on a separate thread.
 */
void Game::load() {

    if (!openStream()) {
        mGameState = GameState::FailedToLoad;
        return;
    }

    if (!setupAudioSources()) {
        mGameState = GameState::FailedToLoad;
        return;
    }

    Result result = mAudioStream->requestStart();
    if (result != Result::OK){
        LOGE("Failed to start stream. Error: %s", convertToText(result));
        mGameState = GameState::FailedToLoad;
        return;
    }

    mGameState = GameState::Playing;
}

void Game::start() {

    // Call load() asynchronously.
    mLoadingResult = std::async(&Game::load, this);
}

void Game::stop(){
    // Add your code here
}

void Game::tap(int64_t eventTimeAsUptime) {
    mClap->setPlaying(true);
}

void Game::tick(){
    switch (mGameState){
        case GameState::Playing:
            SetGLScreenColor(kPlayingColor);
            break;
        case GameState::Loading:
            SetGLScreenColor(kLoadingColor);
            break;
        case GameState::FailedToLoad:
            SetGLScreenColor(kLoadingFailedColor);
            break;
    }
}

void Game::onSurfaceCreated() {
}

void Game::onSurfaceChanged(int widthInPixels, int heightInPixels) {
}

void Game::onSurfaceDestroyed() {
}

/**
 * Get the result of a tap
 *
 * @param tapTimeInMillis - The time the tap occurred in milliseconds
 * @param tapWindowInMillis - The time at the middle of the "tap window" in milliseconds
 * @return TapResult can be Early, Late or Success
 */
TapResult Game::getTapResult(int64_t tapTimeInMillis, int64_t tapWindowInMillis){
    LOGD("Tap time %" PRId64 ", tap window time: %" PRId64, tapTimeInMillis, tapWindowInMillis);
    if (tapTimeInMillis <= tapWindowInMillis + kWindowCenterOffsetMs) {
        if (tapTimeInMillis >= tapWindowInMillis - kWindowCenterOffsetMs) {
            return TapResult::Success;
        } else {
            return TapResult::Early;
        }
    } else {
        return TapResult::Late;
    }
}

/*
 * Decode CLAP.mp3 into PCM data and store it in the Player object
 */
bool Game::setupAudioSources() {

    // Create a data source and player for the clap sound
    std::shared_ptr<AAssetDataSource> mClapSource {
            AAssetDataSource::newFromCompressedAsset(mAssetManager, "CLAP.mp3")
    };
    if (mClapSource == nullptr){
        LOGE("Could not load source data for clap sound");
        return false;
    }
    mClap = std::make_unique<Player>(mClapSource);

    // Create a data source and player for our backing track
    std::shared_ptr<AAssetDataSource> backingTrackSource {
            AAssetDataSource::newFromCompressedAsset(mAssetManager, "FUNKY_HOUSE.mp3")
    };
    if (backingTrackSource == nullptr){
        LOGE("Could not load source data for backing track");
        return false;
    }
    mBackingTrack = std::make_unique<Player>(backingTrackSource);
    mBackingTrack->setPlaying(true);
    mBackingTrack->setLooping(true);

    // Add both players to a mixer
    mMixer.addTrack(mClap.get());
    mMixer.addTrack(mBackingTrack.get());
    mMixer.setChannelCount(mAudioStream->getChannelCount());
    return true;
}

/*
 * Setup an AudioStream to communicate with an audio device.
 */
bool Game::openStream() {
    AudioStreamBuilder builder;
    builder.setFormat(AudioFormat::Float);
    builder.setPerformanceMode(PerformanceMode::LowLatency);
    builder.setSharingMode(SharingMode::Exclusive);
    builder.setSampleRate(48000);

    // Fallback to med. quality resampling algorithm if the device does not support the sample rate specified above.
    builder.setSampleRateConversionQuality(SampleRateConversionQuality::Medium);

    builder.setChannelCount(2);

    // Tell the builder where to find the callback object before the stream is opened.
    builder.setCallback(this);

    Result result = builder.openManagedStream(mAudioStream);
    if (result != Result::OK){
        LOGE("Failed to open stream. Error: %s", convertToText(result));
        return false;
    }
    return true;
}

/*
 * Callback method to get audio data from memory into audio stream.
 *
 * Note: Whenever we receive a container array of type void * we must remember
 * to cast it to the current stream data format (in our case float) and supply
 * data only in that format. Failure to do so will result in some awful noise!
 */
DataCallbackResult Game::onAudioReady(AudioStream *oboeStream, void *audioData, int32_t numFrames) {

    // Render the mixer audio data into the audioData array
    mMixer.renderAudio(static_cast<float*>(audioData), numFrames);

    // Tell the stream we intend to keep sending audio data. Callbacks should continue.
    return DataCallbackResult::Continue;
}
