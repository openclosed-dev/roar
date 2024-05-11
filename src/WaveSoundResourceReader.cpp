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

SoundResourceReader* WaveResourceReader::open(const wchar_t* path) {
    HANDLE file = ::CreateFileW(
        path,
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr
    );

    if (file == INVALID_HANDLE_VALUE) {
        return nullptr;
    }

    return new WaveResourceReader(file);
}

WaveResourceReader::WaveResourceReader(HANDLE file)
:   file(file) {
}

WaveResourceReader::~WaveResourceReader() {
    ::CloseHandle(file);
    file = nullptr;
}

SoundResource* WaveResourceReader::read() {

    RiffChunk chunk{};

    if (!readRiffHeader(&chunk)) {
        return false;
    }

    return readRiffBody(chunk.chunkSize - 4);
}

bool WaveResourceReader::readRiffHeader(RiffChunk* chunk) {

    DWORD bytesRead = 0;

    if (!::ReadFile(file, chunk, sizeof(RiffChunk), &bytesRead, nullptr) || bytesRead < sizeof(RiffChunk)) {
        return false;
    }

    if (!chunk->hasType('R', 'I', 'F', 'F')) {
        return false;
    }

    return fourcc(chunk->fileType, 'W', 'A', 'V', 'E');
}

SoundResource* WaveResourceReader::readRiffBody(DWORD bodySize) {

    WAVEFORMATEXTENSIBLE format{};
    BYTE* data = nullptr;
    DWORD dataSize = 0;

    int chunksProcessed = 0;

    Chunk chunk{};

    DWORD offset = 0;
    DWORD bytesRead = 0;

    while (offset < bodySize) {

        if (!::ReadFile(file, &chunk, sizeof(chunk), &bytesRead, nullptr) || bytesRead < sizeof(chunk)) {
            return false;
        }

        offset += bytesRead;
        DWORD paddedSize = ((chunk.chunkSize + 1) / 2) * 2;

        if (chunk.hasType('f', 'm', 't', ' ')) {
            if (!::ReadFile(file, &format, chunk.chunkSize, &bytesRead, nullptr) || bytesRead < chunk.chunkSize) {
                break;
            }
            chunksProcessed++;
        } else if (chunk.hasType('d', 'a', 't', 'a')) {
            data = new BYTE[chunk.chunkSize];
            dataSize = chunk.chunkSize;
            if (!::ReadFile(file, data, dataSize, &bytesRead, nullptr) || bytesRead < dataSize) {
                break;
            }
            chunksProcessed++;
        } else {
            if (::SetFilePointer(file, paddedSize, nullptr, FILE_CURRENT) == INVALID_SET_FILE_POINTER) {
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

