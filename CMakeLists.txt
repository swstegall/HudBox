cmake_minimum_required(VERSION 3.31.6)
project(HudBox C)

set(CMAKE_C_STANDARD 23)

# Use pkg-config to find GTK, WebKitGTK, GLib, and JSON-GLib
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)
pkg_check_modules(WEBKIT REQUIRED webkitgtk-6.0)
pkg_check_modules(JSONGLIB REQUIRED json-glib-1.0)
pkg_check_modules(GLIB REQUIRED glib-2.0)

add_executable(hudbox
        src/main.c
        src/hb_css.c
        src/hb_config.c
        src/hb_window.c
)

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

# Install rules
include(GNUInstallDirs)
install(TARGETS hudbox RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
