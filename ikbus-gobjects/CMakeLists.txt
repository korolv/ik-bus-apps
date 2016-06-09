cmake_minimum_required(VERSION 2.8)

project(ikbus-gobjects)

set(SOURCE_LIB ikbussocket ikbuscdc)

find_package(PkgConfig)
pkg_check_modules(GIO REQUIRED gio-unix-2.0)
include_directories(../include ${GIO_INCLUDE_DIRS})

add_library(ikbus-gobjects STATIC ${SOURCE_LIB})
