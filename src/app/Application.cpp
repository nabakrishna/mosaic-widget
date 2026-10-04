#include "app/Application.h"
#include "notifications/ToastNotifier.h"
#include <combaseapi.h>

namespace mosaic::app {

int Application::Run(HINSTANCE hInstance, int nCmdShow) {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) return -1;
    notifications::ToastNotifier::EnsureAppIdentity();

    m_window = std::make_unique<platform::Window>();
    hr = m_window->Create(hInstance, nCmdShow);
    if (FAILED(hr)) {
        CoUninitialize();
        return -1;
    }

    int exitCode = m_window->RunMessageLoop();

    m_window.reset();
    CoUninitialize();
    return exitCode;
}

} // namespace mosaic::app
