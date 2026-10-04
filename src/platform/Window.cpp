//fouth code fo rthis file for new modification in feratures on  27-09-2026
#include "platform/Window.h"
#include "notifications/ToastNotifier.h"
#include "widgets/ActivityDateTime.h"
#include "security/WindowsHello.h"
#include <shlobj.h>
#include <shobjidl.h>
#include <wrl/client.h>
#include <dwmapi.h>
#include <shellscalingapi.h>
#include <windowsx.h>
#include <ctime>
#include <string>
#include <sstream>
#include <cwchar>
#include <cstddef>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "shcore.lib")

// DWMWA_SYSTEMBACKDROP_TYPE and DWM_SYSTEMBACKDROP_TYPE were added in the
// Windows 11 22H2 SDK. Guard them so this still compiles against slightly
// older Windows SDKs; the feature simply becomes a no-op there.
#ifndef DWMWA_SYSTEMBACKDROP_TYPE
#define DWMWA_SYSTEMBACKDROP_TYPE 38
// typedef enum {
//     DWMSBT_AUTO = 0,
//     DWMSBT_NONE = 1,
//     DWMSBT_MAINWINDOW = 2,     // Mica
//     DWMSBT_TRANSIENTWINDOW = 3, // Acrylic
//     DWMSBT_TABBEDWINDOW = 4,
// } DWM_SYSTEMBACKDROP_TYPE;
#endif
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

