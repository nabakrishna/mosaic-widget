// #include "ui/SettingsPanel.h"
// #include "ui/Icons.h"
// #include "ui/components/GlassPanel.h"
// #include <windows.h>
// #include <algorithm>
// #include <cwchar>

// using namespace mosaic::widgets;

// namespace mosaic::ui {

// SettingsPanel::SettingsPanel(app::AppSettings* settings, SettingsCallbacks callbacks)
//     : m_settings(settings), m_callbacks(std::move(callbacks)) {
//     m_categories = {
//         { Category::Appearance,     L"Appearance",       true },
//         { Category::WidgetLayout,   L"Widget Layout",    true },
//         { Category::PhotoMedia,     L"Photo & Media",    true },
//         { Category::DateGreeting,   L"Date & Greeting",  false },
//         { Category::Todo,           L"To Do",            false },
//         { Category::SpecialActivity,L"Special Activity", false },
//         { Category::QuickNotes,     L"Quick Notes",      true },
//         { Category::Desktop,        L"Desktop",          true },
//         { Category::Performance,    L"Performance",      true },
//         { Category::Privacy,        L"Privacy & Data",   false },
//         { Category::General,        L"General",          false },
//         { Category::About,          L"About",            true },
//     };
// }

// // --- top-level draw --------------------------------------------------------

// void SettingsPanel::Draw(ID2D1DeviceContext* ctx, ID2D1SolidColorBrush* brush,
//                           IDWriteTextFormat* titleFormat, IDWriteTextFormat* bodyFormat, IDWriteTextFormat* smallFormat,
//                           D2D1_RECT_F fullWindowBounds, const ThemeManager& theme) {
//     if (!m_open) return;

//     m_clickZones.clear();
//     m_toggleBindings.clear();
//     m_activeSlider = nullptr;
//     m_activeSliderOnChange = nullptr;

//     DrawScrim(ctx, brush, fullWindowBounds, theme);

//     float panelWidth = (std::min)(640.0f, (fullWindowBounds.right - fullWindowBounds.left) - 40.0f);
//     float panelHeight = (std::min)(480.0f, (fullWindowBounds.bottom - fullWindowBounds.top) - 40.0f);
//     float cx = (fullWindowBounds.left + fullWindowBounds.right) * 0.5f;
//     float cy = (fullWindowBounds.top + fullWindowBounds.bottom) * 0.5f;
//     m_panelBounds = { cx - panelWidth * 0.5f, cy - panelHeight * 0.5f, cx + panelWidth * 0.5f, cy + panelHeight * 0.5f };

//     components::GlassPanel::Draw(ctx, brush, m_panelBounds, theme);

//     D2D1_POINT_2F closeCenter = { m_panelBounds.right - 26.0f, m_panelBounds.top + 26.0f };
//     m_closeButton.SetBounds(closeCenter, 14.0f);
//     m_closeButton.Draw(ctx, brush, icons::IconKind::Close, theme);

//     D2D1_RECT_F titleRect = { m_panelBounds.left + 24.0f, m_panelBounds.top + 18.0f, m_panelBounds.right - 56.0f, m_panelBounds.top + 42.0f };
//     brush->SetColor(theme.Colors().textPrimary);
//     static const wchar_t* kSettingsTitle = L"Settings";
//     ctx->DrawText(kSettingsTitle, 8, titleFormat, titleRect, brush, D2D1_DRAW_TEXT_OPTIONS_CLIP);

//     float categoryListWidth = 168.0f;
//     D2D1_RECT_F categoryListBounds = {
//         m_panelBounds.left + 12.0f, m_panelBounds.top + 60.0f,
//         m_panelBounds.left + 12.0f + categoryListWidth, m_panelBounds.bottom - 16.0f
//     };
//     D2D1_RECT_F contentBounds = {
//         categoryListBounds.right + 16.0f, m_panelBounds.top + 60.0f,
//         m_panelBounds.right - 20.0f, m_panelBounds.bottom - 20.0f
//     };

//     DrawCategoryList(ctx, brush, bodyFormat, categoryListBounds, theme);

//     std::wstring categoryTitle;
//     for (const auto& c : m_categories) {
//         if (c.id == m_selectedCategory) { categoryTitle = c.label; break; }
//     }
//     D2D1_RECT_F contentHeaderRect = { contentBounds.left, contentBounds.top, contentBounds.right, contentBounds.top + 26.0f };
//     brush->SetColor(theme.Colors().textPrimary);
//     ctx->DrawText(categoryTitle.c_str(), static_cast<UINT32>(categoryTitle.size()), titleFormat, contentHeaderRect, brush,
//                   D2D1_DRAW_TEXT_OPTIONS_CLIP);

//     D2D1_RECT_F contentBody = { contentBounds.left, contentHeaderRect.bottom + 14.0f, contentBounds.right, contentBounds.bottom };

//     switch (m_selectedCategory) {
//     case Category::Appearance:   DrawAppearance(ctx, brush, bodyFormat, smallFormat, contentBody, theme); break;
//     case Category::WidgetLayout: DrawWidgetLayout(ctx, brush, bodyFormat, contentBody, theme); break;
//     case Category::PhotoMedia:   DrawPhotoMedia(ctx, brush, bodyFormat, smallFormat, contentBody, theme); break;
//     case Category::Desktop:      DrawDesktop(ctx, brush, bodyFormat, contentBody, theme); break;
//     case Category::Performance:  DrawPerformance(ctx, brush, bodyFormat, contentBody, theme); break;
//     case Category::QuickNotes:   DrawQuickNotes(ctx, brush, bodyFormat, smallFormat, contentBody, theme); break;
//     case Category::About:        DrawAbout(ctx, brush, bodyFormat, smallFormat, contentBody, theme); break;
//     default:
//         DrawNotImplementedNotice(ctx, brush, bodyFormat, contentBody, theme);
//         break;
//     }
// }

// void SettingsPanel::DrawScrim(ID2D1DeviceContext* ctx, ID2D1SolidColorBrush* brush, D2D1_RECT_F fullBounds, const ThemeManager& theme) {
//     brush->SetColor(WithAlpha(theme.Colors().backgroundFallback, 0.45f));
//     ctx->FillRectangle(fullBounds, brush);
// }

// void SettingsPanel::DrawCategoryList(ID2D1DeviceContext* ctx, ID2D1SolidColorBrush* brush, IDWriteTextFormat* bodyFormat,
//                                       D2D1_RECT_F bounds, const ThemeManager& theme) {
//     const ThemeColors& colors = theme.Colors();
//     m_categoryRowBounds.clear();
//     float y = bounds.top;
//     const float rowHeight = 32.0f;

//     for (const auto& cat : m_categories) {
//         D2D1_RECT_F rowRect = { bounds.left, y, bounds.right, y + rowHeight };
//         m_categoryRowBounds.push_back(rowRect);

//         bool selected = (cat.id == m_selectedCategory);
//         if (selected) {
//             D2D1_RECT_F highlightRect = { rowRect.left, rowRect.top + 2.0f, rowRect.right, rowRect.bottom - 2.0f };
//             D2D1_ROUNDED_RECT rr = D2D1::RoundedRect(highlightRect, 8.0f, 8.0f);
//             brush->SetColor(colors.cardFillHover);
//             ctx->FillRoundedRectangle(rr, brush);
//         }

//         D2D1_RECT_F textRect = { rowRect.left + 14.0f, rowRect.top, rowRect.right - 8.0f, rowRect.bottom };
//         D2D1_COLOR_F textColor = selected ? colors.textPrimary : (cat.implemented ? colors.textSecondary : colors.textMuted);
//         brush->SetColor(textColor);
//         ctx->DrawText(cat.label.c_str(), static_cast<UINT32>(cat.label.size()), bodyFormat, textRect, brush,
//                       D2D1_DRAW_TEXT_OPTIONS_CLIP);

//         Category id = cat.id;
//         m_clickZones.push_back({ rowRect, [this, id] { m_selectedCategory = id; } });

//         y += rowHeight;
//     }
// }

// void SettingsPanel::DrawNotImplementedNotice(ID2D1DeviceContext* ctx, ID2D1SolidColorBrush* brush, IDWriteTextFormat* bodyFormat,
//                                               D2D1_RECT_F bounds, const ThemeManager& theme) {
//     brush->SetColor(theme.Colors().textMuted);
//     static const wchar_t* kNotice = L"This section isn't wired up yet \u2014 coming in a later pass.";
//     ctx->DrawText(kNotice, static_cast<UINT32>(wcslen(kNotice)), bodyFormat, bounds, brush, D2D1_DRAW_TEXT_OPTIONS_CLIP);
// }

// // --- shared row helper -------------------------------------------------

// float SettingsPanel::DrawToggleRow(ID2D1DeviceContext* ctx, ID2D1SolidColorBrush* brush, IDWriteTextFormat* bodyFormat,
//                                     D2D1_RECT_F bounds, float y, const std::wstring& label,
//                                     components::Toggle& toggle, bool currentValue, std::function<void(bool)> onChange,
//                                     const ThemeManager& theme) {
//     const ThemeColors& colors = theme.Colors();
//     const float rowHeight = 36.0f;

//     D2D1_RECT_F labelRect = { bounds.left, y + 7.0f, bounds.right - 56.0f, y + 27.0f };
//     brush->SetColor(colors.textPrimary);
//     ctx->DrawText(label.c_str(), static_cast<UINT32>(label.size()), bodyFormat, labelRect, brush, D2D1_DRAW_TEXT_OPTIONS_CLIP);

