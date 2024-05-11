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

#include "SoundClip.h"

class SoundResource {
private:

    const int numberOfChannels;
    const int samplingRate;
    const int bitsPerSample;
    const std::uint8_t* data;
    const std::uint64_t length;

public:

    SoundResource(
        int numberOfChannels,
        int samplingRate,
        int bitPerSample,
        const std::uint8_t* data,
        std::uint64_t length);

    virtual ~SoundResource();

    int getNumberOfChannels() {
        return numberOfChannels;
    }

    int getSamplingRate() {
        return samplingRate;
    }

    int getBitsPerSample() {
        return bitsPerSample;
    }

    int getBytesPerSec() {
        return getNumberOfChannels() * getSamplingRate() * getBitsPerSample() / 8;
    }

    int getBlockAlign() {
        return getNumberOfChannels() * getBitsPerSample() / 8;
    }

    const std::uint8_t* getData() {
        return data;
    }

    uint64_t getLength() {
        return length;
    }

    SoundClip* slice(int start, int duration);
};
