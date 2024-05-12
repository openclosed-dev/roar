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
#include "SoundResource.h"

SoundResource::SoundResource(
    int numberOfChannels,
    int samplingRate,
    int bitsPerSample,
    const std::uint8_t* data,
    std::uint64_t length)
:   numberOfChannels(numberOfChannels),
    samplingRate(samplingRate),
    bitsPerSample(bitsPerSample),
    data(data),
    length(length)
{
}

SoundResource::~SoundResource()
{
    if (data != nullptr) {
        delete [] data;
        data = nullptr;
    }
}

SoundClip* SoundResource::slice(int start, int duration)
{
    const size_t samplePerSec = getSamplingRate();
    const size_t bytesPerSample = getBitsPerSample() * getNumberOfChannels() / 8L;
    size_t clipOffset = (samplePerSec * start / 1000) * bytesPerSample;
    size_t clipLength = (samplePerSec * duration / 1000) * bytesPerSample;

    if (clipOffset >= length) {
        return nullptr;
    }

    if (clipOffset + clipLength > length) {
        clipLength = length - clipOffset;
    }

    return new SoundClip{data + clipOffset, clipLength};
}