//     float toggleWidth = 40.0f, toggleHeight = 22.0f;
//     D2D1_RECT_F toggleBounds = {
//         bounds.right - toggleWidth, y + (rowHeight - toggleHeight) * 0.5f,
//         bounds.right, y + (rowHeight - toggleHeight) * 0.5f + toggleHeight
//     };
//     toggle.SetBounds(toggleBounds);
//     toggle.SetValue(currentValue);
//     toggle.Draw(ctx, brush, theme);

//     m_toggleBindings.push_back({ &toggle, std::move(onChange) });

//     return y + rowHeight + 6.0f;
// }

// // --- per-category content --------------------------------------------

// void SettingsPanel::DrawAppearance(ID2D1DeviceContext* ctx, ID2D1SolidColorBrush* brush, IDWriteTextFormat* bodyFormat,
//                                     IDWriteTextFormat* smallFormat, D2D1_RECT_F bounds, const ThemeManager& theme) {
//     const ThemeColors& colors = theme.Colors();
//     float y = bounds.top;

//     D2D1_RECT_F labelRect = { bounds.left, y, bounds.right, y + 18.0f };
//     brush->SetColor(colors.textPrimary);
//     static const wchar_t* kTransparencyLabel = L"Transparency";
//     ctx->DrawText(kTransparencyLabel, 12, bodyFormat, labelRect, brush, D2D1_DRAW_TEXT_OPTIONS_CLIP);
//     y += 22.0f;

//     D2D1_RECT_F sliderBounds = { bounds.left, y, bounds.right - 8.0f, y + 20.0f };
//     m_transparencySlider.SetBounds(sliderBounds);
//     m_transparencySlider.SetValue(m_settings->transparency);
//     m_transparencySlider.Draw(ctx, brush, theme);
//     m_activeSlider = &m_transparencySlider;
//     m_activeSliderOnChange = [this](float v) { m_settings->transparency = v; NotifyChanged(); };
//     y += 36.0f;

//     D2D1_RECT_F cornerLabelRect = { bounds.left, y, bounds.right, y + 18.0f };
//     static const wchar_t* kCornerLabel = L"Corner Radius";
//     ctx->DrawText(kCornerLabel, 13, bodyFormat, cornerLabelRect, brush, D2D1_DRAW_TEXT_OPTIONS_CLIP);
//     y += 22.0f;

//     static const wchar_t* kCornerNames[4] = { L"Sharp", L"Small", L"Medium", L"Large" };
//     float btnWidth = (bounds.right - bounds.left - 3.0f * 8.0f) / 4.0f;
//     float bx = bounds.left;
//     for (int i = 0; i < 4; ++i) {
//         D2D1_RECT_F btnRect = { bx, y, bx + btnWidth, y + 30.0f };
//         bool selected = (m_settings->cornerRadiusPreset == i);
//         D2D1_ROUNDED_RECT rr = D2D1::RoundedRect(btnRect, 8.0f, 8.0f);
//         brush->SetColor(selected ? colors.accentViolet : colors.cardFill);
//         ctx->FillRoundedRectangle(rr, brush);
//         brush->SetColor(colors.cardBorder);
//         ctx->DrawRoundedRectangle(rr, brush, 1.0f);
//         brush->SetColor(selected ? colors.textPrimary : colors.textSecondary);
//         ctx->DrawText(kCornerNames[i], static_cast<UINT32>(wcslen(kCornerNames[i])), smallFormat, btnRect, brush,
//                       D2D1_DRAW_TEXT_OPTIONS_CLIP);

//         int preset = i;
//         m_clickZones.push_back({ btnRect, [this, preset] { m_settings->cornerRadiusPreset = preset; NotifyChanged(); } });
//         bx += btnWidth + 8.0f;
//     }
//     y += 42.0f;

//     D2D1_RECT_F accentLabelRect = { bounds.left, y, bounds.right, y + 18.0f };
//     static const wchar_t* kAccentLabel = L"Accent Color";
//     ctx->DrawText(kAccentLabel, 12, bodyFormat, accentLabelRect, brush, D2D1_DRAW_TEXT_OPTIONS_CLIP);
//     y += 24.0f;

//     static const D2D1_COLOR_F kAccentPalette[5] = {
//         { 0.55f, 0.50f, 0.95f, 1.0f }, // violet (default)
//         { 0.35f, 0.55f, 0.95f, 1.0f }, // blue
//         { 0.42f, 0.85f, 0.55f, 1.0f }, // green
//         { 0.95f, 0.75f, 0.40f, 1.0f }, // amber
//         { 0.90f, 0.45f, 0.55f, 1.0f }, // rose
//     };
//     float swatchSize = 28.0f;
//     float sx = bounds.left;
//     for (int i = 0; i < 5; ++i) {
//         D2D1_ELLIPSE swatch = D2D1::Ellipse({ sx + swatchSize * 0.5f, y + swatchSize * 0.5f }, swatchSize * 0.5f, swatchSize * 0.5f);
//         brush->SetColor(kAccentPalette[i]);
//         ctx->FillEllipse(swatch, brush);
//         if (m_settings->accentPreset == i) {
//             brush->SetColor(colors.textPrimary);
//             ctx->DrawEllipse(swatch, brush, 2.0f);
//         }
//         D2D1_RECT_F swatchRect = { sx, y, sx + swatchSize, y + swatchSize };
//         int preset = i;
//         m_clickZones.push_back({ swatchRect, [this, preset] { m_settings->accentPreset = preset; NotifyChanged(); } });
//         sx += swatchSize + 12.0f;
//     }
//     y += swatchSize + 16.0f;

//     y = DrawToggleRow(ctx, brush, bodyFormat, bounds, y, L"Background Blur", m_blurToggle, m_settings->blurEnabled,
//                        [this](bool v) { m_settings->blurEnabled = v; NotifyChanged(); }, theme);
//     DrawToggleRow(ctx, brush, bodyFormat, bounds, y, L"Strong Widget Shadow", m_strongShadowToggle, m_settings->strongShadow,
//                   [this](bool v) { m_settings->strongShadow = v; NotifyChanged(); }, theme);
// }

// void SettingsPanel::DrawWidgetLayout(ID2D1DeviceContext* ctx, ID2D1SolidColorBrush* brush, IDWriteTextFormat* bodyFormat,
//                                       D2D1_RECT_F bounds, const ThemeManager& theme) {
//     const ThemeColors& colors = theme.Colors();
//     float y = bounds.top;

//     static const wchar_t* kWidgetLabels[5] = { L"To Do", L"Photo", L"Special Activity", L"Pinned", L"Quick Notes" };
//     static const WidgetId kWidgetIds[5] = {
//         WidgetId::Todo, WidgetId::Photo, WidgetId::Activity, WidgetId::Pinned, WidgetId::QuickNotes
//     };

//     for (int i = 0; i < 5; ++i) {
//         WidgetId id = kWidgetIds[i];
//         y = DrawToggleRow(ctx, brush, bodyFormat, bounds, y, kWidgetLabels[i], m_widgetToggles[i],
//                            m_settings->IsWidgetEnabled(id),
//                            [this, id](bool v) { m_settings->SetWidgetEnabled(id, v); NotifyChanged(); }, theme);
//     }

//     y += 12.0f;
//     D2D1_RECT_F resetButtonRect = { bounds.left, y, bounds.left + 150.0f, y + 34.0f };
//     D2D1_ROUNDED_RECT rr = D2D1::RoundedRect(resetButtonRect, 10.0f, 10.0f);
//     brush->SetColor(colors.cardFill);
//     ctx->FillRoundedRectangle(rr, brush);
//     brush->SetColor(colors.cardBorder);
//     ctx->DrawRoundedRectangle(rr, brush, 1.0f);
//     brush->SetColor(colors.textPrimary);
//     static const wchar_t* kResetLabel = L"Reset Layout";
//     ctx->DrawText(kResetLabel, 12, bodyFormat, resetButtonRect, brush, D2D1_DRAW_TEXT_OPTIONS_CLIP);

//     m_clickZones.push_back({ resetButtonRect, [this] { if (m_callbacks.onResetLayout) m_callbacks.onResetLayout(); } });
// }

// void SettingsPanel::DrawPhotoMedia(ID2D1DeviceContext* ctx, ID2D1SolidColorBrush* brush, IDWriteTextFormat* bodyFormat,
//                                     IDWriteTextFormat* smallFormat, D2D1_RECT_F bounds, const ThemeManager& theme) {
//     const ThemeColors& colors = theme.Colors();
//     float y = bounds.top;

//     D2D1_RECT_F sourceLabelRect = { bounds.left, y, bounds.right, y + 18.0f };
//     brush->SetColor(colors.textPrimary);
//     static const wchar_t* kSourceLabel = L"Source: Local Folder (Pictures)";
//     ctx->DrawText(kSourceLabel, static_cast<UINT32>(wcslen(kSourceLabel)), bodyFormat, sourceLabelRect, brush,
//                   D2D1_DRAW_TEXT_OPTIONS_CLIP);
//     y += 22.0f;

//     D2D1_RECT_F noteRect = { bounds.left, y, bounds.right, y + 32.0f };
//     brush->SetColor(colors.textMuted);
//     static const wchar_t* kSourceNote = L"Online sources aren't available yet \u2014 see README.";
//     ctx->DrawText(kSourceNote, static_cast<UINT32>(wcslen(kSourceNote)), smallFormat, noteRect, brush,
//                   D2D1_DRAW_TEXT_OPTIONS_CLIP);
//     y += 40.0f;

