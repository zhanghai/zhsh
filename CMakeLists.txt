cmake_minimum_required(VERSION 3.2)
project(zhsh)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(Readline REQUIRED)
include_directories(${READLINE_INCLUDE_DIR})

set(SOURCE_FILES main.c util.h util.c)
add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${CMAKE_PROJECT_NAME} ${READLINE_LIBRARY} ${NCURSES_LIBRARY})
