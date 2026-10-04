#pragma once
#include <d2d1_1.h>
#include <dwrite.h>
#include <string>
#include "ui/Theme.h"
#include "ui/Icons.h"

namespace mosaic::ui::components {

class WidgetCard {
public:
    static D2D1_RECT_F DrawFrame(
        ID2D1DeviceContext* ctx,
        ID2D1SolidColorBrush* brush,
        IDWriteTextFormat* titleFormat,
        const ThemeManager& theme,
        D2D1_RECT_F bounds,
        const std::wstring& title,
        icons::IconKind icon = icons::IconKind::None,
        D2D1_COLOR_F iconColor = D2D1::ColorF(D2D1::ColorF::White),
        bool hovered = false);
};

} // namespace mosaic::ui::components
