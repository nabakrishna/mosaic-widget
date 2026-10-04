#pragma once
#include <d2d1_1.h>
#include <wrl/client.h>

namespace mosaic::ui::icons {

enum class IconKind {
    None,
    Leaf,          // greeting accent
    Gear,          // settings button
    Star,          // special activity
    Pin,           // pinned items
    Lock,          // quick notes lock affordance
    Check,         // completed to-do (drawn over the checkbox fill)
    Close,         // per-row delete "x", shown on hover
    Plus,          // add-task / add-item buttons
    ChevronRight,  // settings category rows (Phase 7)
    ChevronUp,     // move-up reorder button, shown on row hover
    ChevronDown,   // move-down reorder button, shown on row hover
};

void Draw(ID2D1DeviceContext* ctx, ID2D1SolidColorBrush* brush,
          IconKind kind, D2D1_RECT_F bounds, D2D1_COLOR_F color);

} // namespace mosaic::ui::icons