//     D2D1_RECT_F folderButtonRect = { bounds.left, y, bounds.left + 160.0f, y + 34.0f };
//     D2D1_ROUNDED_RECT frr = D2D1::RoundedRect(folderButtonRect, 10.0f, 10.0f);
//     brush->SetColor(colors.cardFill);
//     ctx->FillRoundedRectangle(frr, brush);
//     brush->SetColor(colors.cardBorder);
//     ctx->DrawRoundedRectangle(frr, brush, 1.0f);
//     brush->SetColor(colors.textPrimary);
//     static const wchar_t* kChooseFolderLabel = L"Choose Folder\u2026";
//     ctx->DrawText(kChooseFolderLabel, static_cast<UINT32>(wcslen(kChooseFolderLabel)), bodyFormat, folderButtonRect, brush,
//                   D2D1_DRAW_TEXT_OPTIONS_CLIP);
//     m_clickZones.push_back({ folderButtonRect, [this] { if (m_callbacks.onPickPhotoFolder) m_callbacks.onPickPhotoFolder(); } });
//     y += 50.0f;

//     D2D1_RECT_F rotationLabelRect = { bounds.left, y, bounds.right, y + 18.0f };
//     static const wchar_t* kRotationLabel = L"Rotation Interval";
//     ctx->DrawText(kRotationLabel, static_cast<UINT32>(wcslen(kRotationLabel)), bodyFormat, rotationLabelRect, brush,
//                   D2D1_DRAW_TEXT_OPTIONS_CLIP);
//     y += 22.0f;

//     static const int kPresets[4] = { 1, 5, 15, 30 };
//     static const wchar_t* kPresetLabels[4] = { L"1 min", L"5 min", L"15 min", L"30 min" };
//     float btnWidth = (bounds.right - bounds.left - 3.0f * 8.0f) / 4.0f;
//     float bx = bounds.left;
//     for (int i = 0; i < 4; ++i) {
//         D2D1_RECT_F btnRect = { bx, y, bx + btnWidth, y + 30.0f };
//         bool selected = (m_settings->photoRotationMinutes == kPresets[i]);
//         D2D1_ROUNDED_RECT rr = D2D1::RoundedRect(btnRect, 8.0f, 8.0f);
//         brush->SetColor(selected ? colors.accentViolet : colors.cardFill);
//         ctx->FillRoundedRectangle(rr, brush);
//         brush->SetColor(colors.cardBorder);
//         ctx->DrawRoundedRectangle(rr, brush, 1.0f);
//         brush->SetColor(selected ? colors.textPrimary : colors.textSecondary);
//         ctx->DrawText(kPresetLabels[i], static_cast<UINT32>(wcslen(kPresetLabels[i])), smallFormat, btnRect, brush,
//                       D2D1_DRAW_TEXT_OPTIONS_CLIP);

//         int minutes = kPresets[i];
//         m_clickZones.push_back({ btnRect, [this, minutes] { m_settings->photoRotationMinutes = minutes; NotifyChanged(); } });
//         bx += btnWidth + 8.0f;
//     }
// }

// void SettingsPanel::DrawDesktop(ID2D1DeviceContext* ctx, ID2D1SolidColorBrush* brush, IDWriteTextFormat* bodyFormat,
//                                  D2D1_RECT_F bounds, const ThemeManager& theme) {
//     float y = bounds.top;
//     y = DrawToggleRow(ctx, brush, bodyFormat, bounds, y, L"Always on Top", m_alwaysOnTopToggle, m_settings->alwaysOnTop,
//                        [this](bool v) { m_settings->alwaysOnTop = v; NotifyChanged(); }, theme);
//     DrawToggleRow(ctx, brush, bodyFormat, bounds, y, L"Start with Windows", m_startWithWindowsToggle, m_settings->startWithWindows,
//                   [this](bool v) { m_settings->startWithWindows = v; NotifyChanged(); }, theme);
// }

// void SettingsPanel::DrawPerformance(ID2D1DeviceContext* ctx, ID2D1SolidColorBrush* brush, IDWriteTextFormat* bodyFormat,
//                                      D2D1_RECT_F bounds, const ThemeManager& theme) {
//     float y = bounds.top;
//     DrawToggleRow(ctx, brush, bodyFormat, bounds, y, L"Animations", m_animationsToggle, m_settings->animationsEnabled,
//                   [this](bool v) { m_settings->animationsEnabled = v; NotifyChanged(); }, theme);
// }

// void SettingsPanel::DrawQuickNotes(ID2D1DeviceContext* ctx, ID2D1SolidColorBrush* brush, IDWriteTextFormat* bodyFormat,
//                                    IDWriteTextFormat* smallFormat, D2D1_RECT_F bounds, const ThemeManager& theme) {
//     const ThemeColors& colors = theme.Colors();
//     float y = bounds.top;

//     D2D1_RECT_F labelRect = { bounds.left, y, bounds.right, y + 18.0f };
//     brush->SetColor(colors.textPrimary);
//     static const wchar_t* kAutoLockLabel = L"Auto-lock after";
//     ctx->DrawText(kAutoLockLabel, static_cast<UINT32>(wcslen(kAutoLockLabel)), bodyFormat, labelRect, brush,
//                   D2D1_DRAW_TEXT_OPTIONS_CLIP);
//     y += 22.0f;

//     static const int kTimeouts[4] = { 0, 60, 300, 1800 };
//     static const wchar_t* kTimeoutLabels[4] = { L"Never", L"1 min", L"5 min", L"30 min" };
//     float btnWidth = (bounds.right - bounds.left - 3.0f * 8.0f) / 4.0f;
//     float bx = bounds.left;
//     for (int i = 0; i < 4; ++i) {
//         D2D1_RECT_F btnRect = { bx, y, bx + btnWidth, y + 30.0f };
//         bool selected = (m_settings->noteAutoLockSeconds == kTimeouts[i]);
//         D2D1_ROUNDED_RECT rr = D2D1::RoundedRect(btnRect, 8.0f, 8.0f);
//         brush->SetColor(selected ? colors.accentViolet : colors.cardFill);
//         ctx->FillRoundedRectangle(rr, brush);
//         brush->SetColor(colors.cardBorder);
//         ctx->DrawRoundedRectangle(rr, brush, 1.0f);
//         brush->SetColor(selected ? colors.textPrimary : colors.textSecondary);
//         ctx->DrawText(kTimeoutLabels[i], static_cast<UINT32>(wcslen(kTimeoutLabels[i])), smallFormat, btnRect, brush,
//                       D2D1_DRAW_TEXT_OPTIONS_CLIP);

//         int seconds = kTimeouts[i];
//         m_clickZones.push_back({ btnRect, [this, seconds] { m_settings->noteAutoLockSeconds = seconds; NotifyChanged(); } });
//         bx += btnWidth + 8.0f;
//     }
//     y += 44.0f;

//     y = DrawToggleRow(ctx, brush, bodyFormat, bounds, y, L"Lock when window loses focus",
//                        m_lockOnFocusLossToggle, m_settings->lockNotesOnFocusLoss,
//                        [this](bool v) { m_settings->lockNotesOnFocusLoss = v; NotifyChanged(); }, theme);

//     D2D1_RECT_F noteRect = { bounds.left, y + 6.0f, bounds.right, y + 60.0f };
//     brush->SetColor(colors.textMuted);
//     static const wchar_t* kCryptoNote =
//         L"Notes are encrypted with Windows DPAPI, tied to your Windows account. "
//         L"The password is never stored \u2014 only a salted hash of it.";
//     ctx->DrawText(kCryptoNote, static_cast<UINT32>(wcslen(kCryptoNote)), smallFormat, noteRect, brush,
//                   D2D1_DRAW_TEXT_OPTIONS_CLIP);
// }

// void SettingsPanel::DrawAbout(ID2D1DeviceContext* ctx, ID2D1SolidColorBrush* brush, IDWriteTextFormat* bodyFormat,
//                                IDWriteTextFormat* smallFormat, D2D1_RECT_F bounds, const ThemeManager& theme) {
//     const ThemeColors& colors = theme.Colors();
//     float y = bounds.top;

//     D2D1_RECT_F nameRect = { bounds.left, y, bounds.right, y + 22.0f };
//     brush->SetColor(colors.textPrimary);
//     static const wchar_t* kName = L"Mosaic";
//     ctx->DrawText(kName, 6, bodyFormat, nameRect, brush, D2D1_DRAW_TEXT_OPTIONS_CLIP);
//     y += 26.0f;

//     D2D1_RECT_F descRect = { bounds.left, y, bounds.right, y + 80.0f };
//     brush->SetColor(colors.textMuted);
//     static const wchar_t* kDesc =
//         L"A minimal adaptive desktop workspace. Built with native Win32, "
//         L"Direct2D, and DirectComposition \u2014 no browser runtime.";
//     ctx->DrawText(kDesc, static_cast<UINT32>(wcslen(kDesc)), smallFormat, descRect, brush, D2D1_DRAW_TEXT_OPTIONS_CLIP);
// }

// // --- input dispatch --------------------------------------------------------

// bool SettingsPanel::OnMouseMove(D2D1_POINT_2F pt) {
//     if (!m_open) return false;

