cmake_minimum_required(VERSION 3.28)
project(roar VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED  true)

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

set(SOURCES
    src/Application.cpp
    src/SoundPack.cpp
    src/SoundPackRepository.cpp
    src/SoundPlayer.cpp
    src/WaveResource.cpp
    src/Window.cpp
)

add_definitions(-D_UNICODE -DUNICODE)

add_executable(roar WIN32
    ${SOURCES}
    src/resource.rc
    roar.exe.manifest
)

target_precompile_headers(roar PRIVATE
    <windows.h>
    <shellapi.h>
    <commctrl.h>
    <shlobj.h>
    <iostream>
    <vector>
    <queue>
    <set>
    <unordered_map>
    <filesystem>
    <mutex>
    <fstream>
    <xaudio2.h>
    <nlohmann/json.hpp>
)

target_link_libraries(roar PRIVATE
    comctl32.lib
    xaudio2.lib
    nlohmann_json::nlohmann_json
)
