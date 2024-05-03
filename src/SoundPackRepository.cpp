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
#include "SoundPackRepository.h"
#include "WaveResource.h"
#include "SoundPack.h"

namespace fs = std::filesystem;
using json = nlohmann::json;

class SoundPackLoader {
public:

    SoundPack* load(const fs::path& dir);

private:

    std::string getSound(json& config);

    WaveResource* loadWaveResource(const fs::path& path);

    SoundClipMap* buildKeyMap(json& config, WaveResource* resource);

    SoundClipMap* modifyKeyMap(SoundClipMap& map);
};

SoundPack* SoundPackLoader::load(const fs::path& dir) {

    fs::path configPath = dir / L"config.json";

    std::ifstream stream(configPath);
    json config = json::parse(stream);

    auto resource = loadWaveResource(dir / getSound(config));
    auto map = buildKeyMap(config, resource);

    return new SoundPack(resource, map);
}

std::string SoundPackLoader::getSound(json& config) {

    if (config.contains("sound")) {
        auto property = config.at("sound");
        if (property.is_string()) {
            std::string sound;
            property.get_to(sound);
            return sound;
        }
    }

    return "sound.wav";
}

WaveResource* SoundPackLoader::loadWaveResource(const fs::path& path) {

    WaveResource* resource = nullptr;

    WaveResourceReader* reader = WaveResourceReader::fromFile(path.c_str());
    if (reader != nullptr) {
        resource = reader->read();
        delete reader;
    }

    return resource;
}

SoundClipMap* SoundPackLoader::buildKeyMap(json& config, WaveResource* resource) {

    auto map = new SoundClipMap();

    if (!config.contains("keys")) {
        return map;
    }

    auto keys = config.at("keys");
    if (!keys.is_object()) {
        return map;
    }

    for (auto& [key, value] : keys.items()) {
        try {
            int scanCode = std::stoi(key);
            if (value.is_array() && value.size() == 2) {
                int start = value.at(0);
                int duration = value.at(1);
                map->insert(std::make_pair(scanCode, resource->slice(start, duration)));
            }
        } catch (const std::exception& e) {
        }
    }

    return modifyKeyMap(*map);
}

SoundClipMap* SoundPackLoader::modifyKeyMap(SoundClipMap& map) {
    map[0xe01d] = map[3613]; // right ctrl
    map[0xe037] = map[3639]; // print screen
    map[0xe038] = map[3640]; // right alt
    map[0xe047] = map[3655]; // home
    map[0xe049] = map[3657]; // page up
    map[0xe04f] = map[3663]; // end
    map[0xe051] = map[3665]; // page down
    map[0xe052] = map[3666]; // insert
    map[0xe053] = map[3667]; // delete
    map[0xe05b] = map[3675]; // left win key
    map[0xe05c] = map[3676]; // right win key
    map[0xe11d] = map[3653]; // pause
    return &map;
}

SoundPackRepository::SoundPackRepository(
    const std::set<std::filesystem::path>& dirs)
:   dirs(dirs) {
}

SoundPackRepository:: ~SoundPackRepository() {
}

SoundPack* SoundPackRepository::loadDefault() {
    return load(L"cherrymx-black-abs");
}

SoundPack* SoundPackRepository::load(const wchar_t* name) {

    for (const auto& dir : dirs) {
        std::filesystem::path path = dir / "sound" / name;
        if (std::filesystem::exists(path / "config.json")) {
            SoundPackLoader loader;
            return loader.load(path);
        }
    }

    return nullptr;
}
