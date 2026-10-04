#pragma once
#include <windows.h>
#include <memory>
#include "platform/Window.h"

namespace mosaic::app {

class Application {
public:
    int Run(HINSTANCE hInstance, int nCmdShow);

private:
    std::unique_ptr<platform::Window> m_window;
};

} // namespace mosaic::app