namespace mosaic::platform {

namespace {
constexpr wchar_t kWindowClassName[] = L"MosaicDashboardWindow";
constexpr int kDefaultWidthDip = 940;
constexpr int kDefaultHeightDip = 520;

constexpr bool kAttemptWorkerWReparent = false;
BOOL CALLBACK FindWorkerWProc(HWND hwnd, LPARAM lParam) {
    HWND shellView = FindWindowExW(hwnd, nullptr, L"SHELLDLL_DefView", nullptr);
    if (shellView) {
        HWND* out = reinterpret_cast<HWND*>(lParam);
        *out = FindWindowExW(nullptr, hwnd, L"WorkerW", nullptr);
        return FALSE; // found it -- stop enumerating
    }
    return TRUE;
}

} // namespace

Window::~Window() {
    if (m_foregroundHook) UnhookWinEvent(m_foregroundHook);
    if (m_dashboardView) m_dashboardView->ReleaseDeviceResources();
    if (m_hwnd) DestroyWindow(m_hwnd);
}

LRESULT CALLBACK Window::WndProcThunk(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    Window* self = nullptr;
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        self = reinterpret_cast<Window*>(cs->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->m_hwnd = hwnd;
    } else {
        self = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }
    if (self) return self->HandleMessage(msg, wParam, lParam);
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT Window::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    // TaskbarCreated has no fixed numeric value (it's resolved once via
    // RegisterWindowMessageW in Create()), so it can't be a switch `case`
    // label -- it's checked separately, here, before the switch. Explorer
    // broadcasts it to every top-level window whenever it (re)starts
    // (crash, manual restart, update). Our WorkerW desktop-pinning parent
    // and Shell_NotifyIcon registration are both Explorer-side state that
    // a restart silently drops without telling this window through any
    // other message, so both are redone exactly as at first launch.
    if (msg == m_taskbarCreatedMsg && m_taskbarCreatedMsg != 0) {
        TryReparentToDesktopWorkerW();
        PinToDesktopBottom();
        m_trayIcon.Destroy();
        m_trayIcon.Create(m_hwnd, L"Mosaic");
        return 0;
    }

    switch (msg) {
    case WM_PAINT:
        OnPaint();
        ValidateRect(m_hwnd, nullptr); // we drew via D2D, not GDI BeginPaint/EndPaint
        return 0;

    case WM_SIZE:
        OnResize(LOWORD(lParam), HIWORD(lParam));
        return 0;

    case WM_DPICHANGED:
        OnDpiChanged(HIWORD(wParam), reinterpret_cast<RECT*>(lParam));
        return 0;


    case WM_SHOWWINDOW:
        // NOTE: a real message trace captured on a Windows 11 24H2 machine
        // (via a temporary diagnostic logging block, since removed) proved
        // this handler's original premise wrong: Win+D / the 3-finger
        // swipe-down never sends WM_SHOWWINDOW at all on that build --
        // IsWindowVisible() stayed TRUE throughout, and the actual sequence
        // was WM_ACTIVATE(WA_INACTIVE) then WM_ACTIVATEAPP(FALSE), handled
        // separately below. So this handler is not what fixes that gesture
        // -- it's kept as a real, working safety net for the different,
        // rarer case where something genuinely does call ShowWindow(HIDE)
        // on Mosaic from outside (some Explorer builds/configurations, or
        // future ones, may still do this the documented way).
        if (wParam == FALSE && lParam != SW_PARENTCLOSING && m_dashboardVisible) {
            PostMessage(m_hwnd, WM_MOSAIC_REASSERT_VISIBLE, 0, 0);
        }
        return DefWindowProc(m_hwnd, msg, wParam, lParam);

    case WM_MOSAIC_REASSERT_VISIBLE:
        if (m_dashboardVisible) {
            ShowWindow(m_hwnd, SW_SHOWNA);
            PinToDesktopBottom();
        }
        return 0;




    case WM_TIMER:
        // id 1: once a minute is plenty for a greeting/date string and an
        // activity-reminder check -- this is the event-driven philosophy
        // from spec section 36 applied literally: we redraw because
        // something *could* have changed, not on a tight render loop. It
        // also doubles as the photo rotation clock (see
        // m_settings.photoRotationMinutes) rather than running a third OS
        // timer for something that's naturally expressible in whole
        // minutes.
        // id 2: the short-lived crossfade animation timer -- see
        // StartTransitionTimer's comment in Window.h.
        if (wParam == 1) {
            UpdateHeaderData();
            CheckActivityReminders();

            if (m_quickNotesWidget && m_quickNotesWidget->TickAutoLock()) {
                InvalidateRect(m_hwnd, nullptr, FALSE);
            }

            if (!m_photosPaused && ++m_minutesSinceLastPhoto >= m_settings.photoRotationMinutes) {
                m_minutesSinceLastPhoto = 0;
                if (m_photoWidget && m_graphics) {
                    m_photoWidget->RequestNextPhoto(m_graphics->DeviceContext());
                }
            }
            InvalidateRect(m_hwnd, nullptr, FALSE);
        } else if (wParam == 2) {
            InvalidateRect(m_hwnd, nullptr, FALSE);
            bool photoDone = !m_photoWidget || !m_photoWidget->IsTransitioning();
            bool layoutDone = !m_dashboardView || !m_dashboardView->IsAnimating();
            if (photoDone && layoutDone) {
                StopTransitionTimer();
            }
        }
        return 0;

    // --- Desktop pinning: Z-order enforcement -------------------------
    //
    // Activation (who receives keyboard input) and Z-order (front-to-back
    // stacking) are independent in Win32 -- a window can be the active,
    // focused window while still sitting at the very bottom of the
    // Z-order. That's exploited here: we do NOT use WS_EX_NOACTIVATE (it
    // would silently break WM_CHAR delivery to Quick Notes/To Do, since
    // keyboard input routes to the foreground thread's focus window, and
    // a window that's never foreground doesn't reliably get it even after
    // SetFocus()). Instead, every attempt to move us up the Z-order is
    // intercepted and rewritten to HWND_BOTTOM -- so clicking into the
    // dashboard still activates it (typing works normally), but it's
    // immediately pushed behind every other window again afterward.
    case WM_WINDOWPOSCHANGING: {
        auto* wp = reinterpret_cast<WINDOWPOS*>(lParam);
        if (!(wp->flags & SWP_NOZORDER)) {
            // Only rewrite the Z-order component -- leave position/size
            // changes (e.g. OnDpiChanged's SetWindowPos, which already
            // passes SWP_NOZORDER and so never reaches here) untouched.
            wp->hwndInsertAfter = HWND_BOTTOM;
        }
        return 0;
    }

    case WM_ACTIVATE:
    case WM_ACTIVATEAPP:
        // Real trace from a Windows 11 24H2 machine (captured via a
        // temporary diagnostic block) showed the actual Win+D / 3-finger
        // swipe-down sequence for this window is exactly: WM_ACTIVATE
        // (WA_INACTIVE) then WM_ACTIVATEAPP(FALSE) -- no WM_SHOWWINDOW at
        // all, no WM_SYSCOMMAND/SC_MINIMIZE, IsWindowVisible() staying
        // TRUE throughout. That rules out every message-based hide theory
        // this file previously handled (WM_SHOWWINDOW) -- Explorer isn't
        // hiding Mosaic, it's just raising its own desktop surface above
        // it while deactivating Mosaic in the process, and Mosaic was
        // only re-pinning on activation, never on deactivation -- exactly
        // backwards for this case. Re-pinning unconditionally (not just
        // when wParam != WA_INACTIVE) is what actually stops the desktop
        // surface from staying above Mosaic afterward; PinToDesktopBottom
        // is cheap and idempotent, so calling it on every activation
        // change costs nothing extra on the far more common ordinary
        // activate/deactivate pairs (e.g. clicking a different app).
        PinToDesktopBottom();
        return 0;

    case security::WM_MOSAIC_HELLO_RESULT:
        // The Windows Hello prompt finished on a background thread; this
        // is the UI thread picking up the result (see WindowsHello.h).
        security::WindowsHello::PumpResult();
        InvalidateRect(m_hwnd, nullptr, FALSE);
        return 0;

    case WM_MOSAIC_TRAY:
        // Left-click toggles visibility, right-click opens the menu --
        // the conventions every Windows tray app follows.
        if (LOWORD(lParam) == WM_LBUTTONUP) {
            if (m_dashboardVisible) HideDashboard(); else ShowDashboard();
        } else if (LOWORD(lParam) == WM_RBUTTONUP) {
            m_trayIcon.ShowContextMenu(m_hwnd, m_dashboardVisible, m_photosPaused);
        }
        return 0;

    case WM_COMMAND:
        OnTrayCommand(LOWORD(wParam));
        return 0;

    case WM_KILLFOCUS:
        // Settings > Quick Notes > "Lock when window loses focus".
        if (m_settings.lockNotesOnFocusLoss && m_quickNotesWidget && m_quickNotesWidget->IsUnlocked()) {
            m_quickNotesWidget->Lock();
            InvalidateRect(m_hwnd, nullptr, FALSE);
        }
        return 0;

    case media::WM_MOSAIC_PHOTO_READY:
        m_imagePipeline.PumpResult();
        StartTransitionTimer(); // the just-delivered photo may have started a crossfade
        InvalidateRect(m_hwnd, nullptr, FALSE);
        return 0;

    case WM_MOUSEMOVE:
        OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;

    case WM_MOUSELEAVE:
        OnMouseLeave();
        return 0;

    case WM_LBUTTONDOWN:
        OnLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;

    case WM_LBUTTONUP:
        OnLButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;

    case WM_LBUTTONDBLCLK:
        OnLButtonDblClk(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;

    case WM_CHAR:
        OnChar(static_cast<wchar_t>(wParam));
        return 0;

    case WM_KEYDOWN:
        OnKeyDown(static_cast<UINT>(wParam));
        return 0;

    case WM_ERASEBKGND:
        // Prevent GDI from painting the background -- we own every pixel
        // via the D2D/DirectComposition swap chain.
        return 1;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
}

void Window::EnableAcrylicBackdrop() {
    // Real, native OS-level blur-behind. This is the "macOS-widget-style
    // glass" the design calls for, implemented the cheap way: ask DWM to do
    // it, rather than us sampling and blurring the desktop ourselves (which
    // would cost real CPU/GPU every frame and violate the low-resource
    // requirement). Requires Windows 11 22H2+; harmless no-op otherwise.
    //
    // Settings > Appearance > Background Blur toggles this: DWMSBT_NONE
    // turns the OS backdrop off entirely, leaving the dashboard's own
    // translucent card fills over a plain transparent window.
    DWM_SYSTEMBACKDROP_TYPE backdrop = m_settings.blurEnabled ? DWMSBT_TRANSIENTWINDOW : DWMSBT_NONE;
    DwmSetWindowAttribute(m_hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdrop, sizeof(backdrop));

    BOOL darkMode = TRUE;
    DwmSetWindowAttribute(m_hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(darkMode));

    // Extending the frame into the client area with negative margins tells
    // DWM the entire client area participates in glass composition.
    MARGINS margins = { -1, -1, -1, -1 };
    DwmExtendFrameIntoClientArea(m_hwnd, &margins);
}

// Fires on every foreground-window change, system-wide -- an OS event
// callback via SetWinEventHook, not a poll, so idle CPU cost is zero
// between actual changes. This is the direct, low-risk fix for the
// three-finger-swipe-down / Win+D "Show Desktop" gesture: that gesture
// works by making Progman (or its WorkerW) the foreground window to
// *reveal* the desktop, which is exactly the one specific foreground
// transition this checks for. Explorer's WM_SHOWWINDOW-based hide (the
// separate WM_MOSAIC_REASSERT_VISIBLE path elsewhere in this file) covers
// the rarer case of Mosaic actually being hidden outright; this covers
// the far more common "something else got raised in front of it" case,
// including but not limited to Show Desktop.
//
// Deliberately does NOT attempt the WorkerW-reparent trick (see
// kAttemptWorkerWReparent's comment for why that's off) -- this only
// calls the existing, always-safe PinToDesktopBottom(), so it can only
// ever re-affirm Z-order on a window that's already known to render and
// accept input correctly. It cannot reintroduce either of the failure
// modes found while investigating the reparent approach.
void CALLBACK Window::OnForegroundChanged(HWINEVENTHOOK /*hook*/, DWORD event, HWND hwndForeground,
                                           LONG idObject, LONG idChild, DWORD /*threadId*/, DWORD /*eventTime*/) {
    if (event != EVENT_SYSTEM_FOREGROUND) return;
    if (idObject != OBJID_WINDOW || idChild != CHILDID_SELF) return;
    if (!hwndForeground) return;

    wchar_t className[32]{};
    if (!GetClassNameW(hwndForeground, className, static_cast<int>(std::size(className)))) return;
    // Both Progman (classic layout) and WorkerW (raised-desktop layout,
    // or the classic split's own WorkerW) taking foreground mean the same
    // thing here: Explorer just raised the desktop surface.
    if (wcscmp(className, L"Progman") != 0 && wcscmp(className, L"WorkerW") != 0) return;

    // WinEvent callbacks can fire on a different thread than the one that
    // created the window (WINEVENT_OUTOFCONTEXT explicitly allows this) --
    // PostMessage is the safe, existing way back onto the UI thread, and
    // this reuses the exact reassert path WM_SHOWWINDOW already drives.
    // Looking the window up by class name (rather than threading the
    // Window* through as SetWinEventHook's own userdata, which it doesn't
    // support) is fine here: this is a best-effort nudge, not a
    // correctness-critical path, so the ordinary single-window case this
    // app is built for is all it needs to handle correctly.
    HWND mosaic = FindWindowW(kWindowClassName, nullptr);
    if (mosaic) PostMessage(mosaic, WM_MOSAIC_REASSERT_VISIBLE, 0, 0);
}

void Window::InstallForegroundWatcher() {
    // WINEVENT_OUTOFCONTEXT: no DLL injection into Explorer needed, at
    // the cost of the callback arriving on a worker thread -- handled via
    // PostMessage above. WINEVENT_SKIPOWNPROCESS: never fires for our own
    // window's own foreground transitions (e.g. ShowDashboard's SetFocus),
    // which would otherwise turn every normal click into a wasted
    // re-assert.
    m_foregroundHook = SetWinEventHook(
        EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND,
        nullptr, OnForegroundChanged, 0, 0,
        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
}

void Window::PinToDesktopBottom() {
    // The robust half of desktop pinning: works on every Windows version,
    // survives Explorer updates, and doesn't depend on undocumented
    // internals. SWP_NOACTIVATE here is about *this specific call* not
    // stealing activation -- it has nothing to do with the
    // WS_EX_NOACTIVATE style (which we deliberately don't use; see the
    // WM_WINDOWPOSCHANGING comment above).
    SetWindowPos(m_hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void Window::TryReparentToDesktopWorkerW() {
    if (!kAttemptWorkerWReparent) return;

    HWND progman = FindWindowW(L"Progman", nullptr);
    if (!progman) return;

    // Windows 11 24H2+ ("raised desktop"): Progman itself carries
    // WS_EX_NOREDIRECTIONBITMAP, opting out of its own composition
    // surface, because Explorer composes the desktop icons and wallpaper
    // through a different mechanism than the classic GDI-backed WorkerW
    // split. Reparenting *onto Progman itself* is therefore a hard
    // rendering wall (confirmed the hard way: it made Mosaic invisible
    // while still running). But Explorer still creates a proper, ordinary
    // WS_CHILD WorkerW *underneath* Progman on this layout too -- it just
    // needs the (0xD, 0x1) argument form to spawn it, not the classic
    // (0, 0) -- and that child WorkerW has its own real redirection
    // surface, so reparenting onto *it* (never onto Progman) renders
    // normally. This two-path split, and the exact 0x052C argument forms
    // for each, is confirmed independently by several other
    // desktop-pinning tools (FeatherWall, rexpaper, kirie, Motiva).
    LONG_PTR progmanExStyle = GetWindowLongPtrW(progman, GWL_EXSTYLE);
    bool raisedDesktop = (progmanExStyle & WS_EX_NOREDIRECTIONBITMAP) != 0;

    HWND targetWorkerW = nullptr;
    if (raisedDesktop) {
        // Single send only -- sending this twice has been observed to
        // tear down the WorkerW it just created instead of being a
        // harmless no-op repeat (see rexpaper's raised-desktop fix).
        SendMessageTimeoutW(progman, 0x052C, 0xD, 0x1, SMTO_NORMAL, 1000, nullptr);
        // The child WorkerW sits under Progman itself now, not as a
        // top-level EnumWindows sibling -- FindWindowExW scoped to
        // Progman's children is the equivalent lookup for this layout.
        for (HWND child = FindWindowExW(progman, nullptr, L"WorkerW", nullptr); child;
             child = FindWindowExW(progman, child, L"WorkerW", nullptr)) {
            // The icons' own WorkerW (if Explorer still wraps
            // SHELLDLL_DefView in one on a given build) is not the
            // target -- skip it the same way the classic search does.
            if (!FindWindowExW(child, nullptr, L"SHELLDLL_DefView", nullptr)) {
                targetWorkerW = child;
                break;
            }
        }
        if (!targetWorkerW) {
            OutputDebugStringW(L"Mosaic: raised-desktop WorkerW child not found -- staying on HWND_BOTTOM\n");
            return;
        }
    } else {
        // Classic (pre-24H2) layout: Explorer splits into two top-level
        // WorkerWs; the message needs the classic (0,0) form here.
        SendMessageTimeoutW(progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, nullptr);
        EnumWindows(FindWorkerWProc, reinterpret_cast<LPARAM>(&targetWorkerW));
        if (!targetWorkerW) {
            OutputDebugStringW(L"Mosaic: WorkerW sibling not found on classic-layout Explorer -- staying on HWND_BOTTOM\n");
            return;
        }
    }

    SetParent(m_hwnd, targetWorkerW);
    // Deliberately NOT converting WS_POPUP to WS_CHILD here, even on the
    // raised-desktop path: Mosaic's whole keyboard-input model (see the
    // "Desktop pinning: Z-order enforcement" comment above
    // WM_WINDOWPOSCHANGING) depends on being a real top-level window that
    // can become the foreground/active window on click, which is how
    // WM_CHAR delivery to Quick Notes/To Do actually works. A WS_CHILD
    // window doesn't participate in foreground activation the same way,
    // so converting the style would risk silently breaking typing to
    // save nothing -- SetParent alone (leaving the WS_POPUP style as-is)
    // is enough to place Mosaic in the new Z-order/paint hierarchy, and
    // is a legal, if slightly unusual, Win32 configuration other
    // desktop-pinning tools without a keyboard-input requirement don't
    // need to preserve.
    OutputDebugStringW(raisedDesktop
        ? L"Mosaic: pinned behind desktop icons (raised-desktop WorkerW child)\n"
        : L"Mosaic: pinned behind desktop icons (classic WorkerW sibling)\n");
}

HRESULT Window::Create(HINSTANCE hInstance, int nCmdShow) {
    // Resolved before the window (or the tray icon) exists so the very
    // first WM_TASKBARCREATED broadcast -- which can arrive immediately
    // if Explorer is mid-restart -- is never missed.
    m_taskbarCreatedMsg = RegisterWindowMessageW(L"TaskbarCreated");

    WNDCLASSEX wc{};
    wc.cbSize = sizeof(wc);
    // CS_DBLCLKS: without this, Windows never sends WM_LBUTTONDBLCLK -- the
    // To Do widget's "double-click a row to rename" relies on it.
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = WndProcThunk;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr; // we own painting entirely
    wc.lpszClassName = kWindowClassName;
    RegisterClassEx(&wc);

    // m_dpi = GetDpiForSystem();
    // int widthPx = MulDiv(kDefaultWidthDip, m_dpi, 96);
    // int heightPx = MulDiv(kDefaultHeightDip, m_dpi, 96);

    // // WS_EX_NOREDIRECTIONBITMAP: required so DWM does not allocate its own
    // // redirection surface, which would sit *behind* our DirectComposition
    // // visual and defeat the whole point of presenting through DComp.
    // // WS_EX_TOOLWINDOW: no taskbar button, excluded from Alt-Tab -- this
    // // is what makes Mosaic read as a desktop widget rather than an app.
    // // Deliberately NOT WS_EX_NOACTIVATE -- see the WM_WINDOWPOSCHANGING
    // // comment in HandleMessage for why that would break keyboard input.
    // // WS_POPUP (no title bar/border) matches the borderless widget look.
    // int startX = m_settingsRepository.GetInt(L"WindowX", 50);
    // int startY = m_settingsRepository.GetInt(L"WindowY", 50);
    // HWND hwnd = CreateWindowEx(
    //     WS_EX_NOREDIRECTIONBITMAP | WS_EX_TOOLWINDOW,
    //     kWindowClassName, L"Mosaic",
    //     WS_POPUP | WS_VISIBLE,
    //     startX, startY, widthPx, heightPx,
    //     nullptr, nullptr, hInstance, this);
    m_dpi = GetDpiForSystem();
    int widthPx = MulDiv(kDefaultWidthDip, m_dpi, 96);
    int heightPx = MulDiv(kDefaultHeightDip, m_dpi, 96);

    // 1. Get the primary screen width
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    
    // 2. Calculate the top-right position (leaving a 40-pixel margin from the edges)
    int defaultRightX = screenWidth - widthPx - 40;
    int defaultTopY = 40;

    // 3. Pass the calculated coordinates as the fallback defaults
    int startX = m_settingsRepository.GetInt(L"WindowX", defaultRightX);
    int startY = m_settingsRepository.GetInt(L"WindowY", defaultTopY);
    
    HWND hwnd = CreateWindowEx(
        WS_EX_NOREDIRECTIONBITMAP | WS_EX_TOOLWINDOW,
        kWindowClassName, L"Mosaic",
        WS_POPUP | WS_VISIBLE,
        startX, startY, widthPx, heightPx,
        nullptr, nullptr, hInstance, this);

    if (!hwnd) return HRESULT_FROM_WIN32(GetLastError());
    m_hwnd = hwnd;

    // --- Data layer: open the database before anything tries to use it ---
    // A failed Open() (permissions, disk full, corrupt file) must not
    // crash the dashboard (spec section 60) -- TodoRepository's methods
    // already fail safe (return empty/0 on error) if m_database's handle
    // is null, so we deliberately continue past a failed Open() rather
    // than aborting Create() entirely. The user gets a dashboard with a
    // To Do widget that just can't save anything this session, not a
    // crash.
    HRESULT dbHr = m_database.Open();
    if (SUCCEEDED(dbHr)) {
        m_todoRepository.SeedDefaultsIfEmpty();
        m_activityRepository.SeedDefaultIfEmpty();
        m_pinnedRepository.SeedDefaultsIfEmpty();
    }

    // Settings must load before anything that reads them -- the theme,
    // the backdrop, and which widgets the layout includes all depend on
    // these values. Falls back to AppSettings' own defaults for a fresh
    // install where nothing has been saved yet.
    m_settings.LoadFrom(m_settingsRepository);
    app::ApplyThemeSettings(m_settings, m_theme);
    EnableAcrylicBackdrop();

    // Desktop pinning: robust HWND_BOTTOM enforcement always applies;
    // the WorkerW reparent is attempted on top of it if enabled above.
    // Re-pinning after the reparent matters because HWND_BOTTOM is
    // relative to the *current* parent's children -- the first call
    // above sinks us to the bottom of our original parent, but SetParent
    // inside TryReparentToDesktopWorkerW moves us under a different
    // parent (Progman or a WorkerW) without itself guaranteeing where
    // among that parent's other children (notably SHELLDLL_DefView) we
    // land, so this second call is what actually puts Mosaic behind the
    // icons rather than merely somewhere in the same window.
    PinToDesktopBottom();
    TryReparentToDesktopWorkerW();
    PinToDesktopBottom();

    // Zero-cost (event-driven, not polled) watchdog for the three-finger
    // swipe-down / Win+D "Show Desktop" gesture -- see
    // InstallForegroundWatcher's own comment for how and why.
    InstallForegroundWatcher();

    // --- Photo source ---------------------------------------------------
    // Uses whatever folder Settings > Photo & Media last picked, falling
    // back to the user's Pictures folder if none has been chosen. A
    // missing or empty folder isn't an error -- PhotoWidget shows "No
    // photos found" rather than anything crashing (spec section 60).
    std::wstring photoFolder = m_settingsRepository.GetString(L"photo.folder", L"");
    if (photoFolder.empty()) {
        PWSTR picturesPath = nullptr;
        if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Pictures, 0, nullptr, &picturesPath))) {
            photoFolder = picturesPath;
            CoTaskMemFree(picturesPath);
        }
    }
    if (!photoFolder.empty()) {
        m_photoProvider.SetFolder(photoFolder);
    }
    m_imagePipeline.Start(m_hwnd);

    // Windows Hello runs asynchronously and posts its result back to this
    // window, so the widget can't call it directly -- Window supplies
    // this adapter, which is also the single place that decides Hello is
    // unavailable and lets the widget fall back to a password.
    auto requestHello = [this](const std::wstring& message,
                                std::function<void(security::HelloResult)> onComplete) {
        security::WindowsHello::RequestVerification(m_hwnd, message, std::move(onComplete));
    };

    m_widgetManager = std::make_unique<widgets::WidgetManager>(
        &m_todoRepository, &m_activityRepository, &m_pinnedRepository, &m_notesRepository,
        &m_photoProvider, &m_imagePipeline, requestHello);
    m_widgetManager->Initialize();
    m_photoWidget = static_cast<widgets::PhotoWidget*>(m_widgetManager->Get(widgets::WidgetId::Photo));
    m_quickNotesWidget = static_cast<widgets::QuickNotesWidget*>(m_widgetManager->Get(widgets::WidgetId::QuickNotes));
    if (m_quickNotesWidget) {
        m_quickNotesWidget->SetAutoLockSeconds(m_settings.noteAutoLockSeconds);
    }

    m_trayIcon.Create(m_hwnd, L"Mosaic");

    m_graphics = std::make_unique<ui::GraphicsDevice>();
    HRESULT hr = m_graphics->Initialize(m_hwnd, static_cast<UINT>(widthPx), static_cast<UINT>(heightPx));
    if (FAILED(hr)) return hr;

    ui::SettingsCallbacks settingsCallbacks;
    settingsCallbacks.onSettingsChanged = [this] {
        // Persist immediately, then apply. There's no "Save" button by
        // design -- every other control in Mosaic (To Do, Activity,
        // widget position) already commits on change, and Settings
        // matching that is less surprising than introducing a second
        // convention.
        m_settings.SaveTo(m_settingsRepository);
        app::ApplyThemeSettings(m_settings, m_theme);
        ApplyNonThemeSettings();
        if (m_quickNotesWidget) m_quickNotesWidget->SetAutoLockSeconds(m_settings.noteAutoLockSeconds);
        if (m_dashboardView) m_dashboardView->RebuildLayout(); // widget enable/disable may have changed
        UpdateHeaderData();   // important: refresh greeting/date immediately
        InvalidateRect(m_hwnd, nullptr, FALSE);
    };
    settingsCallbacks.onResetLayout = [this] {
        m_layoutRepository.ClearAll();
        if (m_dashboardView) m_dashboardView->RebuildLayout();
        InvalidateRect(m_hwnd, nullptr, FALSE);
    };
    settingsCallbacks.onPickPhotoFolder = [this] { PickPhotoFolder(); };

    m_dashboardView = std::make_unique<ui::DashboardView>(
        m_graphics->DWriteFactory(), &m_theme, m_widgetManager.get(), &m_layoutRepository,
        &m_settings, std::move(settingsCallbacks));
    hr = m_dashboardView->CreateDeviceResources(m_graphics->DeviceContext());
    if (FAILED(hr)) return hr;
    m_deviceResourcesValid = true;

    if (m_photoWidget) {
        m_photoWidget->RequestNextPhoto(m_graphics->DeviceContext());
    }

    ApplyNonThemeSettings();

    UpdateHeaderData();
    CheckActivityReminders();
    SetTimer(hwnd, /*id*/ 1, 60000, nullptr); // 30s timer for greeting/date refresh and activity reminders

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    PinToDesktopBottom(); // ShowWindow can itself reorder us -- reassert once more after it
    return S_OK;
}

void Window::UpdateHeaderData() {
    SYSTEMTIME st;
    GetLocalTime(&st);

    const wchar_t* greeting =
        (st.wHour < 12)  ? L"Good Morning," :
        (st.wHour < 17)  ? L"Good Afternoon," :
        (st.wHour < 21)  ? L"Good Evening," :
                            L"Good Night,";
    m_headerData.greetingLine = greeting;
    // Falls back to a generic default rather than a specific hardcoded
    // name -- an empty AppSettings::displayName means the user (or a
    // fresh install) never set one, and showing someone else's old
    // hardcoded name in that case would be actively wrong, not just bland.
    m_headerData.userName = m_settings.displayName.empty() ? L"there" : m_settings.displayName;
    m_headerData.motivation = m_settings.motivationText.empty()
        ? L"Keep going, great things take time."
        : m_settings.motivationText;

    static const wchar_t* kWeekday[] = { L"Sun", L"Mon", L"Tue", L"Wed", L"Thu", L"Fri", L"Sat" };
    static const wchar_t* kMonth[] = {
        L"Jan", L"Feb", L"Mar", L"Apr", L"May", L"Jun",
        L"Jul", L"Aug", L"Sep", L"Oct", L"Nov", L"Dec"
    };
    m_headerData.weekday = kWeekday[st.wDayOfWeek];

    std::wstringstream dateStream;
    dateStream << st.wDay << L" " << kMonth[st.wMonth - 1] << L" " << st.wYear;
    m_headerData.fullDate = dateStream.str();
}

void Window::CheckActivityReminders() {
    int64_t now = static_cast<int64_t>(std::time(nullptr));
    auto due = m_activityRepository.GetDueForNotification(now);
    for (const auto& activity : due) {
        std::wstring when = widgets::activity_datetime::FormatForDisplay(activity.dueAt);
        notifications::ToastNotifier::Show(activity.title, when);
        m_activityRepository.MarkNotified(activity.id);
    }
}

void Window::ApplyNonThemeSettings() {
    // Desktop pinning supersedes the old Always-on-Top setting: "always
    // behind every other window" and "always on top of every other
    // window" are directly contradictory, and pinning is now the
    // permanent, unconditional window behavior rather than an optional
    // toggle. m_settings.alwaysOnTop is intentionally not read here
    // anymore -- see the class comment in Window.h. Settings > Desktop's
    // "Always on Top" checkbox is now inert until/unless it's repurposed
    // or removed from the UI; it is not wired to anything as of this
    // change.
    PinToDesktopBottom();

    EnableAcrylicBackdrop(); // picks up m_settings.blurEnabled
    ApplyStartWithWindows(m_settings.startWithWindows);
}

void Window::ApplyStartWithWindows(bool enabled) {
    // The standard unprivileged run-at-login mechanism: a per-user value
    // under HKCU\...\Run. No admin rights, no scheduled task, no
    // service -- and trivially inspectable/removable by the user, which
    // matters for something that modifies startup behavior.
    HKEY key = nullptr;
    if (RegOpenKeyExW(HKEY_CURRENT_USER,
                      L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                      0, KEY_SET_VALUE, &key) != ERROR_SUCCESS) {
        return; // registry unavailable -- fail silently rather than crash (spec section 60)
    }

    if (enabled) {
        wchar_t exePath[MAX_PATH]{};
        DWORD len = GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        if (len > 0 && len < MAX_PATH) {
            // Quoted so a path containing spaces (very common under
            // C:\Users\First Last\...) parses as one argument.
            std::wstring quoted = L"\"" + std::wstring(exePath) + L"\"";
            RegSetValueExW(key, L"Mosaic", 0, REG_SZ,
                           reinterpret_cast<const BYTE*>(quoted.c_str()),
                           static_cast<DWORD>((quoted.size() + 1) * sizeof(wchar_t)));
        }
    } else {
        RegDeleteValueW(key, L"Mosaic"); // absent value is fine; error ignored deliberately
    }
    RegCloseKey(key);
}

void Window::PickPhotoFolder() {
    // IFileDialog with FOS_PICKFOLDERS is the modern folder picker --
    // SHBrowseForFolder still works but looks like Windows XP, which
    // would undercut the whole point of the design.
    Microsoft::WRL::ComPtr<IFileDialog> dialog;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog));
    if (FAILED(hr)) return;

    DWORD options = 0;
    if (SUCCEEDED(dialog->GetOptions(&options))) {
        dialog->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
    }

    if (FAILED(dialog->Show(m_hwnd))) return; // user cancelled -- not an error

    Microsoft::WRL::ComPtr<IShellItem> item;
    if (FAILED(dialog->GetResult(&item))) return;

    PWSTR path = nullptr;
    if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path)) && path) {
        m_settingsRepository.SetString(L"photo.folder", path);
        m_photoProvider.SetFolder(path);
        CoTaskMemFree(path);

        // Show something from the new folder straight away rather than
        // waiting out the rest of the current rotation interval.
        m_minutesSinceLastPhoto = 0;
        if (m_photoWidget && m_graphics) {
            m_photoWidget->RequestNextPhoto(m_graphics->DeviceContext());
        }
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }

    // The picker's own dialog window steals foreground/activation while
    // open (unavoidable -- it's a modal system dialog). Reassert pinning
    // now that it's closed rather than waiting for the next incidental
    // WM_ACTIVATE.
    PinToDesktopBottom();
}

