/*
 * Copyright 2024 the original author or authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

class SoundPack;
class SoundClip;
class ReusableVoice;

class SoundPlayer {
private:

    static const int MAX_VOICES = 8;

    IXAudio2* audio;
    IXAudio2MasteringVoice* masterVoice;

    SoundPack* soundPack;

    // All created voices.
    std::vector<ReusableVoice*> voices;
    // Finished voices.
    std::queue<ReusableVoice*> idle;

    std::mutex mutex;

public:

    static SoundPlayer* create();

    ~SoundPlayer();

    void setSoundPack(SoundPack* soundPack);

    void clearSoundPack();

    bool playSound(int scanCode);

private:

    SoundPlayer(IXAudio2* audio, IXAudio2MasteringVoice* masterVoice);

    ReusableVoice* createVoice();

    ReusableVoice* findIdleVoice();

    void finishVoice(ReusableVoice* voice);

    void destroyAllVoices();

    friend class ReusableVoice;
};
