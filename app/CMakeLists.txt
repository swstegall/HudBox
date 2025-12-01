cmake_minimum_required(VERSION 3.22)
project(HudBox C)

set(CMAKE_C_STANDARD 23)

# Determine version from latest git tag
set(HUDBOX_VERSION "0.0.0")
# Try to get the latest tag using git; fall back to default on failure
find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --abbrev=0
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_TAG
        RESULT_VARIABLE GIT_DESCRIBE_RESULT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(GIT_DESCRIBE_RESULT EQUAL 0 AND NOT GIT_TAG STREQUAL "")
        set(HUDBOX_VERSION "${GIT_TAG}")
    endif()
endif()

# Use pkg-config to find GTK, WebKitGTK, GLib, and JSON-GLib
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)
# Try WebKitGTK for GTK4 across distros: prefer webkitgtk-6.0 (24.04+), fall back to webkit2gtk-4.1, then 4.0 (22.04)
pkg_check_modules(WEBKIT webkitgtk-6.0)
if(NOT WEBKIT_FOUND)
    pkg_check_modules(WEBKIT webkit2gtk-4.1)
endif()
if(NOT WEBKIT_FOUND)
    pkg_check_modules(WEBKIT REQUIRED webkit2gtk-4.0)
endif()
pkg_check_modules(JSONGLIB REQUIRED json-glib-1.0)
pkg_check_modules(GLIB REQUIRED glib-2.0)

add_executable(hudbox
        src/main.c
        src/hb_css.c
        src/hb_config.c
        src/hb_window.c
)

# Expose version to the application as a preprocessor macro
# Example usage in C: printf("%s", HUDBOX_VERSION);
target_compile_definitions(hudbox PRIVATE HUDBOX_VERSION="${HUDBOX_VERSION}")

# Include directories
target_include_directories(hudbox PRIVATE
        src
        ${GTK4_INCLUDE_DIRS}
        ${WEBKIT_INCLUDE_DIRS}
        ${JSONGLIB_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(hudbox
        ${GTK4_LIBRARIES}
        ${WEBKIT_LIBRARIES}
        ${JSONGLIB_LIBRARIES}
)

# Add compiler flags (important for warnings and pkg-config paths)
target_compile_options(hudbox PRIVATE
        ${GTK4_CFLAGS_OTHER}
        ${WEBKIT_CFLAGS_OTHER}
        ${JSONGLIB_CFLAGS_OTHER}
)

# -------------------- Tests --------------------
enable_testing()

# Unit tests for core (GLib-based) modules
add_executable(hb_tests
        tests/test_hb_config.c
        src/hb_config.c
)

target_include_directories(hb_tests PRIVATE
        src
        ${GLIB_INCLUDE_DIRS}
        ${JSONGLIB_INCLUDE_DIRS}
)

target_link_libraries(hb_tests
        ${GLIB_LIBRARIES}
        ${JSONGLIB_LIBRARIES}
)

target_compile_options(hb_tests PRIVATE
        ${GLIB_CFLAGS_OTHER}
        ${JSONGLIB_CFLAGS_OTHER}
)

add_test(NAME hb_tests COMMAND hb_tests)

# Unit test for hb_css (GTK-only)
add_executable(hb_css_tests
        tests/test_hb_css.c
        src/hb_css.c
)

target_include_directories(hb_css_tests PRIVATE
        src
        ${GTK4_INCLUDE_DIRS}
)

target_link_libraries(hb_css_tests
        ${GTK4_LIBRARIES}
)

target_compile_options(hb_css_tests PRIVATE
        ${GTK4_CFLAGS_OTHER}
)

add_test(NAME hb_css_tests COMMAND hb_css_tests)

# Unit test for hb_window (GTK + WebKit)
add_executable(hb_window_tests
        tests/test_hb_window.c
        src/hb_window.c
        src/hb_css.c
        src/hb_config.c
)

target_include_directories(hb_window_tests PRIVATE
        src
        ${GTK4_INCLUDE_DIRS}
        ${WEBKIT_INCLUDE_DIRS}
        ${JSONGLIB_INCLUDE_DIRS}
)

target_link_libraries(hb_window_tests
        ${GTK4_LIBRARIES}
        ${WEBKIT_LIBRARIES}
        ${JSONGLIB_LIBRARIES}
)

target_compile_options(hb_window_tests PRIVATE
        ${GTK4_CFLAGS_OTHER}
        ${WEBKIT_CFLAGS_OTHER}
        ${JSONGLIB_CFLAGS_OTHER}
)

add_test(NAME hb_window_tests COMMAND hb_window_tests)

# Install rules
include(GNUInstallDirs)
install(TARGETS hudbox RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# -------------------- CPack (DEB packaging) --------------------
# Prepare a clean, numeric version for packaging (strip a leading 'v' if present)
string(REGEX REPLACE "^v" "" HUDBOX_VERSION_NUMERIC "${HUDBOX_VERSION}")

set(CPACK_PACKAGE_NAME "hudbox")
set(CPACK_PACKAGE_VENDOR "HudBox Project")
set(CPACK_PACKAGE_CONTACT "swstegall <swstegall@gmail.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "HudBox — lightweight HUD overlay using GTK4/WebKitGTK")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://swstegall.github.io/HudBox/")
set(CPACK_PACKAGE_VERSION "${HUDBOX_VERSION_NUMERIC}")

# Generator and Debian specifics
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
# Let CPack query shared library dependencies via dpkg-shlibdeps
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
# Basic Debian metadata
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

# License (if you have a LICENSE file, it will be included)
if(EXISTS "${CMAKE_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
endif()

include(CPack)