void Window::ShowDashboard() {
    // No SetForegroundWindow here (that was the pre-pinning behavior) --
    // forcing foreground would fight desktop pinning by definition.
    // ShowWindow alone makes it visible; PinToDesktopBottom keeps it
    // exactly where a desktop widget belongs even once shown.
    ShowWindow(m_hwnd, SW_SHOWNA);
    PinToDesktopBottom();
    m_dashboardVisible = true;
}

void Window::HideDashboard() {
    // Lock notes before hiding -- leaving decrypted content in memory
    // behind a hidden window would quietly defeat the lock.
    if (m_quickNotesWidget && m_quickNotesWidget->IsUnlocked()) {
        m_quickNotesWidget->Lock();
    }
    // Cleared before ShowWindow, not after: the WM_SHOWWINDOW handler
    // above runs synchronously inside this very ShowWindow call, and it
    // uses m_dashboardVisible to tell "the user asked to hide this" apart
    // from "Explorer's Show-Desktop sweep hid this out from under us".
    // Clearing it first means that guard is already correct at the moment
    // the handler checks it, instead of racing a hide this function
    // itself just triggered.
    m_dashboardVisible = false;
    ShowWindow(m_hwnd, SW_HIDE);
}

void Window::OnTrayCommand(UINT commandId) {
    switch (commandId) {
    case TrayCmd_Show:
        ShowDashboard();
        break;
    case TrayCmd_Hide:
        HideDashboard();
        break;
    case TrayCmd_Settings:
        ShowDashboard();
        if (m_dashboardView) {
            m_dashboardView->OpenSettings();
            InvalidateRect(m_hwnd, nullptr, FALSE);
        }
        break;
    case TrayCmd_PausePhotos:
        m_photosPaused = !m_photosPaused;
        break;
    case TrayCmd_Quit:
        // Lock (and therefore save) notes before tearing down, so a quit
        // from the tray doesn't silently lose an unsaved edit.
        if (m_quickNotesWidget) m_quickNotesWidget->Lock();
        m_trayIcon.Destroy();
        DestroyWindow(m_hwnd);
        break;
    default:
        break;
    }
}

