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
#include "OggSoundResourceReader.h"
#include "SoundResource.h"
#include <vorbis/vorbisfile.h>

OggSoundResourceReader::OggSoundResourceReader(FILE* file)
:   file(file)
{
}

OggSoundResourceReader::~OggSoundResourceReader()
{
    ::fclose(file);
}

SoundResource* OggSoundResourceReader::read()
{
    OggVorbis_File vf{};

    if (::ov_open_callbacks(file, &vf, nullptr, 0, OV_CALLBACKS_NOCLOSE) < 0) {
        std::cerr << "Not an Ogg bitstream" << std::endl;
        return nullptr;
    }

    vorbis_info* info = ::ov_info(&vf, -1);

    ogg_int64_t numberOfSamples = ::ov_pcm_total(&vf, -1);
    size_t totalBytes = numberOfSamples * 2 * info->channels;

    std::uint8_t* decoded = new std::uint8_t[totalBytes];

    size_t offset = 0;
    int currentSection = 0;
    for (;;) {
        long bytesRead = ::ov_read(
            &vf,
            (char*) decoded + offset,
            4096,
            0, // little endian
            2, // bytes per sample
            1, // signed
            &currentSection);

        if (bytesRead == 0) {
            break;
        } else if (bytesRead > 0) {
            offset += bytesRead;
        } else {
            delete [] decoded;
            return nullptr;
        }
    }

    return new SoundResource(
        info->channels,
        info->rate,
        16,
        decoded,
        totalBytes
    );
}