//     m_closeButton.SetHovered(m_closeButton.HitTest(pt));
//     for (auto& binding : m_toggleBindings) {
//         binding.toggle->SetHovered(binding.toggle->HitTest(pt));
//     }
//     if (m_activeSlider) {
//         m_activeSlider->SetHovered(m_activeSlider->HitTest(pt));
//         if (m_draggingSlider) {
//             m_activeSlider->SetValueFromPointerX(pt.x);
//             if (m_activeSliderOnChange) m_activeSliderOnChange(m_activeSlider->Value());
//         }
//     }

//     // The panel is an occasional-use, modal-style surface (not the
//     // always-on dashboard) — repainting on every move while it's open,
//     // rather than tracking exact hover-state deltas, is a reasonable,
//     // deliberately simple trade-off given that cost only exists while
//     // Settings is literally open.
//     return true;
// }

// bool SettingsPanel::OnLButtonDown(D2D1_POINT_2F pt) {
//     if (!m_open) return false;

//     if (m_closeButton.HitTest(pt)) {
//         Close();
//         return true;
//     }

//     if (m_activeSlider && m_activeSlider->HitTest(pt)) {
//         m_draggingSlider = true;
//         m_activeSlider->SetDragging(true);
//         m_activeSlider->SetValueFromPointerX(pt.x);
//         if (m_activeSliderOnChange) m_activeSliderOnChange(m_activeSlider->Value());
//         return true;
//     }

//     for (auto& binding : m_toggleBindings) {
//         if (binding.toggle->HitTest(pt)) {
//             bool newValue = binding.toggle->Flip();
//             if (binding.onChange) binding.onChange(newValue);
//             return true;
//         }
//     }

//     for (auto& zone : m_clickZones) {
//         if (pt.x >= zone.rect.left && pt.x <= zone.rect.right && pt.y >= zone.rect.top && pt.y <= zone.rect.bottom) {
//             if (zone.onClick) zone.onClick();
//             return true;
//         }
//     }

//     // Clicking inside the panel but not on anything interactive is simply
//     // absorbed. Clicking the scrim outside the panel closes it — the
//     // standard "click outside to dismiss" affordance.
//     bool insidePanel = pt.x >= m_panelBounds.left && pt.x <= m_panelBounds.right &&
//                         pt.y >= m_panelBounds.top && pt.y <= m_panelBounds.bottom;
//     if (!insidePanel) {
//         Close();
//     }
//     return true;
// }

// bool SettingsPanel::OnLButtonUp(D2D1_POINT_2F /*pt*/) {
//     if (!m_open) return false;
//     if (m_draggingSlider) {
//         m_draggingSlider = false;
//         if (m_activeSlider) m_activeSlider->SetDragging(false);
//     }
//     return true;
// }

// bool SettingsPanel::OnKeyDown(unsigned int virtualKey) {
//     if (!m_open) return false;
//     if (virtualKey == VK_ESCAPE) {
//         Close();
//     }
//     return true; // absorb all keys while open so none leak to a focused dashboard widget underneath
// }

// } // namespace mosaic::ui


















//new code for modification on 27-09-2026
#include "ui/SettingsPanel.h"
#include "ui/Icons.h"
#include "ui/components/GlassPanel.h"
#include <windows.h>
#include <algorithm>
#include <cwchar>

using namespace mosaic::widgets;

