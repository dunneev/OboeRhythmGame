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

void Game::load() {
    // Add your code here
}

void Game::start() {
    // Add your code here
}

void Game::stop(){
    // Add your code here
}

void Game::tap(int64_t eventTimeAsUptime) {
    // Add your code here
}

void Game::tick(){
    SetGLScreenColor(kLoadingColor);
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
    return true;
}
