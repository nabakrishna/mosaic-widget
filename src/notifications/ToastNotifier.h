#pragma once
#include <string>

namespace mosaic::notifications {

class ToastNotifier {
public:

    static void EnsureAppIdentity();

    static void Show(const std::wstring& title, const std::wstring& body);

private:
    static const wchar_t* AppUserModelId();
};

} // namespace mosaic::notifications