void Window::StartTransitionTimer() {
    if (m_transitionTimerRunning) return;
    // Settings > Performance > Animations off means the timer never runs:
    // photo changes cut straight to the new image and dropped widgets
    // appear in their final slot immediately. Both code paths already
    // handle "no animation frames arrive" correctly -- they just render
    // their end state -- so this needs no special-casing elsewhere.
    if (!m_settings.animationsEnabled) return;
    bool photoAnimating = m_photoWidget && m_photoWidget->IsTransitioning();
    bool layoutAnimating = m_dashboardView && m_dashboardView->IsAnimating();
    if (!photoAnimating && !layoutAnimating) return;
    // ~30fps is plenty smooth for a simple opacity crossfade or rect
    // interpolation, and cheap enough to run for a couple hundred
    // milliseconds without it reading as "the app just started animating
    // things continuously" -- it stops itself (see the WM_TIMER id==2
    // handler) the moment both report done.
    SetTimer(m_hwnd, /*id*/ 2, 33, nullptr);
    m_transitionTimerRunning = true;
}

void Window::StopTransitionTimer() {
    if (!m_transitionTimerRunning) return;
    KillTimer(m_hwnd, /*id*/ 2);
    m_transitionTimerRunning = false;
}

void Window::OnPaint() {
    if (!m_deviceResourcesValid) return;

    RECT rc;
    GetClientRect(m_hwnd, &rc);
    float dipScale = 96.0f / static_cast<float>(m_dpi);
    D2D1_RECT_F bounds = {
        0.0f, 0.0f,
        static_cast<float>(rc.right - rc.left) * dipScale,
        static_cast<float>(rc.bottom - rc.top) * dipScale
    };

    m_graphics->DeviceContext()->SetDpi(static_cast<float>(m_dpi), static_cast<float>(m_dpi));
    m_graphics->BeginDraw();
    m_graphics->DeviceContext()->Clear(D2D1::ColorF(0, 0, 0, 0)); // fully transparent; DWM backdrop shows through
    m_dashboardView->Draw(m_graphics->DeviceContext(), bounds, m_headerData);
    HRESULT hr = m_graphics->EndDraw();

    if (hr == D2DERR_RECREATE_TARGET || hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
        // Device-lost recovery: drop everything and rebuild. Rare in
        // practice (driver reset, remote-desktop reconnect) but must
        // never crash the dashboard (see spec section 60, error
        // handling).
        m_dashboardView->ReleaseDeviceResources();
        m_deviceResourcesValid = false;

        for (const auto& [id, widget] : m_widgetManager->Widgets()) {
            widget->OnDeviceLost();
        }

        RECT client;
        GetClientRect(m_hwnd, &client);
        m_graphics = std::make_unique<ui::GraphicsDevice>();
        if (SUCCEEDED(m_graphics->Initialize(m_hwnd, client.right - client.left, client.bottom - client.top)) &&
            SUCCEEDED(m_dashboardView->CreateDeviceResources(m_graphics->DeviceContext()))) {
            m_deviceResourcesValid = true;
            if (m_photoWidget) {
                m_photoWidget->RequestNextPhoto(m_graphics->DeviceContext());
            }
            InvalidateRect(m_hwnd, nullptr, FALSE);
        }
    }
}

