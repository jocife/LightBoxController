cmake_minimum_required(VERSION 3.16)
# Extract version dynamically from Git tags
find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE GIT_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
endif()

if(NOT GIT_TAG)
    set(GIT_TAG "v1.0.0") # Fallback if no tags exist
endif()

# CMake's project() command requires a purely numeric version format (X.Y.Z)
# So we strip the 'v' prefix if it exists (e.g. 'v1.1.1' -> '1.1.1')
string(REGEX REPLACE "^v" "" NUMERIC_VERSION "${GIT_TAG}")

project(LightBoxController VERSION ${NUMERIC_VERSION} LANGUAGES CXX)

# --- Output Directorys ---
# Forces all compiled binaries (exe) to separated output directories (Debug, Release)
# in the 'bin' directory at the project root
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/bin/Debug")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/bin/Release")
# ------------------------------

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Automatic MOC, UIC, and RCC handling
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Provide hint for Qt installation directory
list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.10.2/msvc2022_64")

# Detect main Qt version (Qt5 or Qt6)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)

# Load required tools (moc, uic) and specific Qt modules
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS 
    Core Gui Xml Network SvgWidgets Charts Widgets
)

# Source, header, and UI files
set(SOURCES
    src/pages/connection/connectionpagewidget.cpp
    src/pages/ledcontrol/ledcontrolpagewidget.cpp
    src/pages/presets/presetspagewidget.cpp
    src/pages/cie1976/cie1976pagewidget.cpp
    src/pages/sunlightsimulator/sunlightsimulatorpagewidget.cpp
    src/components/ledcontrollerwidget.cpp
    src/components/presetfilemanager.cpp
    src/optimization/optimization_runner.cpp
    src/app/lightboxcontroller.cpp
    src/app/configurationdialog.cpp
    src/main.cpp
    src/pages/connection/connectionpagewidget.h
    src/pages/ledcontrol/ledcontrolpagewidget.h
    src/pages/presets/presetspagewidget.h
    src/pages/cie1976/cie1976pagewidget.h
    src/pages/sunlightsimulator/sunlightsimulatorpagewidget.h
    src/components/ledcontrollerwidget.h
    src/components/presetfilemanager.h
    src/optimization/optimization_runner.h
    src/app/lightboxcontroller.h
    src/app/configurationdialog.h
    src/pages/connection/connectionpagewidget.ui
    src/pages/ledcontrol/ledcontrolpagewidget.ui
    src/pages/presets/presetspagewidget.ui
    src/pages/cie1976/cie1976pagewidget.ui
    src/pages/sunlightsimulator/sunlightsimulatorpagewidget.ui
    src/app/lightboxcontroller.ui
)

# Use target_include_directories so we don't have to change every `#include "..."` exactly.
# It makes the compiler look inside these folders for headers.
include_directories(
    src/app
    src/pages
    src/pages/connection
    src/pages/ledcontrol
    src/pages/presets
    src/pages/cie1976
    src/pages/sunlightsimulator
    src/components
    src/optimization
)

# Define the executable (Defaults to Console subsystem for debugging)
if(WIN32)
    list(APPEND SOURCES resources/icons/lightboxcontroller.rc)
endif()

add_executable(${PROJECT_NAME} ${SOURCES})

# Dynamically hide the console ONLY in Release builds using generator expressions
# This prevents the console window from appearing when running the Release version
if(MSVC)
    # For MSVC (Visual Studio)
    target_link_options(${PROJECT_NAME} PRIVATE 
        "$<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>"
        "$<$<CONFIG:Release>:/ENTRY:mainCRTStartup>"
    )
elseif(MINGW)
    # Fallback option to the MinGW compiler
    target_link_options(${PROJECT_NAME} PRIVATE 
        "$<$<CONFIG:Release>:-mwindows>"
    )
endif()

