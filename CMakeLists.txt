cmake_minimum_required(VERSION 3.10)
project(fr-browser VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Find required packages
find_package(PkgConfig REQUIRED)

# GTK3, WebKit2GTK, and JSON-GLib
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

# Try WebKit2GTK 4.1 first, then fall back to 4.0
pkg_check_modules(WEBKIT2 webkit2gtk-4.1)
if(NOT WEBKIT2_FOUND)
    pkg_check_modules(WEBKIT2 REQUIRED webkit2gtk-4.0)
endif()

pkg_check_modules(JSON_GLIB REQUIRED json-glib-1.0)

# Include directories
include_directories(${GTK3_INCLUDE_DIRS})
include_directories(${WEBKIT2_INCLUDE_DIRS})
include_directories(${JSON_GLIB_INCLUDE_DIRS})
include_directories(src)

# Compiler flags
add_definitions(${GTK3_CFLAGS_OTHER})
add_definitions(${WEBKIT2_CFLAGS_OTHER})
add_definitions(${JSON_GLIB_CFLAGS_OTHER})

# Source files
set(SOURCES
    src/main.c
    src/browser.c
    src/window.c
    src/tabs.c
    src/bookmarks.c
    src/history.c
    src/utils.c
)

# Headers
set(HEADERS
    src/browser.h
    src/window.h
    src/tabs.h
    src/bookmarks.h
    src/history.h
    src/utils.h
)

# Create executable
add_executable(fr-browser ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(fr-browser 
    ${GTK3_LIBRARIES}
    ${WEBKIT2_LIBRARIES}
    ${JSON_GLIB_LIBRARIES}
)

# Compiler and linker flags
target_compile_options(fr-browser PRIVATE ${GTK3_CFLAGS} ${WEBKIT2_CFLAGS} ${JSON_GLIB_CFLAGS})
target_link_directories(fr-browser PRIVATE ${GTK3_LIBRARY_DIRS} ${WEBKIT2_LIBRARY_DIRS} ${JSON_GLIB_LIBRARY_DIRS})

# Install target
install(TARGETS fr-browser DESTINATION bin)

# Desktop file
configure_file(
    ${CMAKE_SOURCE_DIR}/fr-browser.desktop.in
    ${CMAKE_BINARY_DIR}/fr-browser.desktop
    @ONLY
)

install(FILES ${CMAKE_BINARY_DIR}/fr-browser.desktop 
        DESTINATION share/applications)

# Icon
install(FILES icons/fr-browser.png 
        DESTINATION share/pixmaps)
