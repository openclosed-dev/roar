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
#pragma once

class SoundPlayer;

class Window {
private:

    HINSTANCE module;
    HWND handle;

    HICON notificationIcon;

    SoundPlayer* soundPlayer;

    std::unordered_map<int, bool> keyState;

public:

    static void registerClass(HINSTANCE module);

    static Window* create(
        const wchar_t* title,
        SoundPlayer* soundPlayer,
        HINSTANCE module);

    ~Window();

    void destroy();

    void show(int state);

private:

    Window(HINSTANCE module, SoundPlayer* soundPlayer);

    bool createWindow(const wchar_t* title);

    LRESULT handleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    LRESULT handleNotificationMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    bool handleCommand(WPARAM wParam, LPARAM lParam);

    void handleRawInput(HRAWINPUT rawInput);

    void handleKeyboardEvent(const RAWKEYBOARD& keyboard);

    LRESULT callDefaultHandler(UINT msg, WPARAM wParam, LPARAM lParam);

    void addNotificationIcon();

    void deleteNotificationIcon();

    void showContextMenu(const POINT& pt);

    static LRESULT windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};
