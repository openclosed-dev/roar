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
#include "Window.h"
#include "resource.h"
#include "SoundPlayer.h"

static const wchar_t CLASS_NAME[] = L"RoarWindow";
static const GUID NOTIFICATION_GUID = {0xdcae2d01, 0x416c, 0x4743, { 0xb6, 0x1c, 0x6c, 0xbc, 0xd1, 0x84, 0x67, 0x20}};

static const UINT WM_NOTIFICATION_CALLBACK = WM_APP + 1;

void Window::registerClass(HINSTANCE module) {
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = windowProc;
    wc.hInstance = module;
    wc.hIcon = ::LoadIcon(module, MAKEINTRESOURCEW(IDI_NOTIFICATION_ICON));
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
    wc.lpszClassName = CLASS_NAME;
    ::RegisterClassExW(&wc);
}

Window* Window::create(const wchar_t* title, SoundPlayer* soundPlayer, HINSTANCE module) {

    Window* window = new Window(module, soundPlayer);
    window->createWindow(title);

    return window;
}

Window::Window(HINSTANCE module, SoundPlayer* soundPlayer)
:   module(module),
    handle(nullptr),
    notificationIcon(nullptr),
    soundPlayer(soundPlayer) {
}

Window::~Window() {
    keyState.clear();
}

bool Window::createWindow(const wchar_t* title) {
    HWND hwnd = ::CreateWindowExW(
        0,
        CLASS_NAME,
        title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        this->module,
        this
    );

    RAWINPUTDEVICE device = {
        0x01,  // generic
        0x06,  // keyboard
        RIDEV_INPUTSINK,
        hwnd
    };
    ::RegisterRawInputDevices(&device, 1, sizeof(RAWINPUTDEVICE));

    return true;
}

void Window::destroy() {
    if (this->handle != nullptr) {
        ::DestroyWindow(this->handle);
    }
}

void Window::show(int state) {
    ::ShowWindow(this->handle, state);
}

LRESULT Window::handleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            addNotificationIcon();
            break;
        case WM_DESTROY:
            deleteNotificationIcon();
            ::PostQuitMessage(0);
            break;
        case WM_COMMAND:
            if (handleCommand(wParam, lParam)) {
                return 0;
            }
            break;
        case WM_INPUT:
            handleRawInput((HRAWINPUT) lParam);
            break;
        case WM_NOTIFICATION_CALLBACK:
            return handleNotificationMessage(msg, wParam, lParam);
    }
    return callDefaultHandler(msg, wParam, lParam);
}

LRESULT Window::handleNotificationMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (LOWORD(lParam)) {
        case WM_CONTEXTMENU: {
            POINT pt{};
            // wParam does not provide correct posision.
            ::GetCursorPos(&pt);
            showContextMenu(pt);
        }
        break;
    }
    return callDefaultHandler(msg, wParam, lParam);
}

bool Window::handleCommand(WPARAM wParam, LPARAM lParam) {
    switch (LOWORD(wParam)) {
        case IDM_EXIT:
            destroy();
            return true;
    }
    return false;
}

void Window::handleRawInput(HRAWINPUT rawInputHandle) {
    RAWINPUT rawInput;
    UINT bufferSize = sizeof(rawInput);

    if (::GetRawInputData(rawInputHandle, RID_INPUT, &rawInput, &bufferSize, sizeof(RAWINPUTHEADER))) {
        if (rawInput.header.dwType == RIM_TYPEKEYBOARD) {
            handleKeyboardEvent(rawInput.data.keyboard);
        }
     }
}

void Window::handleKeyboardEvent(const RAWKEYBOARD& keyboard) {
    std::uint16_t scanCode = keyboard.MakeCode & 0x00ff;
    if (keyboard.Flags & RI_KEY_E0) {
        scanCode |= 0xe000;
    } else if (keyboard.Flags & RI_KEY_E1) {
        scanCode |= 0xe100;
    }

    if ((keyboard.Flags & RI_KEY_BREAK) == 0) {
        // key down
        if (!keyState[scanCode]) {
            keyState[scanCode] = true;
            soundPlayer->playSound(scanCode);
        }
    } else {
        // key up
        keyState[scanCode] = false;
    }
}

LRESULT Window::callDefaultHandler(UINT msg, WPARAM wParam, LPARAM lParam) {
    return ::DefWindowProcW(this->handle, msg, wParam, lParam);
}

void Window::addNotificationIcon() {

    ::LoadIconMetric(this->module, MAKEINTRESOURCEW(IDI_NOTIFICATION_ICON), LIM_SMALL, &this->notificationIcon);

    NOTIFYICONDATA nid{};
    nid.cbSize = sizeof(nid);
    nid.hWnd = this->handle;
    nid.uFlags = NIF_GUID | NIF_ICON | NIF_TIP | NIF_SHOWTIP | NIF_MESSAGE;
    nid.guidItem = NOTIFICATION_GUID;
    nid.hIcon = notificationIcon;
    nid.uCallbackMessage = WM_NOTIFICATION_CALLBACK;
    ::LoadString(this->module, IDS_NOTIFICATION_TOOLTIP, nid.szTip, ARRAYSIZE(nid.szTip));
    ::Shell_NotifyIcon(NIM_ADD, &nid);

    nid.uVersion = NOTIFYICON_VERSION_4;
    ::Shell_NotifyIcon(NIM_SETVERSION, &nid);
}

void Window::deleteNotificationIcon() {
    NOTIFYICONDATA nid{};
    nid.cbSize = sizeof(nid);
    nid.uFlags = NIF_GUID;
    nid.guidItem = NOTIFICATION_GUID;
    ::Shell_NotifyIcon(NIM_DELETE, &nid);

    if (this->notificationIcon != nullptr) {
        ::DestroyIcon(this->notificationIcon);
        this->notificationIcon = nullptr;
    }
}

void Window::showContextMenu(const POINT& pt) {

    HMENU menu = ::LoadMenu(this->module, MAKEINTRESOURCE(IDC_CONTEXT_MENU));
    HMENU submenu = ::GetSubMenu(menu, 0);

    ::SetForegroundWindow(this->handle);

    UINT flags = TPM_RIGHTBUTTON;
    if (::GetSystemMetrics(SM_MENUDROPALIGNMENT) != 0) {
        flags |= TPM_RIGHTALIGN;
    } else {
        flags |= TPM_LEFTALIGN;
    }
    ::TrackPopupMenuEx(submenu, flags, pt.x, pt.y, this->handle, nullptr);

    ::DestroyMenu(menu);
}

LRESULT CALLBACK Window::windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    Window* window = nullptr;
    if (msg == WM_NCCREATE) {
        CREATESTRUCT* cs = (CREATESTRUCT*) lParam;
        window = (Window*) cs->lpCreateParams;
        window->handle = hwnd;
        ::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) window);
    } else {
        window = (Window*)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    if (window != nullptr) {
        return window->handleMessage(msg, wParam, lParam);
    } else {
        return ::DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

