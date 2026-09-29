# Mosaic Manual

This manual describes how to use Mosaic and how its source code is organized.
It reflects the current implementation; settings or features marked as
placeholders are not functional controls yet.

## 1. Project Overview

Mosaic is a native Windows desktop dashboard written in C++20. It uses Win32
for the window and message loop, Direct2D and DirectComposition for drawing,
Direct3D 11 for the graphics device, and SQLite for local data. It does not
embed a browser or use a cross-platform UI framework.

The dashboard is designed to sit on the desktop behind ordinary application
windows. A system-tray icon provides access to show/hide, Settings, pause photo
rotation, and quit commands.

## 2. User Guide

### First Launch

Mosaic creates its database under `%LOCALAPPDATA%\Mosaic\mosaic.db`. By default,
the To Do, Photo, and Pinned widgets are enabled. Special Activity and Quick
Notes can be enabled in **Settings > Widget Layout**. Photos are read from the
Windows Pictures folder until another folder is chosen.

### Dashboard Widgets

| Widget | How to use it |
|---|---|
| **To Do** | Select **+** to add a task. Click its checkbox to mark it complete. Double-click a task to rename it. Hover a row to reveal its delete button. Press Enter to commit text or Escape to cancel an edit. |
| **Special Activity** | Select **+**, enter a title, then enter a date and time. Accepted dates include `today 6:00 pm`, `tomorrow 18:00`, and `2026-09-30 11:59 pm`. Reminders appear as Windows notifications; the app checks for due reminders about once a minute. |
| **Photo** | Displays photos found in the selected folder and its subfolders. The list is shuffled and cycled so each discovered photo is shown before the order repeats. Choose a different folder or rotation interval in Settings. |
| **Pinned** | Add, edit, and delete text labels. Pinned entries are labels only; they are not currently linked to files or URLs. |
| **Quick Notes** | Unlock with the password you set, or use Windows Hello when Windows reports it is available. The note is saved when it is locked. Settings control automatic locking and locking when the window loses focus. |

Widgets can be dragged to new grid positions. The grid avoids overlap, compacts
gaps, and saves positions between launches. Widgets cannot currently be resized.

### Settings Panel

Open Settings from the tray menu. Changes are saved automatically; there is no
Save button. Close with the X button, press Escape, or click outside the panel.
When editing a text field, Enter saves, Escape cancels, and clicking elsewhere
commits the edit.

| Category | Current behavior |
|---|---|
| **Appearance** | Adjust card transparency, corner radius, accent color, background blur, and widget shadow. |
| **Widget Layout** | Enable or disable each widget and reset saved positions to the default layout. Disabled widgets do not occupy grid space. |
| **Photo & Media** | Select a local photo folder and set rotation to 1, 5, 15, or 30 minutes. |
| **Date & Greeting** | Edit the displayed name and motivational line. Empty values use the built-in defaults. |
| **To Do** | This Settings category is a placeholder. The To Do dashboard widget itself works. |
| **Special Activity** | This Settings category is a placeholder. The Special Activity dashboard widget itself works. |
| **Quick Notes** | Set automatic lock to Never, 1, 5, or 30 minutes, and choose whether losing window focus locks the note. |
| **Desktop** | Control whether Mosaic starts with Windows. The **Always on Top** control is currently present but has no effect; the dashboard is kept behind normal windows. |
| **Performance** | Turn layout and photo transition animations on or off. |
| **Privacy & Data** | Displays information about local storage and the limits of note protection. |
| **General** | Displays introductory information; it has no settings controls. |
| **About** | Displays project details, version, and credits. |

### Notes, Privacy, and Data

Tasks, activities, pinned labels, layout positions, and settings are stored in
the local SQLite database. Mosaic reads photos from the folder you select; it
does not move or upload them. There is no account or cloud-sync feature.

Quick Notes are encrypted at rest with Windows DPAPI and are tied to the
current Windows account. The optional password controls access in Mosaic; it is
not the DPAPI encryption key. Mosaic stores a salted PBKDF2-SHA256 password
hash, not the password. A database copy cannot be decrypted under a different
Windows account, but DPAPI does not protect notes from code already running as
the signed-in account. Windows Hello, when available, uses the actual Windows
authentication prompt.

To erase Mosaic's local data, close the app and delete
`%LOCALAPPDATA%\Mosaic\mosaic.db`. This removes saved tasks, activities, pinned
items, layout, settings, and notes; it cannot be undone. Deleting the database
does not delete photos in your selected photo folder.

## 3. Developer Guide

### Source Layout

