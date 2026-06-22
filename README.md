# TD4_02

## Build and run

This repository contains only the game project. Engine binaries and SDK files are installed by CalyxLauncher from the CalyxEngine GitHub release that matches `.calyxproj` `engineVersion`.

1. Open `TD4_02.sln` in Visual Studio.
2. Build `CalyxLauncher` when you want to install or update the engine package.
3. Set `TD4_02` as the startup project and run Debug, Develop, or Release.

`TD4_02` builds the game code as a DLL under `Generated/Outputs/<Configuration>`. `CalyxGame` is the engine-side host executable that loads that DLL from the paths stored in `.calyxproj`.

Default SDK path: `%LOCALAPPDATA%\\CalyxEngine\\Engines\\v1.0.8\\SDK`.
`CALYX_ENGINE_SDK_DIR` can override this path for local SDK testing.

Debug and Develop builds are intended for editor/development UI. Release builds are intended to run without that GUI.
