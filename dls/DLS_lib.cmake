# -----------------------------------------------------------------------------
# DirtyLittleScript CMAKE for build as lib
# -----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.24)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(FORCE_02 OFF)
set(BUILD_WITH_FORCEO2 ${FORCE_02} CACHE BOOL "Build Application Release with -02 on gcc")
if(NOT MSVC AND BUILD_WITH_FORCEO2)
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
    set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
endif()

set (NO_MATH OFF)
set(DLS_NO_MATH ${NO_MATH} CACHE BOOL "Disable Math Functions")

set (NO_STRINGS OFF)
set(DLS_NO_STRINGS ${NO_STRINGS} CACHE BOOL "Disable String Functions")

set (NO_HELP OFF)
set(DLS_NO_HELP ${NO_HELP} CACHE BOOL "Disable Help Functions")


if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DLS_DEBUG)
endif()

set(DLS_SRC
    # main
    ${CMAKE_CURRENT_LIST_DIR}/DirtyLittleScript.h
    ${CMAKE_CURRENT_LIST_DIR}/DirtyLittleScript.cpp

    # core
    ${CMAKE_CURRENT_LIST_DIR}/core/ArenaAllocator.h
    ${CMAKE_CURRENT_LIST_DIR}/core/FunctionMap.h
    ${CMAKE_CURRENT_LIST_DIR}/core/Globals.h
    ${CMAKE_CURRENT_LIST_DIR}/core/Lexer.h
    ${CMAKE_CURRENT_LIST_DIR}/core/Lexer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/core/Marshall.h
    ${CMAKE_CURRENT_LIST_DIR}/core/Parser.h
    ${CMAKE_CURRENT_LIST_DIR}/core/Parser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/core/Token.h
    ${CMAKE_CURRENT_LIST_DIR}/core/Token.cpp
    ${CMAKE_CURRENT_LIST_DIR}/core/Value.h
    ${CMAKE_CURRENT_LIST_DIR}/core/Value.cpp
    ${CMAKE_CURRENT_LIST_DIR}/core/VM.h
    ${CMAKE_CURRENT_LIST_DIR}/core/VM.cpp


    # ToolBox
    ${CMAKE_CURRENT_LIST_DIR}/toolbox/SymbolTable.h
    ${CMAKE_CURRENT_LIST_DIR}/toolbox/Tools.h

    # Extras
    ${CMAKE_CURRENT_LIST_DIR}/extras/Expressions.h
    ${CMAKE_CURRENT_LIST_DIR}/extras/Expressions.cpp
)


# Functions
if (DLS_NO_MATH)
    add_compile_definitions(DLS_NO_MATH)
else()
    list (APPEND DLS_SRC
        ${CMAKE_CURRENT_LIST_DIR}/functions/MathFunctions.h
        ${CMAKE_CURRENT_LIST_DIR}/functions/MathFunctions.cpp
    )
endif()

if (DLS_NO_STRINGS)
    add_compile_definitions(DLS_NO_STRINGS)
else()
    list (APPEND DLS_SRC
        ${CMAKE_CURRENT_LIST_DIR}/functions/StringFunctions.h
        ${CMAKE_CURRENT_LIST_DIR}/functions/StringFunctions.cpp
    )
endif()

if (DLS_NO_HELP)
    add_compile_definitions(DLS_NO_HELP)
else()
    list (APPEND DLS_SRC
        ${CMAKE_CURRENT_LIST_DIR}/functions/HelpFunctions.h
        ${CMAKE_CURRENT_LIST_DIR}/functions/HelpFunctions.cpp
    )
endif()



add_library(DirtyLittleScript
    ${DLS_SRC}
)

target_include_directories(DirtyLittleScript PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)