# Include directories setup
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# --- Robust external library setup using imported targets ---
# Locate the import library (.lib) for the MATLAB DLL
find_library(OPTIMIZE_LED_WEIGHTS_LIB
    NAMES optimize_led_weights
    PATHS "${CMAKE_CURRENT_SOURCE_DIR}/lib"
    REQUIRED
    NO_DEFAULT_PATH
)

# Create an imported target for the external library to ensure clean linking
add_library(External::optimize_led_weights UNKNOWN IMPORTED)
set_target_properties(External::optimize_led_weights PROPERTIES
    IMPORTED_LOCATION "${OPTIMIZE_LED_WEIGHTS_LIB}"
)

# Link Qt modules and external library to the executable
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Xml
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::SvgWidgets
    Qt${QT_VERSION_MAJOR}::Charts
    Qt${QT_VERSION_MAJOR}::Widgets
    External::optimize_led_weights
)

# --- Automatic Windows Deployment (DLLs and windeployqt) ---
if(WIN32)
    # 1. Copy the external DLLs to the output directory
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/optimize_led_weights.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/libiomp5md.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        COMMENT "Copying optimize_led_weights.dll and libiomp5md.dll to the output directory"
    )

    # 2. Copy the entire contents of the resources folder to the output directory
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/resources"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        COMMENT "Copying all resources to the output directory"
    )

    # 3. Locate the Qt binary directory via the qmake target
    get_target_property(_qt_qmake_exec Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
    get_filename_component(_qt_bin_dir "${_qt_qmake_exec}" DIRECTORY)
    
    # 4. Find and run the windeployqt executable
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Running windeployqt..."
            COMMAND "${WINDEPLOYQT_EXECUTABLE}" "$<TARGET_FILE:${PROJECT_NAME}>"
            COMMENT "Deploying Qt libraries to the output directory"
        )
    else()
        message(WARNING "windeployqt not found. Manual deployment required.")
    endif()
endif()
# -----------------------------------------------------------

# --- Install Rules (Required for Deployment and CPack) ---
if(WIN32)
    # Copy the entire configured 'bin' folder (exe, DLLs, resources) into the installer root
    # $<CONFIG> dynamically evaluates to "Release" or "Debug" at packaging time
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/$<CONFIG>/" DESTINATION ".")
elseif(QNX)
    install(TARGETS ${PROJECT_NAME} DESTINATION /tmp/${PROJECT_NAME}/bin)
elseif(UNIX AND NOT ANDROID)
    install(TARGETS ${PROJECT_NAME} DESTINATION /opt/${PROJECT_NAME}/bin)
endif()

# --- CPack Configuration for NSIS Installer (Must be at the EOF) ---
if(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_PACKAGE_NAME "LightBoxController")
    set(CPACK_PACKAGE_VENDOR "BME MOGI")
    set(CPACK_PACKAGE_VERSION "${NUMERIC_VERSION}")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    
    # NSIS specific settings for installation path and UI names
    set(CPACK_PACKAGE_INSTALL_DIRECTORY "LightBoxController")
    set(CPACK_NSIS_DISPLAY_NAME "LightBox Controller Application")
    set(CPACK_NSIS_INSTALLED_ICON_NAME "LightBoxController.exe")
    
    # Create Start Menu shortcut: "Executable name (without .exe)" "Shortcut name"
    set(CPACK_PACKAGE_EXECUTABLES "LightBoxController" "LightBox Controller")
    
    # Force explicit icons for Start Menu and Desktop shortcuts to prevent Windows cache issues
    set(CPACK_NSIS_CREATE_ICONS_EXTRA 
        "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\LightBox Controller.lnk' '$INSTDIR\\\\LightBoxController.exe' '' '$INSTDIR\\\\LightBoxController.exe' 0\n  CreateShortCut '$DESKTOP\\\\LightBox Controller.lnk' '$INSTDIR\\\\LightBoxController.exe' '' '$INSTDIR\\\\LightBoxController.exe' 0"
    )
    
    include(CPack)
endif()
