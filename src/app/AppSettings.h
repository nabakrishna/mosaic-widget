
//new code for some feature modification on date 27-09-2026
#pragma once
#include "data/SettingsRepository.h"
#include "widgets/IWidget.h"
#include "ui/Theme.h"

namespace mosaic::app {


struct AppSettings {
    // --- Appearance ---------------------------------------------------
    float transparency = 0.55f;      // maps directly to ThemeColors::cardFill.a
    int cornerRadiusPreset = 2;      // 0=Sharp 1=Small 2=Medium 3=Large
    int accentPreset = 0;            // index into SettingsPanel's fixed palette
    bool strongShadow = false;
    bool blurEnabled = true;         // DWM system backdrop on/off

    // --- Widget Layout --------------------------------------------------
    //for the first tim euser will only the photo,todo and pinned widget box
    bool widgetEnabled[5] = { true, true, false, true, false }; // indexed by WidgetId

    // --- Photo & Media --------------------------------------------------
    int photoRotationMinutes = 5;

    // --- Desktop ---------------------------------------------------
    bool alwaysOnTop = false;
    bool startWithWindows = false;

    // --- Quick Notes ---------------------------------------------
    // Seconds of inactivity before notes re-lock; 0 means "Never".
    int noteAutoLockSeconds = 300;
    bool lockNotesOnFocusLoss = true;

    // --- Performance -----------------------------------------------
    bool animationsEnabled = true;

    // --- Date & Greeting -------------------------------------------
    // Empty by default: an empty displayName means Window::UpdateHeaderData
    // falls back to its own default rather than ever showing a literal
    // blank name, matching how a genuinely first-run setting should behave
    // (see LoadFrom/SaveTo and Window.cpp's UpdateHeaderData).
    std::wstring displayName;
    std::wstring motivationText;

    bool IsWidgetEnabled(widgets::WidgetId id) const {
        int index = static_cast<int>(id);
        return (index >= 0 && index < 5) ? widgetEnabled[index] : true;
    }
    void SetWidgetEnabled(widgets::WidgetId id, bool enabled) {
        int index = static_cast<int>(id);
        if (index >= 0 && index < 5) widgetEnabled[index] = enabled;
    }

    // Loads every value from `repo`, falling back to the struct's own
    // defaults (already set above) for anything never saved before —
    // exactly the "fresh install" case.
    void LoadFrom(data::SettingsRepository& repo);
    void SaveTo(data::SettingsRepository& repo) const;
};

void ApplyThemeSettings(const AppSettings& settings, ui::ThemeManager& theme);

} // namespace mosaic::app