namespace mosaic::ui {

SettingsPanel::SettingsPanel(app::AppSettings* settings, SettingsCallbacks callbacks)
    : m_settings(settings), m_callbacks(std::move(callbacks)) {
    m_categories = {
        { Category::Appearance,     L"Appearance",       true },
        { Category::WidgetLayout,   L"Widget Layout",    true },
        { Category::PhotoMedia,     L"Photo & Media",    true },
        { Category::DateGreeting,   L"Date & Greeting",  true },
        { Category::Todo,           L"To Do",            false },
        { Category::SpecialActivity,L"Special Activity", false },
        { Category::QuickNotes,     L"Quick Notes",      true },
        { Category::Desktop,        L"Desktop",          true },
        { Category::Performance,    L"Performance",      true },
        { Category::Privacy,        L"Privacy & Data",   true },
        { Category::General,        L"General",          true },
        { Category::About,          L"About",            true },
    };
}

// --- top-level draw --------------------------------------------------------

void SettingsPanel::Draw(ID2D1DeviceContext* ctx, ID2D1SolidColorBrush* brush,
                          IDWriteTextFormat* titleFormat, IDWriteTextFormat* bodyFormat, IDWriteTextFormat* smallFormat,
                          D2D1_RECT_F fullWindowBounds, const ThemeManager& theme) {
    if (!m_open) return;

    m_clickZones.clear();
    m_toggleBindings.clear();
    m_activeSlider = nullptr;
    m_activeSliderOnChange = nullptr;

    DrawScrim(ctx, brush, fullWindowBounds, theme);

    float panelWidth = (std::min)(640.0f, (fullWindowBounds.right - fullWindowBounds.left) - 40.0f);
    float panelHeight = (std::min)(480.0f, (fullWindowBounds.bottom - fullWindowBounds.top) - 40.0f);
    float cx = (fullWindowBounds.left + fullWindowBounds.right) * 0.5f;
    float cy = (fullWindowBounds.top + fullWindowBounds.bottom) * 0.5f;
    m_panelBounds = { cx - panelWidth * 0.5f, cy - panelHeight * 0.5f, cx + panelWidth * 0.5f, cy + panelHeight * 0.5f };

    components::GlassPanel::Draw(ctx, brush, m_panelBounds, theme);

    D2D1_POINT_2F closeCenter = { m_panelBounds.right - 26.0f, m_panelBounds.top + 26.0f };
    m_closeButton.SetBounds(closeCenter, 14.0f);
    m_closeButton.Draw(ctx, brush, icons::IconKind::Close, theme);

    D2D1_RECT_F titleRect = { m_panelBounds.left + 24.0f, m_panelBounds.top + 18.0f, m_panelBounds.right - 56.0f, m_panelBounds.top + 42.0f };
    brush->SetColor(theme.Colors().textPrimary);
    static const wchar_t* kSettingsTitle = L"Settings";
    ctx->DrawText(kSettingsTitle, 8, titleFormat, titleRect, brush, D2D1_DRAW_TEXT_OPTIONS_CLIP);

    float categoryListWidth = 168.0f;
    D2D1_RECT_F categoryListBounds = {
        m_panelBounds.left + 12.0f, m_panelBounds.top + 60.0f,
        m_panelBounds.left + 12.0f + categoryListWidth, m_panelBounds.bottom - 16.0f
    };
    D2D1_RECT_F contentBounds = {
        categoryListBounds.right + 16.0f, m_panelBounds.top + 60.0f,
        m_panelBounds.right - 20.0f, m_panelBounds.bottom - 20.0f
    };

    DrawCategoryList(ctx, brush, bodyFormat, categoryListBounds, theme);

    std::wstring categoryTitle;
    for (const auto& c : m_categories) {
        if (c.id == m_selectedCategory) { categoryTitle = c.label; break; }
    }
    D2D1_RECT_F contentHeaderRect = { contentBounds.left, contentBounds.top, contentBounds.right, contentBounds.top + 26.0f };
    brush->SetColor(theme.Colors().textPrimary);
    ctx->DrawText(categoryTitle.c_str(), static_cast<UINT32>(categoryTitle.size()), titleFormat, contentHeaderRect, brush,
                  D2D1_DRAW_TEXT_OPTIONS_CLIP);

    D2D1_RECT_F contentBody = { contentBounds.left, contentHeaderRect.bottom + 14.0f, contentBounds.right, contentBounds.bottom };

    switch (m_selectedCategory) {
    case Category::Appearance:   DrawAppearance(ctx, brush, bodyFormat, smallFormat, contentBody, theme); break;
    case Category::WidgetLayout: DrawWidgetLayout(ctx, brush, bodyFormat, contentBody, theme); break;
    case Category::PhotoMedia:   DrawPhotoMedia(ctx, brush, bodyFormat, smallFormat, contentBody, theme); break;
    case Category::Desktop:      DrawDesktop(ctx, brush, bodyFormat, contentBody, theme); break;
    case Category::Performance:  DrawPerformance(ctx, brush, bodyFormat, contentBody, theme); break;
    case Category::QuickNotes:   DrawQuickNotes(ctx, brush, bodyFormat, smallFormat, contentBody, theme); break;
    case Category::DateGreeting: DrawDateGreeting(ctx, brush, bodyFormat, smallFormat, contentBody, theme); break;
    case Category::About:        DrawAbout(ctx, brush, bodyFormat, smallFormat, contentBody, theme); break;
    case Category::General:      DrawGeneral(ctx, brush, bodyFormat, smallFormat, contentBody, theme); break;
    case Category::Privacy:      DrawPrivacy(ctx, brush, bodyFormat, smallFormat, contentBody, theme); break;
    default:
        DrawNotImplementedNotice(ctx, brush, bodyFormat, contentBody, theme);
        break;
    }
}

void SettingsPanel::DrawScrim(ID2D1DeviceContext* ctx, ID2D1SolidColorBrush* brush, D2D1_RECT_F fullBounds, const ThemeManager& theme) {
    brush->SetColor(WithAlpha(theme.Colors().backgroundFallback, 0.45f));
    ctx->FillRectangle(fullBounds, brush);
}

void SettingsPanel::DrawCategoryList(ID2D1DeviceContext* ctx, ID2D1SolidColorBrush* brush, IDWriteTextFormat* bodyFormat,
                                      D2D1_RECT_F bounds, const ThemeManager& theme) {
    const ThemeColors& colors = theme.Colors();
    m_categoryRowBounds.clear();
    float y = bounds.top;
    const float rowHeight = 32.0f;

    for (const auto& cat : m_categories) {
        D2D1_RECT_F rowRect = { bounds.left, y, bounds.right, y + rowHeight };
        m_categoryRowBounds.push_back(rowRect);

        bool selected = (cat.id == m_selectedCategory);
        if (selected) {
            D2D1_RECT_F highlightRect = { rowRect.left, rowRect.top + 2.0f, rowRect.right, rowRect.bottom - 2.0f };
            D2D1_ROUNDED_RECT rr = D2D1::RoundedRect(highlightRect, 8.0f, 8.0f);
            brush->SetColor(colors.cardFillHover);
            ctx->FillRoundedRectangle(rr, brush);
        }

        D2D1_RECT_F textRect = { rowRect.left + 14.0f, rowRect.top, rowRect.right - 8.0f, rowRect.bottom };
        D2D1_COLOR_F textColor = selected ? colors.textPrimary : (cat.implemented ? colors.textSecondary : colors.textMuted);
        brush->SetColor(textColor);
        ctx->DrawText(cat.label.c_str(), static_cast<UINT32>(cat.label.size()), bodyFormat, textRect, brush,
                      D2D1_DRAW_TEXT_OPTIONS_CLIP);

        Category id = cat.id;
        m_clickZones.push_back({ rowRect, [this, id] { m_selectedCategory = id; } });

        y += rowHeight;
    }
}

void SettingsPanel::DrawNotImplementedNotice(ID2D1DeviceContext* ctx, ID2D1SolidColorBrush* brush, IDWriteTextFormat* bodyFormat,
                                              D2D1_RECT_F bounds, const ThemeManager& theme) {
    brush->SetColor(theme.Colors().textMuted);
    static const wchar_t* kNotice = L"This section isn't wired up yet \u2014 coming in a later pass.";
    ctx->DrawText(kNotice, static_cast<UINT32>(wcslen(kNotice)), bodyFormat, bounds, brush, D2D1_DRAW_TEXT_OPTIONS_CLIP);
}

// --- shared row helper -------------------------------------------------

float SettingsPanel::DrawToggleRow(ID2D1DeviceContext* ctx, ID2D1SolidColorBrush* brush, IDWriteTextFormat* bodyFormat,
                                    D2D1_RECT_F bounds, float y, const std::wstring& label,
                                    components::Toggle& toggle, bool currentValue, std::function<void(bool)> onChange,
                                    const ThemeManager& theme) {
    const ThemeColors& colors = theme.Colors();
    const float rowHeight = 36.0f;

    D2D1_RECT_F labelRect = { bounds.left, y + 7.0f, bounds.right - 56.0f, y + 27.0f };
    brush->SetColor(colors.textPrimary);
    ctx->DrawText(label.c_str(), static_cast<UINT32>(label.size()), bodyFormat, labelRect, brush, D2D1_DRAW_TEXT_OPTIONS_CLIP);

    float toggleWidth = 40.0f, toggleHeight = 22.0f;
    D2D1_RECT_F toggleBounds = {
        bounds.right - toggleWidth, y + (rowHeight - toggleHeight) * 0.5f,
        bounds.right, y + (rowHeight - toggleHeight) * 0.5f + toggleHeight
    };
    toggle.SetBounds(toggleBounds);
    toggle.SetValue(currentValue);
    toggle.Draw(ctx, brush, theme);

    m_toggleBindings.push_back({ &toggle, std::move(onChange) });

    return y + rowHeight + 6.0f;
}

// --- per-category content --------------------------------------------

void SettingsPanel::DrawAppearance(ID2D1DeviceContext* ctx, ID2D1SolidColorBrush* brush, IDWriteTextFormat* bodyFormat,
                                    IDWriteTextFormat* smallFormat, D2D1_RECT_F bounds, const ThemeManager& theme) {
    const ThemeColors& colors = theme.Colors();
    float y = bounds.top;

    D2D1_RECT_F labelRect = { bounds.left, y, bounds.right, y + 18.0f };
    brush->SetColor(colors.textPrimary);
    static const wchar_t* kTransparencyLabel = L"Transparency";
    ctx->DrawText(kTransparencyLabel, 12, bodyFormat, labelRect, brush, D2D1_DRAW_TEXT_OPTIONS_CLIP);
    y += 22.0f;

    D2D1_RECT_F sliderBounds = { bounds.left, y, bounds.right - 8.0f, y + 20.0f };
    m_transparencySlider.SetBounds(sliderBounds);
    m_transparencySlider.SetValue(m_settings->transparency);
    m_transparencySlider.Draw(ctx, brush, theme);
    m_activeSlider = &m_transparencySlider;
    m_activeSliderOnChange = [this](float v) { m_settings->transparency = v; NotifyChanged(); };
    y += 36.0f;

    D2D1_RECT_F cornerLabelRect = { bounds.left, y, bounds.right, y + 18.0f };
    static const wchar_t* kCornerLabel = L"Corner Radius";
    ctx->DrawText(kCornerLabel, 13, bodyFormat, cornerLabelRect, brush, D2D1_DRAW_TEXT_OPTIONS_CLIP);
    y += 22.0f;

    static const wchar_t* kCornerNames[4] = { L"Sharp", L"Small", L"Medium", L"Large" };
    float btnWidth = (bounds.right - bounds.left - 3.0f * 8.0f) / 4.0f;
    float bx = bounds.left;
    for (int i = 0; i < 4; ++i) {
        D2D1_RECT_F btnRect = { bx, y, bx + btnWidth, y + 30.0f };
        bool selected = (m_settings->cornerRadiusPreset == i);
        D2D1_ROUNDED_RECT rr = D2D1::RoundedRect(btnRect, 8.0f, 8.0f);
        brush->SetColor(selected ? colors.accentViolet : colors.cardFill);
        ctx->FillRoundedRectangle(rr, brush);
        brush->SetColor(colors.cardBorder);
        ctx->DrawRoundedRectangle(rr, brush, 1.0f);
        brush->SetColor(selected ? colors.textPrimary : colors.textSecondary);
        ctx->DrawText(kCornerNames[i], static_cast<UINT32>(wcslen(kCornerNames[i])), smallFormat, btnRect, brush,
                      D2D1_DRAW_TEXT_OPTIONS_CLIP);

        int preset = i;
        m_clickZones.push_back({ btnRect, [this, preset] { m_settings->cornerRadiusPreset = preset; NotifyChanged(); } });
        bx += btnWidth + 8.0f;
    }
    y += 42.0f;

    D2D1_RECT_F accentLabelRect = { bounds.left, y, bounds.right, y + 18.0f };
    static const wchar_t* kAccentLabel = L"Accent Color";
    ctx->DrawText(kAccentLabel, 12, bodyFormat, accentLabelRect, brush, D2D1_DRAW_TEXT_OPTIONS_CLIP);
    y += 24.0f;

    static const D2D1_COLOR_F kAccentPalette[5] = {
        { 0.55f, 0.50f, 0.95f, 1.0f }, // violet (default)
        { 0.35f, 0.55f, 0.95f, 1.0f }, // blue
        { 0.42f, 0.85f, 0.55f, 1.0f }, // green
        { 0.95f, 0.75f, 0.40f, 1.0f }, // amber
        { 0.90f, 0.45f, 0.55f, 1.0f }, // rose
    };
    float swatchSize = 28.0f;
    float sx = bounds.left;
    for (int i = 0; i < 5; ++i) {
        D2D1_ELLIPSE swatch = D2D1::Ellipse({ sx + swatchSize * 0.5f, y + swatchSize * 0.5f }, swatchSize * 0.5f, swatchSize * 0.5f);
        brush->SetColor(kAccentPalette[i]);
        ctx->FillEllipse(swatch, brush);
        if (m_settings->accentPreset == i) {
            brush->SetColor(colors.textPrimary);
            ctx->DrawEllipse(swatch, brush, 2.0f);
        }
        D2D1_RECT_F swatchRect = { sx, y, sx + swatchSize, y + swatchSize };
        int preset = i;
        m_clickZones.push_back({ swatchRect, [this, preset] { m_settings->accentPreset = preset; NotifyChanged(); } });
        sx += swatchSize + 12.0f;
    }
    y += swatchSize + 16.0f;

    y = DrawToggleRow(ctx, brush, bodyFormat, bounds, y, L"Background Blur", m_blurToggle, m_settings->blurEnabled,
                       [this](bool v) { m_settings->blurEnabled = v; NotifyChanged(); }, theme);
    DrawToggleRow(ctx, brush, bodyFormat, bounds, y, L"Strong Widget Shadow", m_strongShadowToggle, m_settings->strongShadow,
                  [this](bool v) { m_settings->strongShadow = v; NotifyChanged(); }, theme);
}

void SettingsPanel::DrawWidgetLayout(ID2D1DeviceContext* ctx, ID2D1SolidColorBrush* brush, IDWriteTextFormat* bodyFormat,
                                      D2D1_RECT_F bounds, const ThemeManager& theme) {
    const ThemeColors& colors = theme.Colors();
    float y = bounds.top;

    static const wchar_t* kWidgetLabels[5] = { L"To Do", L"Photo", L"Special Activity", L"Pinned", L"Quick Notes" };
    static const WidgetId kWidgetIds[5] = {
        WidgetId::Todo, WidgetId::Photo, WidgetId::Activity, WidgetId::Pinned, WidgetId::QuickNotes
    };

    for (int i = 0; i < 5; ++i) {
        WidgetId id = kWidgetIds[i];
        y = DrawToggleRow(ctx, brush, bodyFormat, bounds, y, kWidgetLabels[i], m_widgetToggles[i],
                           m_settings->IsWidgetEnabled(id),
                           [this, id](bool v) { m_settings->SetWidgetEnabled(id, v); NotifyChanged(); }, theme);
    }

    y += 12.0f;
    D2D1_RECT_F resetButtonRect = { bounds.left, y, bounds.left + 150.0f, y + 34.0f };
    D2D1_ROUNDED_RECT rr = D2D1::RoundedRect(resetButtonRect, 10.0f, 10.0f);
    brush->SetColor(colors.cardFill);
    ctx->FillRoundedRectangle(rr, brush);
    brush->SetColor(colors.cardBorder);
    ctx->DrawRoundedRectangle(rr, brush, 1.0f);
    brush->SetColor(colors.textPrimary);
    static const wchar_t* kResetLabel = L"Reset Layout";
    ctx->DrawText(kResetLabel, 12, bodyFormat, resetButtonRect, brush, D2D1_DRAW_TEXT_OPTIONS_CLIP);

    m_clickZones.push_back({ resetButtonRect, [this] { if (m_callbacks.onResetLayout) m_callbacks.onResetLayout(); } });
}

void SettingsPanel::DrawPhotoMedia(ID2D1DeviceContext* ctx, ID2D1SolidColorBrush* brush, IDWriteTextFormat* bodyFormat,
                                    IDWriteTextFormat* smallFormat, D2D1_RECT_F bounds, const ThemeManager& theme) {
    const ThemeColors& colors = theme.Colors();
    float y = bounds.top;

    D2D1_RECT_F sourceLabelRect = { bounds.left, y, bounds.right, y + 18.0f };
    brush->SetColor(colors.textPrimary);
    static const wchar_t* kSourceLabel = L"Source: Local Folder (Pictures)";
    ctx->DrawText(kSourceLabel, static_cast<UINT32>(wcslen(kSourceLabel)), bodyFormat, sourceLabelRect, brush,
                  D2D1_DRAW_TEXT_OPTIONS_CLIP);
    y += 22.0f;

    D2D1_RECT_F noteRect = { bounds.left, y, bounds.right, y + 32.0f };
    brush->SetColor(colors.textMuted);
    static const wchar_t* kSourceNote = L"Online sources aren't available yet \u2014 see README.";
    ctx->DrawText(kSourceNote, static_cast<UINT32>(wcslen(kSourceNote)), smallFormat, noteRect, brush,
                  D2D1_DRAW_TEXT_OPTIONS_CLIP);
    y += 40.0f;

    D2D1_RECT_F folderButtonRect = { bounds.left, y, bounds.left + 160.0f, y + 34.0f };
    D2D1_ROUNDED_RECT frr = D2D1::RoundedRect(folderButtonRect, 10.0f, 10.0f);
    brush->SetColor(colors.cardFill);
    ctx->FillRoundedRectangle(frr, brush);
    brush->SetColor(colors.cardBorder);
    ctx->DrawRoundedRectangle(frr, brush, 1.0f);
    brush->SetColor(colors.textPrimary);
    static const wchar_t* kChooseFolderLabel = L"Choose Folder\u2026";
    ctx->DrawText(kChooseFolderLabel, static_cast<UINT32>(wcslen(kChooseFolderLabel)), bodyFormat, folderButtonRect, brush,
                  D2D1_DRAW_TEXT_OPTIONS_CLIP);
    m_clickZones.push_back({ folderButtonRect, [this] { if (m_callbacks.onPickPhotoFolder) m_callbacks.onPickPhotoFolder(); } });
    y += 50.0f;

    D2D1_RECT_F rotationLabelRect = { bounds.left, y, bounds.right, y + 18.0f };
    static const wchar_t* kRotationLabel = L"Rotation Interval";
    ctx->DrawText(kRotationLabel, static_cast<UINT32>(wcslen(kRotationLabel)), bodyFormat, rotationLabelRect, brush,
                  D2D1_DRAW_TEXT_OPTIONS_CLIP);
    y += 22.0f;

    static const int kPresets[4] = { 1, 5, 15, 30 };
    static const wchar_t* kPresetLabels[4] = { L"1 min", L"5 min", L"15 min", L"30 min" };
    float btnWidth = (bounds.right - bounds.left - 3.0f * 8.0f) / 4.0f;
    float bx = bounds.left;
    for (int i = 0; i < 4; ++i) {
        D2D1_RECT_F btnRect = { bx, y, bx + btnWidth, y + 30.0f };
        bool selected = (m_settings->photoRotationMinutes == kPresets[i]);
        D2D1_ROUNDED_RECT rr = D2D1::RoundedRect(btnRect, 8.0f, 8.0f);
        brush->SetColor(selected ? colors.accentViolet : colors.cardFill);
        ctx->FillRoundedRectangle(rr, brush);
        brush->SetColor(colors.cardBorder);
        ctx->DrawRoundedRectangle(rr, brush, 1.0f);
        brush->SetColor(selected ? colors.textPrimary : colors.textSecondary);
        ctx->DrawText(kPresetLabels[i], static_cast<UINT32>(wcslen(kPresetLabels[i])), smallFormat, btnRect, brush,
                      D2D1_DRAW_TEXT_OPTIONS_CLIP);

        int minutes = kPresets[i];
        m_clickZones.push_back({ btnRect, [this, minutes] { m_settings->photoRotationMinutes = minutes; NotifyChanged(); } });
        bx += btnWidth + 8.0f;
    }
}

void SettingsPanel::DrawDesktop(ID2D1DeviceContext* ctx, ID2D1SolidColorBrush* brush, IDWriteTextFormat* bodyFormat,
                                 D2D1_RECT_F bounds, const ThemeManager& theme) {
    float y = bounds.top;
    y = DrawToggleRow(ctx, brush, bodyFormat, bounds, y, L"Always on Top", m_alwaysOnTopToggle, m_settings->alwaysOnTop,
                       [this](bool v) { m_settings->alwaysOnTop = v; NotifyChanged(); }, theme);
    DrawToggleRow(ctx, brush, bodyFormat, bounds, y, L"Start with Windows", m_startWithWindowsToggle, m_settings->startWithWindows,
                  [this](bool v) { m_settings->startWithWindows = v; NotifyChanged(); }, theme);
}

void SettingsPanel::DrawPerformance(ID2D1DeviceContext* ctx, ID2D1SolidColorBrush* brush, IDWriteTextFormat* bodyFormat,
                                     D2D1_RECT_F bounds, const ThemeManager& theme) {
    float y = bounds.top;
    DrawToggleRow(ctx, brush, bodyFormat, bounds, y, L"Animations", m_animationsToggle, m_settings->animationsEnabled,
                  [this](bool v) { m_settings->animationsEnabled = v; NotifyChanged(); }, theme);
}

void SettingsPanel::DrawQuickNotes(ID2D1DeviceContext* ctx, ID2D1SolidColorBrush* brush, IDWriteTextFormat* bodyFormat,
                                   IDWriteTextFormat* smallFormat, D2D1_RECT_F bounds, const ThemeManager& theme) {
    const ThemeColors& colors = theme.Colors();
    float y = bounds.top;

    D2D1_RECT_F labelRect = { bounds.left, y, bounds.right, y + 18.0f };
    brush->SetColor(colors.textPrimary);
    static const wchar_t* kAutoLockLabel = L"Auto-lock after";
    ctx->DrawText(kAutoLockLabel, static_cast<UINT32>(wcslen(kAutoLockLabel)), bodyFormat, labelRect, brush,
                  D2D1_DRAW_TEXT_OPTIONS_CLIP);
    y += 22.0f;

    static const int kTimeouts[4] = { 0, 60, 300, 1800 };
    static const wchar_t* kTimeoutLabels[4] = { L"Never", L"1 min", L"5 min", L"30 min" };
    float btnWidth = (bounds.right - bounds.left - 3.0f * 8.0f) / 4.0f;
    float bx = bounds.left;
    for (int i = 0; i < 4; ++i) {
        D2D1_RECT_F btnRect = { bx, y, bx + btnWidth, y + 30.0f };
        bool selected = (m_settings->noteAutoLockSeconds == kTimeouts[i]);
        D2D1_ROUNDED_RECT rr = D2D1::RoundedRect(btnRect, 8.0f, 8.0f);
        brush->SetColor(selected ? colors.accentViolet : colors.cardFill);
        ctx->FillRoundedRectangle(rr, brush);
        brush->SetColor(colors.cardBorder);
        ctx->DrawRoundedRectangle(rr, brush, 1.0f);
        brush->SetColor(selected ? colors.textPrimary : colors.textSecondary);
        ctx->DrawText(kTimeoutLabels[i], static_cast<UINT32>(wcslen(kTimeoutLabels[i])), smallFormat, btnRect, brush,
                      D2D1_DRAW_TEXT_OPTIONS_CLIP);

        int seconds = kTimeouts[i];
        m_clickZones.push_back({ btnRect, [this, seconds] { m_settings->noteAutoLockSeconds = seconds; NotifyChanged(); } });
        bx += btnWidth + 8.0f;
    }
    y += 44.0f;

    y = DrawToggleRow(ctx, brush, bodyFormat, bounds, y, L"Lock when window loses focus",
                       m_lockOnFocusLossToggle, m_settings->lockNotesOnFocusLoss,
                       [this](bool v) { m_settings->lockNotesOnFocusLoss = v; NotifyChanged(); }, theme);

    D2D1_RECT_F noteRect = { bounds.left, y + 6.0f, bounds.right, y + 60.0f };
    brush->SetColor(colors.textMuted);
    static const wchar_t* kCryptoNote =
        L"Notes are encrypted with Windows DPAPI, tied to your Windows account. "
        L"The password is never stored \u2014 only a salted hash of it.";
    ctx->DrawText(kCryptoNote, static_cast<UINT32>(wcslen(kCryptoNote)), smallFormat, noteRect, brush,
                  D2D1_DRAW_TEXT_OPTIONS_CLIP);
}

float SettingsPanel::DrawEditableTextRow(ID2D1DeviceContext* ctx, ID2D1SolidColorBrush* brush, IDWriteTextFormat* bodyFormat,
                                          IDWriteTextFormat* smallFormat, D2D1_RECT_F bounds, float y,
                                          const std::wstring& label, const std::wstring& committedValue,
                                          const std::wstring& placeholder, int fieldId, const ThemeManager& theme) {
    const ThemeColors& colors = theme.Colors();

    D2D1_RECT_F labelRect = { bounds.left, y, bounds.right, y + 18.0f };
    brush->SetColor(colors.textSecondary);
    ctx->DrawText(label.c_str(), static_cast<UINT32>(label.size()), smallFormat, labelRect, brush,
                  D2D1_DRAW_TEXT_OPTIONS_CLIP);
    y += 22.0f;

    bool isEditingThis = (m_editingField == fieldId);
    D2D1_RECT_F boxRect = { bounds.left, y, bounds.right, y + 38.0f };
    D2D1_ROUNDED_RECT rr = D2D1::RoundedRect(boxRect, 8.0f, 8.0f);
    brush->SetColor(colors.cardFill);
    ctx->FillRoundedRectangle(rr, brush);
    // A distinct border color is the whole "this box is live" cue -- see
    // this function's own header comment on why not a measured caret.
    brush->SetColor(isEditingThis ? colors.accentViolet : colors.cardBorder);
    ctx->DrawRoundedRectangle(rr, brush, isEditingThis ? 1.5f : 1.0f);

    D2D1_RECT_F textRect = { boxRect.left + 12.0f, boxRect.top, boxRect.right - 12.0f, boxRect.bottom };
    if (isEditingThis) {
        // Trailing "|" stands in for a caret without needing a text-
        // measurement pass (DrawEditableTextRow has no IDWriteFactory to
        // do that with, unlike TodoWidget's equivalent) -- still a clear,
        // immediate "you're typing here" signal.
        std::wstring shown = m_editBuffer + L"|";
        brush->SetColor(colors.textPrimary);
        ctx->DrawText(shown.c_str(), static_cast<UINT32>(shown.size()), bodyFormat, textRect, brush,
                      D2D1_DRAW_TEXT_OPTIONS_CLIP);
    } else if (!committedValue.empty()) {
        brush->SetColor(colors.textPrimary);
        ctx->DrawText(committedValue.c_str(), static_cast<UINT32>(committedValue.size()), bodyFormat, textRect, brush,
                      D2D1_DRAW_TEXT_OPTIONS_CLIP);
    } else {
        brush->SetColor(colors.textMuted);
        ctx->DrawText(placeholder.c_str(), static_cast<UINT32>(placeholder.size()), bodyFormat, textRect, brush,
                      D2D1_DRAW_TEXT_OPTIONS_CLIP);
    }

    m_clickZones.push_back({ boxRect, [this, fieldId, committedValue] {
        if (m_editingField == fieldId) return; // already editing this one -- clicking again shouldn't reset the buffer
        m_editingField = fieldId;
        m_editBuffer = committedValue;
    } });

    return y + 38.0f + 16.0f; // box height + gap before the next row
}

void SettingsPanel::DrawDateGreeting(ID2D1DeviceContext* ctx, ID2D1SolidColorBrush* brush, IDWriteTextFormat* bodyFormat,
                                      IDWriteTextFormat* smallFormat, D2D1_RECT_F bounds, const ThemeManager& theme) {
    const ThemeColors& colors = theme.Colors();
    float y = bounds.top;

    y = DrawEditableTextRow(ctx, brush, bodyFormat, smallFormat, bounds, y,
                             L"Your name", m_settings->displayName, L"there", /*fieldId*/ 0, theme);
    y = DrawEditableTextRow(ctx, brush, bodyFormat, smallFormat, bounds, y,
                             L"Motivational line", m_settings->motivationText,
                             L"Keep going, great things take time.", /*fieldId*/ 1, theme);

    D2D1_RECT_F noteRect = { bounds.left, y + 4.0f, bounds.right, y + 56.0f };
    brush->SetColor(colors.textMuted);
    static const wchar_t* kEditNote =
        L"Click a box to edit, Enter to save, Escape to cancel. "
        L"Leave a box empty to use the default shown as its placeholder.";
    ctx->DrawText(kEditNote, static_cast<UINT32>(wcslen(kEditNote)), smallFormat, noteRect, brush,
                  D2D1_DRAW_TEXT_OPTIONS_CLIP);
}
//draw about code--
void SettingsPanel::DrawAbout(ID2D1DeviceContext* ctx, ID2D1SolidColorBrush* brush, IDWriteTextFormat* bodyFormat,
                              IDWriteTextFormat* smallFormat, D2D1_RECT_F bounds, const ThemeManager& theme) {
    const ThemeColors& colors = theme.Colors();
    float y = bounds.top;

    // Enable word wrapping on formats
    smallFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);
    bodyFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);

    // Main App Title + Version
    static const wchar_t* kName = L"Mosaic  \u2014  Version 1.0.0";
    D2D1_RECT_F nameRect = { bounds.left, y, bounds.right, y + 22.0f };
    brush->SetColor(colors.textPrimary);
    ctx->DrawText(kName, static_cast<UINT32>(wcslen(kName)), bodyFormat, nameRect, brush,
                  D2D1_DRAW_TEXT_OPTIONS_CLIP);
    y += 28.0f;

    // Subtitle / Intro tagline
    static const wchar_t* kIntro =
        L"A minimal, adaptive desktop workspace that keeps your tasks, notes, and reminders "
        L"together in one calm place on your desktop.";
    D2D1_RECT_F introRect = { bounds.left, y, bounds.right, y + 36.0f };
    brush->SetColor(colors.textSecondary);
    ctx->DrawText(kIntro, static_cast<UINT32>(wcslen(kIntro)), smallFormat, introRect, brush,
                  D2D1_DRAW_TEXT_OPTIONS_CLIP);
    y += 42.0f;

    // Section drawer (heading in smallFormat + textSecondary, body in smallFormat + textMuted)
    auto drawSection = [&](const wchar_t* heading, const wchar_t* text, float textHeight) {
        D2D1_RECT_F headRect = { bounds.left, y, bounds.right, y + 18.0f };
        brush->SetColor(colors.textSecondary);
        ctx->DrawText(heading, static_cast<UINT32>(wcslen(heading)), smallFormat, headRect, brush,
                      D2D1_DRAW_TEXT_OPTIONS_CLIP);
        y += 20.0f;

        D2D1_RECT_F textRect = { bounds.left, y, bounds.right, y + textHeight };
        brush->SetColor(colors.textMuted);
        ctx->DrawText(text, static_cast<UINT32>(wcslen(text)), smallFormat, textRect, brush,
                      D2D1_DRAW_TEXT_OPTIONS_CLIP);
        y += textHeight + 14.0f;
    };

    drawSection(
        L"Private by design",
        L"Everything is stored locally on this PC. Mosaic has no account and no cloud sync. "
        L"Quick Notes can be locked and are encrypted with Windows data protection.",
        36.0f);

    drawSection(
        L"Lightweight",
        L"Built natively for Windows with Win32, Direct2D and DirectComposition, with no "
        L"browser runtime, so it stays light on CPU and memory.",
        36.0f);

    drawSection(
        L"Storage",
        L"Includes SQLite (public domain) for local storage.",
        18.0f);

    // Author Credits
    y += 4.0f;
    static const wchar_t* kAuthor = L"Built by \u2014 Naba Krishna Hazarika";
    D2D1_RECT_F authorRect = { bounds.left, y, bounds.right, y + 20.0f };
    brush->SetColor(colors.accentViolet);
    ctx->DrawText(kAuthor, static_cast<UINT32>(wcslen(kAuthor)), smallFormat, authorRect, brush,
                  D2D1_DRAW_TEXT_OPTIONS_CLIP);
}
//drawgeneral code --------------------------------------on date 28-09-2026
void SettingsPanel::DrawGeneral(ID2D1DeviceContext* ctx, ID2D1SolidColorBrush* brush, IDWriteTextFormat* bodyFormat,
                                 IDWriteTextFormat* smallFormat, D2D1_RECT_F bounds, const ThemeManager& theme) {
    const ThemeColors& colors = theme.Colors();
    float y = bounds.top;

    // Enable word wrapping
    smallFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);
    bodyFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);

    static const wchar_t* kTitle = L"Welcome to Mosaic";
    D2D1_RECT_F titleRect = { bounds.left, y, bounds.right, y + 22.0f };
    brush->SetColor(colors.textPrimary);
    ctx->DrawText(kTitle, static_cast<UINT32>(wcslen(kTitle)), bodyFormat, titleRect, brush,
                  D2D1_DRAW_TEXT_OPTIONS_CLIP);
    y += 30.0f;

    // Draws one paragraph and moves y below it with a consistent gap
    auto drawParagraph = [&](const wchar_t* text, const D2D1_COLOR_F& color) {
        float height = 18.0f; // Uniform height for a single line
        D2D1_RECT_F rect = { bounds.left, y, bounds.right, y + height };
        brush->SetColor(color);
        ctx->DrawText(text, static_cast<UINT32>(wcslen(text)), smallFormat, rect, brush,
                      D2D1_DRAW_TEXT_OPTIONS_CLIP);
        y += height + 10.0f; // Consistent 10px gap between lines
    };

    drawParagraph(
        L"Use the categories on the left to personalise your workspace.",
        colors.textSecondary);

    drawParagraph(
        L"Appearance & Widget Layout control how Mosaic looks and where your widgets sit.",
        colors.textMuted);

    drawParagraph(
        L"Desktop controls startup and window behavior.",
        colors.textMuted);

    drawParagraph(
        L"Quick Notes controls locking and privacy for your notes.",
        colors.textMuted);

    drawParagraph(
        L"Your changes are saved automatically and restored the next time Mosaic opens.",
        colors.textMuted);

    drawParagraph(
        L"Your data is stored locally in your user profile.",
        colors.textMuted);
}
//draw data and priavcy section---------------------------------------
void SettingsPanel::DrawPrivacy(ID2D1DeviceContext* ctx, ID2D1SolidColorBrush* brush, IDWriteTextFormat* bodyFormat,
                                IDWriteTextFormat* smallFormat, D2D1_RECT_F bounds, const ThemeManager& theme) {
    const ThemeColors& colors = theme.Colors();
    float y = bounds.top;

    smallFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);
    bodyFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);

    auto drawSection = [&](const wchar_t* heading, const wchar_t* text, float textHeight) {
        // Section Header: uses smallFormat and textSecondary
        D2D1_RECT_F headRect = { bounds.left, y, bounds.right, y + 18.0f };
        brush->SetColor(colors.textSecondary);
        ctx->DrawText(heading, static_cast<UINT32>(wcslen(heading)), smallFormat, headRect, brush,
                      D2D1_DRAW_TEXT_OPTIONS_CLIP);
        y += 20.0f;

        // Paragraph Body: uses smallFormat and textMuted
        D2D1_RECT_F textRect = { bounds.left, y, bounds.right, y + textHeight };
        brush->SetColor(colors.textMuted);
        ctx->DrawText(text, static_cast<UINT32>(wcslen(text)), smallFormat, textRect, brush,
                      D2D1_DRAW_TEXT_OPTIONS_CLIP);
        y += textHeight + 14.0f;
    };

    drawSection(
        L"Stays on this PC",
        L"Mosaic stores your tasks, pins, activities, notes & settings in a single local database "
        L"in your user profile. It has no account & cloud sync, and it doesn't connect to the "
        L"internet.",
        48.0f);

    drawSection(
        L"Quick Notes protection",
        L"Notes are encrypted with Windows data protection before they are saved. A copy of the "
        L"database taken to another PC or opened under a different Windows account cannot be read. "
        L"The optional password only controls access inside Mosaic and is never stored, only a "
        L"salted hash of it.",
        68.0f);

    drawSection(
        L"What this does not cover",
        L"Notes are tied to your Windows account, not to the password. Someone already signed in "
        L"to your account could still open them. Use a strong Windows sign-in and lock your PC "
        L"when away.",
        50.0f);

    drawSection(
        L"Your photos",
        L"Mosaic only reads images from the folder you choose. It never moves, edits or uploads them.",
        18.0f);

    drawSection(
        L"Your data folder",
        L"Everything is in %LOCALAPPDATA%\\Mosaic\\mosaic.db. Close Mosaic and delete that file "
        L"to erase all Mosaic data. This cannot be undone.",
        36.0f);
}
// --- input dispatch --------------------------------------------------------

