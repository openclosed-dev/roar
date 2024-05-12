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
#include "WaveSoundResourceReader.h"
#include "SoundResource.h"

static bool fourcc(const BYTE* t, char c1, char c2, char c3, char c4) {
    return t[0] == c1 && t[1] == c2 && t[2] == c3 && t[3] == c4;
}

struct Chunk {
    BYTE chunkType[4];
    DWORD chunkSize;

    boolean hasType(char c1, char c2, char c3, char c4) {
        return fourcc(chunkType, c1, c2, c3, c4);
    }
};

struct RiffChunk : Chunk {
    BYTE fileType[4];
};

WaveSoundResourceReader::WaveSoundResourceReader(FILE* file)
:   file(file)
{
}

WaveSoundResourceReader::~WaveSoundResourceReader()
{
    ::fclose(file);
}

SoundResource* WaveSoundResourceReader::read()
{
    RiffChunk chunk{};

    if (!readRiffHeader(&chunk)) {
        return false;
    }

    return readRiffBody(chunk.chunkSize - 4);
}

bool WaveSoundResourceReader::readRiffHeader(RiffChunk* chunk)
{
    if (::fread(chunk, sizeof(RiffChunk), 1, file) != 1) {
        return false;
    }

    if (!chunk->hasType('R', 'I', 'F', 'F')) {
        return false;
    }

    return fourcc(chunk->fileType, 'W', 'A', 'V', 'E');
}

SoundResource* WaveSoundResourceReader::readRiffBody(long bodySize)
{
    WAVEFORMATEXTENSIBLE format{};
    std::uint8_t* data = nullptr;
    std::uint32_t dataSize = 0;

    int chunksProcessed = 0;

    Chunk chunk{};

    long offset = 0;

    while (offset < bodySize) {

        if (::fread(&chunk, sizeof(chunk), 1, file) != 1) {
            return false;
        }

        offset += sizeof(chunk);
        long paddedSize = ((chunk.chunkSize + 1) / 2) * 2;

        if (chunk.hasType('f', 'm', 't', ' ')) {
            if (::fread(&format, chunk.chunkSize, 1, file) != 1) {
                break;
            }
            chunksProcessed++;
        } else if (chunk.hasType('d', 'a', 't', 'a')) {
            data = new std::uint8_t[chunk.chunkSize];
            dataSize = chunk.chunkSize;
            if (::fread(data, dataSize, 1, file) != 1) {
                break;
            }
            chunksProcessed++;
        } else {
            if (::fseek(file, paddedSize, SEEK_CUR) != 0) {
                break;
            }
        }

        offset += paddedSize;

        if (chunksProcessed >= 2) {
            return new SoundResource(
                format.Format.nChannels,
                format.Format.nSamplesPerSec,
                format.Format.wBitsPerSample,
                data,
                dataSize);
        }
    }

    if (data != nullptr) {
        delete [] data;
    }

    return nullptr;
}

