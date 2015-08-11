cmake_minimum_required(VERSION 3.2)
project(zhsh)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(Readline REQUIRED)
include_directories(${READLINE_INCLUDE_DIR})
find_package(FLEX REQUIRED)
find_package(LEMON REQUIRED)

LEMON_TARGET(zhsh_line_parser line_parser.y ${PROJECT_SOURCE_DIR}/line_parser.c ${PROJECT_SOURCE_DIR}/line_parser.h)
# Debug
#FLEX_TARGET(zhsh_line_lexer line_lexer.l ${PROJECT_SOURCE_DIR}/line_lexer.c COMPILE_FLAGS "--header-file=${PROJECT_SOURCE_DIR}/line_lexer.h --debug")
FLEX_TARGET(zhsh_line_lexer line_lexer.l ${PROJECT_SOURCE_DIR}/line_lexer.c COMPILE_FLAGS "--header-file=${PROJECT_SOURCE_DIR}/line_lexer.h")

set(SOURCE_FILES shell.h shell.c line_syntax.h line_syntax.c parser.h parser.c util.h util.c quirk/quirk.h quirk/quirk.c quirk/echo.h quirk/echo.c quirk/env.h quirk/env.c quirk/help.h quirk/help.c quirk/ls.h quirk/ls.c)
set_source_files_properties(${LEMON_zhsh_line_parser_OUTPUTS} ${FLEX_zhsh_line_lexer_OUTPUTS} PROPERTIES COMPILE_FLAGS -Wno-unused)
add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_FILES} ${LEMON_zhsh_line_parser_OUTPUTS} ${FLEX_zhsh_line_lexer_OUTPUTS})
target_link_libraries(${CMAKE_PROJECT_NAME} ${READLINE_LIBRARY} ${NCURSES_LIBRARY})
