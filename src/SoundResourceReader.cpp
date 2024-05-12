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
#include "SoundResourceReader.h"
#include "OggSoundResourceReader.h"
#include "WaveSoundResourceReader.h"

SoundResourceReader* SoundResourceReader::fromFile(const wchar_t* path) {
    return fromFile(std::filesystem::path(path));
}

SoundResourceReader* SoundResourceReader::fromFile(const std::filesystem::path& path) {
    std::string e = path.extension().string();
    std::transform(e.begin(), e.end(), e.begin(), ::tolower);

    FILE* file = ::fopen(path.string().c_str(), "rb");
    if (file != nullptr) {
        if (e == ".ogg") {
            return new OggSoundResourceReader(file);
        } else if (e == ".wav") {
            return new WaveSoundResourceReader(file);
        }
        ::fclose(file);
    }

    return nullptr;
}
