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
#include "Application.h"
#include "Window.h"
#include "SoundPlayer.h"

Application::Application(HINSTANCE module)
:   module(module),
    repository(getDirectories(module)) {

    Window::registerClass(module);
}

int Application::run(const wchar_t* commandLine, int show) {

    HRESULT hr = ::CoInitializeEx( nullptr, COINIT_MULTITHREADED );
    if (FAILED(hr))
        return 1;

    SoundPack* soundPack = repository.loadDefault();

    SoundPlayer* soundPlayer = createSoundPlayer(soundPack);
    if (soundPlayer == nullptr) {
        return 1;
    }

    Window* window = Window::create(L"Hello Window", soundPlayer, module);
    window->show(SW_HIDE);

    loop();

    delete window;
    delete soundPlayer;

    ::CoUninitialize();

    return 0;
}

SoundPlayer* Application::createSoundPlayer(SoundPack* soundPack) {

    SoundPlayer* soundPlayer = SoundPlayer::create();
    if (soundPlayer != nullptr) {
        soundPlayer->setSoundPack(soundPack);
    }

    return soundPlayer;
}

void Application::loop() {
    MSG msg{};
    while (::GetMessage(&msg, NULL, 0, 0)) {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }
}

std::set<std::filesystem::path> Application::getDirectories(HINSTANCE module) {
    std::set<std::filesystem::path> dirs;
    dirs.insert(std::filesystem::current_path());
    dirs.insert(getApplicationDirectory(module));
    return dirs;
}

std::filesystem::path Application::getApplicationDirectory(HINSTANCE module) {
    wchar_t modulePath[MAX_PATH + 1]{};
    ::GetModuleFileName(module, modulePath, MAX_PATH + 1);

    std::filesystem::path path(modulePath);
    return path.parent_path();
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, wchar_t* commandLine, int show) {
    Application app(hInstance);
    return app.run(commandLine, show);
}
