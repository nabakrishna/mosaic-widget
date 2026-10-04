#pragma once
#include <d2d1_1.h>
#include "ui/Theme.h"

namespace mosaic::ui::components {
class GlassPanel {
public:

    static void Draw(ID2D1DeviceContext* ctx, ID2D1SolidColorBrush* scratchBrush,
                      D2D1_RECT_F rect, const ThemeManager& theme, bool hovered = false);
};

} // namespace mosaic::ui::components