void Window::OnResize(UINT width, UINT height) {
    if (!m_graphics) return;
    m_graphics->Resize(width, height);
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void Window::OnDpiChanged(UINT newDpi, const RECT* suggestedRect) {
    m_dpi = newDpi;
    if (suggestedRect) {
        SetWindowPos(m_hwnd, nullptr,
                     suggestedRect->left, suggestedRect->top,
                     suggestedRect->right - suggestedRect->left,
                     suggestedRect->bottom - suggestedRect->top,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

D2D1_POINT_2F Window::PixelToDip(int pixelX, int pixelY) const {
    float scale = 96.0f / static_cast<float>(m_dpi);
    return { static_cast<float>(pixelX) * scale, static_cast<float>(pixelY) * scale };
}

void Window::OnMouseMove(int pixelX, int pixelY) {
    if (!m_trackingMouseLeave) {
        // Ask Windows to send us exactly one WM_MOUSELEAVE when the
        // cursor exits the client area -- the standard pattern for hover
        // state, since WM_MOUSEMOVE alone never fires once the pointer
        // leaves.
        TRACKMOUSEEVENT tme{};
        tme.cbSize = sizeof(tme);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = m_hwnd;
        TrackMouseEvent(&tme);
        m_trackingMouseLeave = true;
    }

    if (!m_dashboardView) return;
    D2D1_POINT_2F dip = PixelToDip(pixelX, pixelY);
    if (m_dashboardView->OnMouseMove(dip)) {
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

void Window::OnMouseLeave() {
    m_trackingMouseLeave = false;
    if (!m_dashboardView) return;
    if (m_dashboardView->OnMouseLeave()) {
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

void Window::OnLButtonDown(int pixelX, int pixelY) {
    if (!m_dashboardView) return;
    // Windows sends focus to whatever window was clicked, but WS_POPUP
    // windows don't automatically take keyboard focus the way a normal
    // top-level window does -- without this, WM_CHAR/WM_KEYDOWN never
    // arrive after clicking into the dashboard.
    SetFocus(m_hwnd);

    // Captures the mouse for the duration of a potential drag: without
    // this, moving the cursor fast enough during a drag can leave the
    // window's client area, which would stop delivering WM_MOUSEMOVE
    // (and worse, the eventual WM_LBUTTONUP) entirely. Released
    // unconditionally in OnLButtonUp, whether or not a drag actually
    // happened.

    // SetCapture(m_hwnd);

    D2D1_POINT_2F dip = PixelToDip(pixelX, pixelY);
    bool widgetClicked = m_dashboardView->OnLButtonDown(dip);
    // bool changed = m_dashboardView->OnLButtonDown(dip);

    // if (changed) InvalidateRect(m_hwnd, nullptr, FALSE);
    if (!widgetClicked) {
        // We clicked the empty background! Drag the whole OS window.
        ReleaseCapture();
        SendMessage(m_hwnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
    } else {
        // We clicked a card! Let DashboardView handle the internal drag/resize.
        SetCapture(m_hwnd);
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

void Window::OnLButtonUp(int pixelX, int pixelY) {
    ReleaseCapture();
    if (!m_dashboardView) return;

    D2D1_POINT_2F dip = PixelToDip(pixelX, pixelY);
    bool changed = m_dashboardView->OnLButtonUp(dip);

    // A drop may have just started a settle animation (or a drag that
    // moved but didn't cross the threshold may have just resolved into
    // an ordinary click) -- either way, make sure the shared animation
    // timer is running if DashboardView now needs it.
    StartTransitionTimer();

    if (changed) InvalidateRect(m_hwnd, nullptr, FALSE);
}

void Window::OnLButtonDblClk(int pixelX, int pixelY) {
    if (!m_dashboardView) return;
    D2D1_POINT_2F dip = PixelToDip(pixelX, pixelY);
    if (m_dashboardView->OnDoubleClick(dip)) {
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

void Window::OnChar(wchar_t ch) {
    if (!m_dashboardView) return;
    if (m_dashboardView->OnChar(ch)) {
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

void Window::OnKeyDown(UINT virtualKey) {
    if (!m_dashboardView) return;
    if (m_dashboardView->OnKeyDown(virtualKey)) {
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
}

int Window::RunMessageLoop() {
    MSG msg{};
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return static_cast<int>(msg.wParam);
}

} // namespace mosaic::platform