bool SettingsPanel::OnMouseMove(D2D1_POINT_2F pt) {
    if (!m_open) return false;

    m_closeButton.SetHovered(m_closeButton.HitTest(pt));
    for (auto& binding : m_toggleBindings) {
        binding.toggle->SetHovered(binding.toggle->HitTest(pt));
    }
    if (m_activeSlider) {
        m_activeSlider->SetHovered(m_activeSlider->HitTest(pt));
        if (m_draggingSlider) {
            m_activeSlider->SetValueFromPointerX(pt.x);
            if (m_activeSliderOnChange) m_activeSliderOnChange(m_activeSlider->Value());
        }
    }

    // The panel is an occasional-use, modal-style surface (not the
    // always-on dashboard) — repainting on every move while it's open,
    // rather than tracking exact hover-state deltas, is a reasonable,
    // deliberately simple trade-off given that cost only exists while
    // Settings is literally open.
    return true;
}

bool SettingsPanel::OnLButtonDown(D2D1_POINT_2F pt) {
    if (!m_open) return false;

    // Any click at all -- the close button, a different category, a
    // toggle, another field's box -- commits whatever's being edited
    // first, matching TodoWidget's click-away-commits behavior (see
    // TodoWidget::OnLButtonDown) rather than silently discarding typed
    // text on an incidental click elsewhere in the panel.
    if (m_editingField != -1) {
        CommitEditingField();
    }

    if (m_closeButton.HitTest(pt)) {
        Close();
        return true;
    }

    if (m_activeSlider && m_activeSlider->HitTest(pt)) {
        m_draggingSlider = true;
        m_activeSlider->SetDragging(true);
        m_activeSlider->SetValueFromPointerX(pt.x);
        if (m_activeSliderOnChange) m_activeSliderOnChange(m_activeSlider->Value());
        return true;
    }

    for (auto& binding : m_toggleBindings) {
        if (binding.toggle->HitTest(pt)) {
            bool newValue = binding.toggle->Flip();
            if (binding.onChange) binding.onChange(newValue);
            return true;
        }
    }

    for (auto& zone : m_clickZones) {
        if (pt.x >= zone.rect.left && pt.x <= zone.rect.right && pt.y >= zone.rect.top && pt.y <= zone.rect.bottom) {
            if (zone.onClick) zone.onClick();
            return true;
        }
    }

    // Clicking inside the panel but not on anything interactive is simply
    // absorbed. Clicking the scrim outside the panel closes it — the
    // standard "click outside to dismiss" affordance.
    bool insidePanel = pt.x >= m_panelBounds.left && pt.x <= m_panelBounds.right &&
                        pt.y >= m_panelBounds.top && pt.y <= m_panelBounds.bottom;
    if (!insidePanel) {
        Close();
    }
    return true;
}

