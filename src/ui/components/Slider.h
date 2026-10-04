#pragma once
#include <d2d1_1.h>
#include <algorithm>
#include "ui/Theme.h"

namespace mosaic::ui::components {

class Slider {
public:
    void SetBounds(D2D1_RECT_F bounds) { m_bounds = bounds; }
    void SetValue(float v) { m_value = std::clamp(v, 0.0f, 1.0f); }
    float Value() const { return m_value; }

    bool HitTest(D2D1_POINT_2F point) const {
        // Generous vertical hit area (full track height + a little) so the
        // thumb is easy to grab without needing pixel-perfect precision.
        return point.x >= m_bounds.left && point.x <= m_bounds.right &&
               point.y >= m_bounds.top - 6.0f && point.y <= m_bounds.bottom + 6.0f;
    }
    void SetValueFromPointerX(float x) {
        float width = m_bounds.right - m_bounds.left;
        if (width <= 0.0f) return;
        m_value = std::clamp((x - m_bounds.left) / width, 0.0f, 1.0f);
    }

    void SetHovered(bool hovered) { m_hovered = hovered; }
    void SetDragging(bool dragging) { m_dragging = dragging; }

    void Draw(ID2D1DeviceContext* ctx, ID2D1SolidColorBrush* brush, const ThemeManager& theme) const {
        const ThemeColors& colors = theme.Colors();
        float trackY = (m_bounds.top + m_bounds.bottom) * 0.5f;
        float trackHeight = 4.0f;

        D2D1_RECT_F fullTrack = { m_bounds.left, trackY - trackHeight * 0.5f, m_bounds.right, trackY + trackHeight * 0.5f };
        D2D1_ROUNDED_RECT fullRR = D2D1::RoundedRect(fullTrack, trackHeight * 0.5f, trackHeight * 0.5f);
        brush->SetColor(colors.cardFill);
        ctx->FillRoundedRectangle(fullRR, brush);

        float fillX = m_bounds.left + (m_bounds.right - m_bounds.left) * m_value;
        D2D1_RECT_F filledTrack = { m_bounds.left, trackY - trackHeight * 0.5f, fillX, trackY + trackHeight * 0.5f };
        D2D1_ROUNDED_RECT filledRR = D2D1::RoundedRect(filledTrack, trackHeight * 0.5f, trackHeight * 0.5f);
        brush->SetColor(colors.accentViolet);
        ctx->FillRoundedRectangle(filledRR, brush);

        float thumbRadius = (m_dragging || m_hovered) ? 8.0f : 7.0f;
        D2D1_ELLIPSE thumb = D2D1::Ellipse({ fillX, trackY }, thumbRadius, thumbRadius);
        brush->SetColor(colors.textPrimary);
        ctx->FillEllipse(thumb, brush);
    }

private:
    D2D1_RECT_F m_bounds{};
    float m_value = 0.0f;
    bool m_hovered = false;
    bool m_dragging = false;
};

} // namespace mosaic::ui::components
