cmake_minimum_required(VERSION 3.28)
project(roar VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED  true)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "$ENV{APPDATA}/${PROJECT_NAME}" CACHE PATH "..." FORCE)
endif()

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

configure_file(src/version.h.in version.h)

set(sources
    src/Application.cpp
    src/SoundPack.cpp
    src/SoundPackRepository.cpp
    src/SoundPlayer.cpp
    src/SoundResource.cpp
    src/SoundResourceReader.cpp
    src/WaveSoundResourceReader.cpp
    src/Window.cpp
)

add_definitions(-D_UNICODE -DUNICODE)

add_executable(roar WIN32
    ${sources}
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

target_include_directories(roar PRIVATE
    "${PROJECT_BINARY_DIR}"
)

target_link_libraries(roar PRIVATE
    comctl32.lib
    xaudio2.lib
    nlohmann_json::nlohmann_json
)

install(TARGETS roar DESTINATION bin)
install(FILES README.md LICENSE DESTINATION .)
install(DIRECTORY sound DESTINATION .)
install(DIRECTORY legal DESTINATION .)

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_GENERATOR "ZIP")
include(CPack)
