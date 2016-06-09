cmake_minimum_required(VERSION 2.8)

project(ikbus-apps)

find_package(PkgConfig)
pkg_check_modules(GIO REQUIRED gio-unix-2.0)
pkg_check_modules(PLAYERCTL REQUIRED playerctl-1.0)
include_directories(include ${GIO_INCLUDE_DIRS} ${PLAYERCTL_INCLUDE_DIRS} ikbus-gobjects)

add_executable(cdc-agent apps/cdc-agent.c)

add_subdirectory(ikbus-gobjects)

target_link_libraries(cdc-agent ${GIO_LIBRARIES} ${PLAYERCTL_LIBRARIES} ikbus-gobjects)
