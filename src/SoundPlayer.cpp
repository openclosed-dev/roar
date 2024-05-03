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
#include "SoundPlayer.h"
#include "SoundPack.h"

class ReusableVoice : public IXAudio2VoiceCallback {
private:

    SoundPlayer* player;
    IXAudio2SourceVoice* source;

public:

    ReusableVoice(SoundPlayer* player);
    ~ReusableVoice();

    void setSourceVoice(IXAudio2SourceVoice* source) {
        this->source = source;
    }

    IXAudio2SourceVoice* getSourceVoice() {
        return source;
    }

    virtual void OnStreamEnd();

    // Callbacks to ignore.
    virtual void OnVoiceProcessingPassEnd() {}
    virtual void OnVoiceProcessingPassStart(UINT32 SamplesRequired) {}
    virtual void OnBufferEnd(void * pBufferContext) {}
    virtual void OnBufferStart(void * pBufferContext) {}
    virtual void OnLoopEnd(void * pBufferContext) {}
    virtual void OnVoiceError(void * pBufferContext, HRESULT Error) {}
};

ReusableVoice::ReusableVoice(SoundPlayer* player)
:   player(player), source(nullptr) {
}

ReusableVoice::~ReusableVoice() {
    if (source != nullptr) {
        source->DestroyVoice();
        source = nullptr;
    }
}

void ReusableVoice::OnStreamEnd() {
    player->finishVoice(this);
}

SoundPlayer* SoundPlayer::create() {
    IXAudio2* audio = nullptr;
    HRESULT hr = XAudio2Create(&audio, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(hr)) {
        return nullptr;
    }

    IXAudio2MasteringVoice* masterVoice = nullptr;
    hr = audio->CreateMasteringVoice(&masterVoice);
    if (FAILED(hr)) {
        return nullptr;
    }

    return new SoundPlayer(audio, masterVoice);
}

SoundPlayer::SoundPlayer(IXAudio2* audio, IXAudio2MasteringVoice* masterVoice)
:   audio(audio),
    masterVoice(masterVoice),
    soundPack(nullptr) {

    voices.reserve(MAX_VOICES);
}

SoundPlayer::~SoundPlayer() {

    destroyAllVoices();

    clearSoundPack();

    if (masterVoice != nullptr) {
        masterVoice->DestroyVoice();
        masterVoice = nullptr;
    }

    if (audio != nullptr) {
        audio->Release();
        audio = nullptr;
    }
}

void SoundPlayer::setSoundPack(SoundPack* soundPack) {
    if (this->soundPack != soundPack) {
        clearSoundPack();
        this->soundPack = soundPack;
    }
}

void SoundPlayer::clearSoundPack() {
    if (this->soundPack != nullptr) {
        delete this->soundPack;
        this->soundPack = nullptr;
    }
}

bool SoundPlayer::playSound(int scanCode) {

    if (soundPack == nullptr) {
        return false;
    }

    SoundClip* clip = soundPack->getClip(scanCode);
    if (clip == nullptr) {
        return false;
    }

    ReusableVoice* voice = findIdleVoice();
    if (voice == nullptr) {
        voice = createVoice();
        if (voice == nullptr) {
            return false;
        }
    }

    XAUDIO2_BUFFER buffer{};
    buffer.AudioBytes = clip->length;
    buffer.pAudioData = clip->data;
    buffer.Flags = XAUDIO2_END_OF_STREAM;

    IXAudio2SourceVoice* source = voice->getSourceVoice();
    HRESULT hr = source->SubmitSourceBuffer(&buffer);
    if (SUCCEEDED(hr)) {
        hr = source->Start(0);
        if (SUCCEEDED(hr)) {
            return true;
        }
    }

    return false;
}

ReusableVoice* SoundPlayer::createVoice() {

    if (voices.size() >= MAX_VOICES) {
        return nullptr;
    }

    ReusableVoice* reusable = new ReusableVoice(this);

    IXAudio2SourceVoice* voice = nullptr;
    HRESULT hr = audio->CreateSourceVoice(
        &voice, soundPack->getFormat(), 0, XAUDIO2_DEFAULT_FREQ_RATIO, reusable);
    if (FAILED(hr)) {
        delete reusable;
        return nullptr;
    }

    reusable->setSourceVoice(voice);
    voices.push_back(reusable);

    return reusable;
}

void SoundPlayer::destroyAllVoices() {

    while (!idle.empty()) {
        idle.pop();
    }

    auto it =  voices.begin();
    while (it != voices.end()) {
        delete *it++;
    }

    voices.clear();
}

ReusableVoice* SoundPlayer::findIdleVoice() {
    std::lock_guard lock(mutex);
    if (idle.empty()) {
        return nullptr;
    } else {
        auto first = idle.front();
        idle.pop();
        return first;
    }
}

void SoundPlayer::finishVoice(ReusableVoice* voice) {
    std::lock_guard lock(mutex);
    idle.push(voice);
}