| Path | Responsibility |
|---|---|
| `src/main.cpp` | Windows application entry point. |
| `src/app/` | Application startup and in-memory settings model. |
| `src/platform/` | Win32 window, message handling, desktop behavior, and tray icon. |
| `src/ui/` | Direct2D graphics resources, theme, dashboard, and Settings panel. |
| `src/ui/components/` | Reusable controls and card/panel drawing helpers. |
| `src/widgets/` | Dashboard widget implementations and widget registry. |
| `src/layout/` | Grid placement and compaction logic, independent of rendering. |
| `src/data/` | SQLite database, models, and repositories. |
| `src/media/` | Local photo discovery and asynchronous image decoding. |
| `src/security/` | DPAPI note encryption and Windows Hello integration. |
| `src/notifications/` | Windows toast notifications for activities. |
| `tests/` | Unit tests for layout logic and activity date/time parsing. |
| `third_party/sqlite/` | Vendored SQLite amalgamation. |

### Runtime Flow

1. `Application::Run` configures per-monitor DPI awareness and initializes COM.
2. `Window` creates the Win32 window, opens the local database, loads settings,
   initializes repositories and widgets, then starts the message loop.
3. Window messages route input to the Settings panel or dashboard. Paint events
   call `DashboardView`, which places and renders enabled widgets.
4. Widgets use repositories for persistent data rather than issuing SQLite
   queries directly. The Settings panel updates `AppSettings`; a callback saves
   changes, reapplies theme/platform settings, rebuilds layout when needed, and
   requests a repaint.
5. Photo decoding runs on a worker thread. Completion is posted to the window;
   Direct2D/GPU resources are then updated on the UI thread.

Rendering uses a D3D11 device, a Direct2D device context, a composition swap
chain, and DirectComposition. Most redraws are requested by state changes. A
minute timer handles routine date/reminder/photo work; a temporary timer runs
only while a photo or layout transition is active.

### Settings Panel Design

`SettingsPanel` is an overlay drawn by `DashboardView`, not a separate native
window. It owns the panel's drawing and hit-test zones, while `Window` supplies
callbacks for operations requiring platform access, such as opening the photo
folder picker or changing startup registration. `AppSettings` holds current
values; `SettingsRepository` persists typed key/value settings in SQLite.

When adding a setting, update its default in `AppSettings`, load/save it in
`AppSettings.cpp`, draw and handle it in `SettingsPanel`, and apply it from the
Window callback if it affects platform behavior. Keep platform APIs out of the
panel itself; use a callback like the existing photo-folder and reset-layout
callbacks.

### Adding a Widget

1. Add a `WidgetId` and implement `IWidget`, including metadata, rendering, and
   any input hooks the widget needs. Widget render bounds are in DIPs.
2. Register its factory in `WidgetManager::Initialize`.
3. Add it to `DashboardView::RebuildLayout`'s widget order and integrate its
   visibility setting in `AppSettings` and `SettingsPanel` if it should be
   user-configurable.
4. For persistent data, add a model/repository backed by `Database` rather than
   calling SQLite directly from the widget.
5. Add unit tests for pure logic; keep Win32/Direct2D work on the UI thread.

### Build and Test

Requirements: Windows, an MSVC C++ toolchain with the Windows SDK, CMake 3.20+
and Ninja. The application uses C++20. VS Code and its CMake Tools/C++
extensions are optional ways to drive the build.

Build a Debug version:

```powershell
cmake --preset ninja-debug
cmake --build --preset ninja-debug
```

Build and run the unit tests (the first configure may fetch GoogleTest):

```powershell
cmake --preset ninja-tests
cmake --build --preset ninja-tests
ctest --preset ninja-tests
```

The Ninja Debug executable is written to `build/bin/Mosaic.exe`. SQLite is
used from `third_party/sqlite/` when its amalgamation files are present;
otherwise CMake attempts to fetch it during configuration.

## Drawbacks

1. Due to grid system the customizable for the box(pinned,todo,photo etc) some time places at very incorrectly.
2. now this application(**mosiac.exe**) take 13MB to 22MB ram, we  can reduce to less than 10MB by optimizing the image pipeline
3. Also we can make more globalise functions and methods
4. there is no security for .exe file. if some get the user's .exe file they can get the window hello fingerprint by reverse engineering.
5. i didn't check for the microsoft security.so may it show 
    ```
    Windows Defender SmartScreen: A blue or grey pop-up saying "Windows protected your PC" with a message that an unrecognized app started and might put your PC at risk. This happens because the developer did not sign the .exe with a verified digital certificate.
    ```
