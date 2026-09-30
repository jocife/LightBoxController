# LED GUI

## Table of Contents
- [LED GUI](#led-gui)
  - [Table of Contents](#table-of-contents)
  - [Overview](#overview)
    - [What is Qt?](#what-is-qt)
  - [Prerequisites](#prerequisites)
    - [1. C++ Compiler \& Qt Framework](#1-c-compiler--qt-framework)
    - [2. Build \& Packaging Tools](#2-build--packaging-tools)
    - [3. MATLAB Environment](#3-matlab-environment)
    - [4. Development \& Debugging](#4-development--debugging)
  - [Project Structure](#project-structure)
    - [Key Directories in Detail:](#key-directories-in-detail)
  - [Getting Started](#getting-started)
  - [How to use MATLAB Coder](#how-to-use-matlab-coder)
    - [Automated Build (Recommended)](#automated-build-recommended)
    - [Manual Build (Alternative)](#manual-build-alternative)
  - [Quick Build \& Run Scripts](#quick-build--run-scripts)
    - [`update_project.ps1` — Complete Project Update (Recommended)](#update_projectps1--complete-project-update-recommended)
    - [`update_matlab.ps1` — MATLAB Build Only](#update_matlabps1--matlab-build-only)
    - [`build_release_exe.ps1` — Release Rebuild \& Packaging](#build_release_exeps1--release-rebuild--packaging)
    - [`publish_release.ps1` — Build \& Publish Forgejo Release](#publish_releaseps1--build--publish-forgejo-release)
    - [`update_qt.ps1` — Qt Build \& Run Only](#update_qtps1--qt-build--run-only)
  - [Uninstalling the Application](#uninstalling-the-application)
  - [License](#license)

## Overview

This application is used to control a specific light box and is built with C++ using the Qt framework.

### What is Qt?

> Qt (/ˈkjuːt/ pronounced "cute") is a cross-platform application development framework for creating graphical user interfaces as well as cross-platform applications that run on various software and hardware platforms such as Linux, Windows, macOS, Android or embedded systems with little or no change in the underlying codebase while still being a native application with native capabilities and speed. [[Wikipedia](https://en.wikipedia.org/wiki/Qt_(software))]


---

## Prerequisites

Ensure you have the following software and tools installed before building the project. Where applicable, you can use [WinGet](https://github.com/microsoft/winget-cli) for quick installation via a terminal.

### 1. C++ Compiler & Qt Framework

- **[Visual Studio Community 2022](https://visualstudio.microsoft.com/vs/community/)** (C/C++ Compiler for Windows)
  ```powershell
  winget install -e --id=Microsoft.VisualStudio.2022.Community
  ```
  > **Configuration:** Open *Visual Studio Installer* &rarr; Modify &rarr; check **Desktop development with C++** &rarr; Install.

- **[Qt](https://www.qt.io/development/download-qt-installer-oss)** (Graphical User Interface Framework)
  > **Configuration:** Open *Qt Maintenance Tool* &rarr; Add or remove components &rarr; select **Qt \<version\>** &rarr; check **MSVC 2022 64-bit** &rarr; Install.

### 2. Build & Packaging Tools
- **[CMake](https://cmake.org/download/)**
  ```powershell
  winget install -e --id=Kitware.CMake
  ```
- **[Vulkan SDK](https://vulkan.lunarg.com/sdk/home/)** (Required graphics backend for CMake/Qt)
  ```powershell
  winget install -e --id=KhronosGroup.VulkanSDK
  ```
- **[NSIS (Nullsoft Scriptable Install System)](https://nsis.sourceforge.io/Download)** (Used to package Windows installers)
  ```powershell
  winget install -e --id=NSIS.NSIS
  ```

### 3. MATLAB Environment
- **[MATLAB](https://www.mathworks.com/products/matlab.html)** (Required for regenerating the C++ optimization algorithm logic)
  > **Required Add-Ons:** Ensure both **MATLAB Coder** and the **Optimization Toolbox** are installed in your MATLAB environment before attempting to generate the layout files.

### 4. Development & Debugging
- **[Visual Studio Code](https://code.visualstudio.com/Download/)** (Recommended Code Editor)
  ```powershell
  winget install -e --id=Microsoft.VisualStudioCode
  ```
  **Recommended Extensions:**
  - [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) (Microsoft)
  - [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) (Microsoft)
  - [Markdown All in One](https://marketplace.visualstudio.com/items?itemName=yzhang.markdown-all-in-one) (Yu Zhang)
  - [Markdown Preview Enhanced](https://marketplace.visualstudio.com/items?itemName=shd101wyy.markdown-preview-enhanced) (Yiyi Wang)

    > *(Tip: To view `.md` files side-by-side, right-click the markdown file &rarr; Markdown Preview Enhanced: Open Preview to the Side).*

- **[Hercules](https://www.hw-group.com/software/hercules-setup-utility/)** (Local TCP Server for connection debugging)
  > **Configuration:** Open *Hercules* &rarr; TCP Server &rarr; Server Status &rarr; Set Port to `1234` &rarr; Listen.

---

## Project Structure

This repository is intuitively organized to separate the Qt frontend, the CMake build system, and the MATLAB optimization logic. Here is an overview of the core components:

```text
led-gui/
├── .vscode/          # VS Code tasks, launch configurations, and IntelliSense settings
├── bin/              # Compiled `.exe` binaries and required runtime assets (Debug/Release)
├── build/            # CMake build artifacts, caches, and temporary compilation files
├── include/          # External C++ headers (populated by the MATLAB Coder generation)
├── lib/              # Compiled dependencies, `.lib` static libraries, and `.dll` files
├── matlab/           # MATLAB source scripts outlining the core lighting algorithms
├── resources/        # Application assets (hardware configs, user presets, UI icons, stylesheets)
├── scripts/          # Automation scripts (e.g., PowerShell utilities for DLL management)
└── src/              # C++ source code for the Qt graphical user interface
    ├── app/          # Main application window and orchestrator logic
    ├── components/   # Reusable UI widgets shared across multiple pages
    ├── optimization/ # Core C++ wrapper logic for interacting with external algorithms
    ├── pages/        # The main application tabs (Connection, LED Control, Presets, CIE 1931)
    └── main.cpp      # Application entry point
```

### Key Directories in Detail:
- **`matlab/`**: Contains the mathematical models and constraints for the LED light box. You must process these files with MATLAB Coder to generate the underlying C++ backend before building the UI.
- **`src/`**: Houses all Qt-related code. It is cleanly modularized into `app` (the main window frame), `pages` (the individual functional tabs), and `components` (small, standalone widgets).
- **`resources/`**: Includes necessary runtime data like CSV calibration files (`configs/`), saved parameters (`presets/`), and visual elements (`icons/`, `themes/`).

---

## Getting Started

1. **Clone the repository:**
   ```bash
   git clone https://github.com/jocife/led-gui.git
   ```

2. **Configure your local Qt environment paths:**
   Before building, you must explicitly update the project configuration files to match your installed Qt and MSVC versions (e.g., replace `<QT_VERSION>` with `6.8.0` and `<MSVC_VERSION>` with `2022`). 
   
   Update the fallback paths in the following three files:

   - **`CMakeLists.txt`**:
     ```cmake
     list(APPEND CMAKE_PREFIX_PATH "C:/Qt/<QT_VERSION>/msvc<MSVC_VERSION>_64")
     ```
   
   - **`.vscode/settings.json`**:
     ```json
     {
       "cmake.configureArgs":[
         "-DCMAKE_PREFIX_PATH=C:\\Qt\\<QT_VERSION>\\msvc<MSVC_VERSION>_64"
       ]
     }
     ```
   
   - **`.vscode/tasks.json`**:
     ```json
     {
       "tasks": [
         {
           "command": "C:\\Qt\\<QT_VERSION>\\msvc<MSVC_VERSION>_64\\bin\\designer.exe"
         }
       ]
     }
     ```
     > **UI Editing Tip:** To visually edit `.ui` files directly from VS Code, ensure the file is open and focused in the editor, press `Ctrl+Shift+P`, search for `Tasks: Run Task`, and select **Open in Qt Designer**.

3. **Configure the CMake build environment:**
   Set up the necessary CMake caches and build directories.
   ```bash
   cmake -S . -B build
   ```

4. **Compile the application:**
   Compile the source code. By default, you should build the optimized Release configuration.
   ```bash
   cmake --build build --config Release
   ```

   - *Optional:* To compile the application with debugging symbols, run:
     ```bash
     cmake --build build --config Debug
     ```

5. **Generate a Windows installer (Optional):**
   Package the compiled Release binaries into a standalone `.exe` installer using CPack and NSIS.
   ```bash
   cd build/
   cpack -C Release -G NSIS
   ```

   *The generated installer will be located at:* `build/LED_GUI-<VERSION>-win64.exe`

6. **Launch the application:**
   Execute the compiled binary directly from the terminal.
   ```powershell
   cmd /c start .\bin\Release\LED_GUI.exe
   ```

   - *Optional:* To launch the Debug version, run:
     ```powershell
     cmd /c start .\bin\Debug\LED_GUI.exe
     ```

---

## How to use MATLAB Coder

All MATLAB-related source files are located in the `matlab/` directory. Before building the Qt application, you must use MATLAB Coder to generate the required C++ headers and DLLs.

### Automated Build (Recommended)

The easiest way to build the MATLAB DLL is to use the automated PowerShell script:

**From the project root, run:**
```powershell
.\scripts\build_matlab_dll.ps1
```

This script will automatically:
1. Generate `parameters.mat` using `generate_parameters.m`
2. Compile the MATLAB code into a C++ DLL using `build_dll.m`

**Optional parameters:**
- `-SkipParameterGeneration`: Skip parameter generation if parameters are already up-to-date
  ```powershell
  .\scripts\build_matlab_dll.ps1 -SkipParameterGeneration
  ```

After the build completes, run the separate copy script:
```powershell
.\scripts\copy_matlab_files.ps1
```

### Manual Build (Alternative)

If you prefer to run each step manually, follow the steps below.

1. **Open the MATLAB workspace:**
   Launch MATLAB and navigate to the project directory by clicking `Browse For Folder` &rarr; finding `<repository_root>/matlab/` &rarr; clicking `Select Folder`.

2. **Verify required Add-Ons:**
   Ensure your MATLAB environment has both **MATLAB Coder** and the **Optimization Toolbox** installed before proceeding.

3. **Generate the parameters file:**
   In the `Files` section, find `build_data/generate_parameters.m` &rarr; right click `generate_parameters.m` &rarr; `Run`.
   
   Alternatively, run the following from the MATLAB Command Window:
   ```matlab
   run('build_data/generate_parameters.m')
   ```
   
   Or, run from PowerShell (from the project root):
   ```powershell
   matlab -batch "cd('matlab/build_data'); generate_parameters; disp('Parameters generated')"
   ```
   This generates the internal `build_data/output/parameters.mat` file.

4. **Compile the algorithm into C++:**
   To generate the C++ DLL, right click `build_dll.m` in the `Files` section &rarr; `Run`.
   
   Alternatively, run the following from the MATLAB Command Window:
   ```matlab
   run('build_dll.m')
   ```
   
   Or, run from PowerShell (from the project root):
   ```powershell
   matlab -batch "cd('matlab'); build_dll; disp('Done building DLL')"
   ```
   This will open a build report and generate the necessary C++ `.h`, `.lib`, and `.dll` files in the `codegen/dll/optimize_led_weights/` folder.

5. **Copy the generated files and dependencies:**
   Run the provided PowerShell script which will automatically move the generated files and required MATLAB dependencies into the correct folders.

   Open a PowerShell terminal at the project root and run:
   ```powershell
   .\scripts\copy_matlab_files.ps1
   ```

   *For reference, the script performs the following file copies:*
   - **From** `matlab/codegen/dll/optimize_led_weights/`:
     - `*.h` files &rarr; `include/`
     - `*.lib` and `*.dll` files &rarr; `lib/`
   - **From** MATLAB installation (`C:/Program Files/MATLAB/<MATLAB_VERSION>/`):
     - `extern/include/tmwtypes.h` &rarr; `include/`
     - `bin/win64/libiomp5md.dll` &rarr; `lib/`

   <br>

   > **Note:** By default, the script automatically detects and uses the latest MATLAB version installed at the default `C:/Program Files/MATLAB/` directory. 
   > 
   > *Optional*: If your MATLAB is installed in a custom location, you can specify the path using the `-MatlabRootDir` parameter:
   > ```powershell
   > .\scripts\copy_matlab_files.ps1 -MatlabRootDir "D:/MathWorks/MATLAB/<MATLAB_VERSION>"
   > ```

6. **Relink dependencies:**
   Rebuild the Qt application using CMake to properly include the newly generated MATLAB DLLs. (See **Step 4** in the [Getting Started](#getting-started) section).

---

## Quick Build & Run Scripts

For a streamlined development workflow, three helper PowerShell scripts are provided in the `scripts/` directory:

### `update_project.ps1` — Complete Project Update (Recommended)
Runs the entire project update workflow in a single command:

```powershell
.\scripts\update_project.ps1
```

**What it does:**
1. Builds MATLAB DLL and copies files to the project
2. Rebuilds the C++ Qt application
3. Runs the compiled executable

**Optional parameters:**
- `-Configuration`: Specify `Debug` (default) or `Release`
  ```powershell
  .\scripts\update_project.ps1 -Configuration Release
  ```
- `-SkipMatlabBuild`: Skip MATLAB build, only rebuild Qt
  ```powershell
  .\scripts\update_project.ps1 -SkipMatlabBuild
  ```
- `-NoRun`: Build only, without running the application
  ```powershell
  .\scripts\update_project.ps1 -NoRun
  ```
- `-MatlabRootDir`: Specify custom MATLAB installation path
  ```powershell
  .\scripts\update_project.ps1 -MatlabRootDir "D:/MathWorks/MATLAB/R2024a"
  ```

### `update_matlab.ps1` — MATLAB Build Only
Orchestrates the full MATLAB DLL generation and deployment process:

```powershell
.\scripts\update_matlab.ps1
```

**What it does:**
1. Generates `parameters.mat` with reference data
2. Compiles MATLAB code into C++ DLLs
3. Copies generated files and dependencies to the Qt project

**Optional parameters:**
- `-SkipParameterGeneration`: Skip parameter generation if parameters are already up-to-date
  ```powershell
  .\scripts\update_matlab.ps1 -SkipParameterGeneration
  ```
- `-MatlabRootDir`: Specify custom MATLAB installation path
  ```powershell
  .\scripts\update_matlab.ps1 -MatlabRootDir "D:/MathWorks/MATLAB/R2024a"
  ```

### `build_release_exe.ps1` — Release Rebuild & Packaging
Rebuilds the Release executable from source and packages it for distribution:

```powershell
.\scripts\build_release_exe.ps1
```

**What it does:**
1. Removes any stale `bin/Release` output before building
2. Reconfigures and rebuilds the C++ Qt project from source
3. Creates the NSIS installer by default

**Optional parameters:**
- `-SkipInstaller`: Build the Release executable without generating the installer
  ```powershell
  .\scripts\build_release_exe.ps1 -SkipInstaller
  ```

### `publish_release.ps1` — Build & Publish Forgejo Release
Builds a versioned Windows installer locally and publishes it as a Forgejo release asset:

```powershell
.\scripts\publish_release.ps1 -Version 1.0.1 -ReleaseNotes "Bug fixes and improvements"
```

The script:
1. Verifies that the working tree is clean
2. Configures CMake and builds the versioned NSIS installer
3. Creates and pushes the corresponding Git tag (for example, `v1.0.1`)
4. Calculates the installer SHA-256 checksum
5. Creates the Forgejo release and uploads the installer

The Forgejo API token is requested securely at runtime and is not stored in the repository. The token must have `write:repository` permission.

Use `-Draft` or `-Prerelease` when appropriate. To publish an already-built installer without rebuilding:

```powershell
.\scripts\publish_release.ps1 -Version 1.0.1 -SkipBuild
```

### `update_qt.ps1` — Qt Build & Run Only
Builds the C++ Qt project and runs the resulting executable:

```powershell
.\scripts\update_qt.ps1
```

**What it does:**
1. Builds the C++ project using CMake
2. Runs the compiled application

**Optional parameters:**
- `-Configuration`: Specify `Debug` (default) or `Release`
  ```powershell
  .\scripts\update_qt.ps1 -Configuration Release
  ```
- `-NoRun`: Build only, without running the application
  ```powershell
  .\scripts\update_qt.ps1 -NoRun
  ```

**Example workflows:**

*Full update with Debug build and run:*
```powershell
.\scripts\update_project.ps1
```

*Build Release version without running:*
```powershell
.\scripts\update_project.ps1 -Configuration Release -NoRun
```

*Skip MATLAB rebuild (faster iteration):*
```powershell
.\scripts\update_project.ps1 -SkipMatlabBuild
```

---

## Uninstalling the Application

If you installed the application with the Windows installer, you can uninstall it at any time:

1. Open **Control Panel** &rarr; **All Control Panel Items** &rarr; **Programs and Features**.
2. Search for **LED GUI Application**.
3. Select the program and click **Uninstall**.
4. Follow the on-screen instructions provided by the NSIS uninstaller to complete the removal.

> The uninstaller will guide you through the standard Windows removal process and remove the installed application files from your system.

---

## License

This project is an academic research, open source and availabe under the [GNU General Public License v3.0](LICENSE).
Feel free to modify, distribute, and use the app in accordance with the terms of the license.

