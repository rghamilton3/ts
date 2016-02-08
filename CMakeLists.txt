cmake_minimum_required(VERSION 2.6)
project(ts)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)
find_library(NCURSES_PANEL_LIBRARY NAMES panel)
find_library(NCURSES_MENU_LIBRARY NAMES menu)

set(SOURCE_FILES main.c ts.h)
add_executable(ts ${SOURCE_FILES})
target_link_libraries(ts ${CURSES_LIBRARIES} ${NCURSES_PANEL_LIBRARY} ${NCURSES_MENU_LIBRARY})