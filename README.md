# Mosaic
<p align="">
   <img src="resources/icons/app.ico" alt="Mosaic app icon" width="112">
</p>

A native Win32 + Direct2D + DirectComposition dashboard shell. No Electron,
no browser runtime, no third-party UI framework — just the Windows graphics
stack, driven directly.
</br>
Mosaic does not use Electron, a browser runtime, or a cross-platform UI toolkit. Its rendering and window management are built directly on the Windows graphics stack.

## Platforms

[![Build](https://github.com/nabakrishna/mosaic-widget/actions/workflows/build.yml/badge.svg)](https://github.com/nabakrishna/mosaic-widget/actions/workflows/build.yml)
[![Latest release](https://img.shields.io/github/v/release/nabakrishna/mosaic-widget?color=2ea44f)](https://github.com/nabakrishna/mosaic-widget/releases/latest)
[![Windows](https://img.shields.io/badge/platform-Windows-0078D6?logo=windows&logoColor=white)](https://github.com/nabakrishna/mosaic-widget)
[![CMake](https://img.shields.io/badge/build-CMake-064F8C?logo=cmake&logoColor=white)](https://github.com/nabakrishna/mosaic-widget/blob/main/CMakeLists.txt)
[![Direct2D](https://img.shields.io/badge/graphics-Direct2D-0078D4?logo=windows&logoColor=white)](https://learn.microsoft.com/windows/win32/direct2d/direct2d-portal)
[![DirectComposition](https://img.shields.io/badge/composition-DirectComposition-0078D4?logo=windows&logoColor=white)](https://learn.microsoft.com/windows/win32/directcomp/directcomposition-portal)

## Features

- Desktop widgets for tasks, photos, activities, pinned items, and quick notes.
- Customizable appearance, widget layout, and greeting text.
- Local storage; Quick Notes are protected with Windows data protection.

## Why native C++

Mosaic uses C++20 with Win32, Direct2D, Direct3D 11, and DirectComposition to
integrate directly with the Windows desktop and graphics stack. This avoids
shipping an embedded browser runtime and gives the app control over rendering
and window behavior. The trade-off is that Mosaic is Windows-specific and
requires more platform-specific code than a cross-platform UI framework.

## Performance design

Mosaic redraws when the window is invalidated instead of running a continuous
render loop. Routine date, reminder, and photo checks share a once-per-minute
timer. A temporary timer runs at about 30 frames per second only during layout
or photo transitions, then stops; animations can also be disabled in Settings.
These design choices reduce unnecessary idle work, but performance varies by
device and should be measured before making speed or memory comparisons.

## Download

Download the packaged Windows build from the repository's **Releases** page,
extract it, and run `Mosaic.exe`. If no packaged release is available yet,
build the app from source using the steps below.

## Build from source

You need the following tools:

1. Visual Studio Build Tools (**Desktop development with C++**)
2. CMake 3.20 or newer
3. Ninja
4. VS Code extensions: **CMake Tools** (`ms-vscode.cmake-tools`) and **C/C++** (`ms-vscode.cpptools`), both from Microsoft

1. Clone the repository and open its folder in VS Code.
2. Press `Ctrl+Shift+P` → **CMake: Select a Kit** and choose an installed
   Visual Studio Release kit for **amd64**.
3. Run **CMake: Configure**, then **CMake: Build**.
4. The executable is created at `build/bin/Mosaic.exe`.

## Data and privacy

Mosaic stores its database at `%LOCALAPPDATA%\Mosaic\mosaic.db`. Photos are
read from the folder you select; Mosaic does not move or upload them. Quick
Notes use Windows data protection and are tied to your Windows account.

## Screenshots

![Mosaic Architecture](results/arch-mosaic.svg)

![mosaic look](results/final_look.png)
![Mosaic settings panel](results/settings_panel.png)

## limitations
1. On Windows 11 24H2 and later, the "Show Desktop" gesture (Win+D or a three-finger swipe down on precision touchpads) can temporarily cover the widget until you swipe up or click back into it
2. Quick Notes encryption is tied to your Windows user account — anyone already signed in to that account can still open locked notes; a Windows account with a strong sign-in is the actual security boundary

For more details, see the [Drawbacks section in the manual](manual.md#drawbacks).

## License

See [LICENSE](LICENSE).