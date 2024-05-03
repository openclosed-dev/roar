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
#include "SoundPack.h"
#include "WaveResource.h"

SoundPack::SoundPack(WaveResource* resource, SoundClipMap* map)
:   resource(resource),
    map(map) {
}

SoundPack::~SoundPack() {
    if (map != nullptr) {
        std::set<SoundClip*> distinct;
        for (const auto& pair : *map) {
            distinct.insert(pair.second);
        }
        for (const auto* clip : distinct) {
            delete clip;
        }
        delete map;
        map = nullptr;
    }

    if (resource != nullptr) {
        delete resource;
        resource = nullptr;
    }
}

const WAVEFORMATEX* SoundPack::getFormat() {
    return (WAVEFORMATEX*) &resource->getFormat();
}

SoundClip* SoundPack::getClip(int scanCode) {
    return (*map)[scanCode];
}