bool SettingsPanel::OnLButtonUp(D2D1_POINT_2F /*pt*/) {
    if (!m_open) return false;
    if (m_draggingSlider) {
        m_draggingSlider = false;
        if (m_activeSlider) m_activeSlider->SetDragging(false);
    }
    return true;
}

bool SettingsPanel::OnKeyDown(unsigned int virtualKey) {
    if (!m_open) return false;

    if (m_editingField != -1) {
        // While a text field is being edited, Escape/Enter apply to the
        // edit itself, not the whole panel -- exactly TodoWidget's
        // Escape-cancels/Enter-commits split (see TodoWidget::OnKeyDown),
        // which is the only other place in Mosaic Escape already has this
        // more specific meaning before falling back to closing something.
        if (virtualKey == VK_ESCAPE) {
            m_editingField = -1;
            m_editBuffer.clear();
            return true;
        }
        if (virtualKey == VK_RETURN) {
            CommitEditingField();
            return true;
        }
        if (virtualKey == VK_BACK) {
            if (!m_editBuffer.empty()) m_editBuffer.pop_back();
            return true;
        }
        return true; // absorb everything else too -- no other key has a meaning mid-edit
    }

    if (virtualKey == VK_ESCAPE) {
        Close();
    }
    return true; // absorb all keys while open so none leak to a focused dashboard widget underneath
}

bool SettingsPanel::OnChar(wchar_t ch) {
    if (!m_open || m_editingField == -1) return false;
    // Filter out control characters Enter/Backspace/Escape send through
    // WM_CHAR too (\r, \b, 0x1B) -- OnKeyDown already handles those by
    // virtual-key above, so letting them through here would double-handle
    // Enter/Backspace or insert a literal control character into the name.
    if (ch == L'\r' || ch == L'\b' || ch == 0x1B) return true;
    // A generous but real ceiling -- long enough for any plausible name or
    // one-line motivational message, short enough that DrawEditableTextRow
    // (which doesn't wrap or scroll text) never has to truncate visually.
    if (m_editBuffer.size() < 120) {
        m_editBuffer.push_back(ch);
    }
    return true;
}

void SettingsPanel::CommitEditingField() {
    if (m_editingField == 0) {
        m_settings->displayName = m_editBuffer;
    } else if (m_editingField == 1) {
        m_settings->motivationText = m_editBuffer;
    }
    m_editingField = -1;
    m_editBuffer.clear();
    NotifyChanged();
}

} // namespace mosaic::ui