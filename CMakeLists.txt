cmake_minimum_required(VERSION 3.10)

# Project name
project(DBZBT3)

# Check compiler and set constant
if ((${CMAKE_CXX_COMPILER_ID} STREQUAL GNU AND NOT CYGWIN) OR ${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS 1)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
    endif ()
else ()
    message(FATAL_ERROR "Unsupported compiler!")
endif ()

# Define Qt path
set(QT_PATH_FILE "CMakeQtPath.txt")
if (EXISTS ${PROJECT_SOURCE_DIR}/${QT_PATH_FILE})
    file(STRINGS ${QT_PATH_FILE} CMAKE_PREFIX_PATH)
else ()
    file(WRITE ${QT_PATH_FILE})
    message(FATAL_ERROR "Qt path non set\nSet it manually in ${QT_PATH_FILE}")
endif ()

if (EXISTS ${CMAKE_PREFIX_PATH})
    message(STATUS "Qt path: ${CMAKE_PREFIX_PATH}")
else ()
    message(FATAL_ERROR "Qt path ${CMAKE_PREFIX_PATH} not exists!\nSet a valid one in ${QT_PATH_FILE}")
endif ()

# Define debug postfix/suffix
string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_UPPER)
if (${CMAKE_BUILD_TYPE_UPPER} STREQUAL DEBUG)
    set(CMAKE_EXECUTABLE_SUFFIX d${CMAKE_EXECUTABLE_SUFFIX})
    set(CMAKE_DEBUG_POSTFIX d)
    add_definitions(-DDBZBT3_DEBUG)
endif ()

if (WIN32 AND NOT (${CMAKE_BUILD_TYPE_UPPER} STREQUAL DEBUG))
    message(STATUS "Setting Windows executable")
    set(CMAKE_WIN32_EXECUTABLE ON)
endif ()

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# QT += widgets
find_package(Qt5 COMPONENTS Widgets REQUIRED)

# Define output path
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/Binaries/${CMAKE_CXX_COMPILER_ID})
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/Binaries/${CMAKE_CXX_COMPILER_ID})

# Define various path
set(SHARED_PATH Libraries/Shared)
set(DIALOG_PATH Libraries/Dialog)
set(AFL_CORE_PATH Libraries/AFLCore)
set(AFS_CORE_PATH Libraries/AFSCore)
set(CTE_CORE_PATH Libraries/CTECore)
set(AFL_CONVERTER_PATH AFL-Converter)
set(AFS_MANAGER_PATH AFS-Manager)
set(CTE_PLUGIN_PATH CTE-Plugin)

# Define Shared source code
set(SHARED_SOURCES ${SHARED_PATH}/Shared.cpp)
set(SHARED_RESOURCES ${SHARED_PATH}/Shared.rc)

# Define Dialog source code
set(DIALOG_SOURCES ${DIALOG_PATH}/ButtonEvent.cpp ${DIALOG_PATH}/Dialog.cpp ${DIALOG_PATH}/Message.cpp ${DIALOG_PATH}/Progress.cpp ${DIALOG_PATH}/Warning.cpp)
set(DIALOG_RESOURCES ${DIALOG_PATH}/Dialog.rc ${DIALOG_PATH}/Dialog.qrc)

# Define AFL source code
set(AFL_CONVERTER_SOURCES ${AFL_CONVERTER_PATH}/Main.cpp)
set(AFL_CONVERTER_RESOURCES ${AFL_CONVERTER_PATH}/AFL-Converter.rc)
set(AFL_CORE_SOURCES ${AFL_CORE_PATH}/AFLCore.cpp)
set(AFL_CORE_RESOURCES ${AFL_CORE_PATH}/AFLCore.rc)

# Define AFS source code
set(AFS_MANAGER_SOURCES ${AFS_MANAGER_PATH}/Main.cpp ${AFS_MANAGER_PATH}/MainWindow.cpp ${AFS_MANAGER_PATH}/Unpacker.cpp)
set(AFS_MANAGER_RESOURCES ${AFS_MANAGER_PATH}/AFS-Manager.rc ${AFS_MANAGER_PATH}/MainWindow.qrc)
set(AFS_CORE_SOURCES ${AFS_CORE_PATH}/AFSCore.cpp)
set(AFS_CORE_RESOURCES ${AFS_CORE_PATH}/AFSCore.rc)

# Define CTE source code
set(CTE_PLUGIN_SOURCES ${CTE_PLUGIN_PATH}/Main.cpp)
set(CTE_PLUGIN_RESOURCES ${CTE_PLUGIN_PATH}/CTE-Plugin.rc)
set(CTE_CORE_SOURCES ${CTE_CORE_PATH}/CTECore.cpp)
set(CTE_CORE_RESOURCES ${CTE_CORE_PATH}/CTECore.rc)

# Define forms paths
set(CMAKE_AUTOUIC_SEARCH_PATHS ${DIALOG_PATH} ${AFS_MANAGER_PATH})

# Shared
add_library(Shared SHARED ${SHARED_SOURCES} ${SHARED_RESOURCES})
if (MINGW)
    message(STATUS "Including psapi library")
    target_link_libraries(Shared psapi)
endif ()

# Dialog
add_library(Dialog SHARED ${DIALOG_SOURCES} ${DIALOG_RESOURCES})
target_link_libraries(Dialog Qt5::Widgets)

# AFLCore
add_library(AFLCore SHARED ${AFL_CORE_SOURCES} ${AFL_CORE_RESOURCES})
target_link_libraries(AFLCore Shared)

# AFSCore
add_library(AFSCore SHARED ${AFS_CORE_SOURCES} ${AFS_CORE_RESOURCES})
target_link_libraries(AFSCore AFLCore)

# CTECore
add_library(CTECore SHARED ${CTE_CORE_SOURCES} ${CTE_CORE_RESOURCES})
target_link_libraries(CTECore Shared)

# AFL-Converter
add_executable(AFL-Converter ${AFL_CONVERTER_SOURCES} ${AFL_CONVERTER_RESOURCES})
target_link_libraries(AFL-Converter AFLCore Dialog)

# AFS-Manager
add_executable(AFS-Manager ${AFS_MANAGER_SOURCES} ${AFS_MANAGER_RESOURCES})
target_link_libraries(AFS-Manager AFSCore Dialog)

# CTE-Plugin
add_executable(CTE-Plugin ${CTE_PLUGIN_SOURCES} ${CTE_PLUGIN_RESOURCES})
target_link_libraries(CTE-Plugin CTECore Dialog)
