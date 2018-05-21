cmake_minimum_required(VERSION 3.10)

# Project name
project(DBZBT3)

set(QT_VERSION 5.10.1)

# Define Qt path
if ((${CMAKE_CXX_COMPILER_ID} STREQUAL GNU AND NOT CYGWIN) OR ${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
        set(CMAKE_PREFIX_PATH "C:\\Qt\\${QT_VERSION}\\msvc2015")
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS 1)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
    elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
        if (WIN32)
            set(CMAKE_PREFIX_PATH "C:\\Qt\\${QT_VERSION}\\mingw53_32")
        else ()
            set(CMAKE_PREFIX_PATH "$ENV{HOME}/Qt/${QT_VERSION}/gcc_64")
        endif ()
    endif ()
else ()
    message(FATAL_ERROR "Unsupported compiler!")
endif ()

if (EXISTS ${CMAKE_PREFIX_PATH})
    message(STATUS "Qt path: ${CMAKE_PREFIX_PATH}")
else ()
    message(FATAL_ERROR "Qt path not exists!\nSet it manually to continue")
endif ()

if (WIN32)
    message(STATUS "Setting Windows executable")
    set(CMAKE_WIN32_EXECUTABLE ON)
endif ()

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Define debug postfix
set(CMAKE_DEBUG_POSTFIX d)

# QT += widgets
find_package(Qt5 COMPONENTS Widgets REQUIRED)

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
#set(SHARED_Q_OBJECT_HEADERS)
set(SHARED_SOURCES ${SHARED_PATH}/Shared.cpp)
set(SHARED_RESOURCES ${SHARED_PATH}/Shared.rc)

# Define Dialog source code
set(DIALOG_Q_OBJECT_HEADERS ${DIALOG_PATH}/Dialog.h)
set(DIALOG_SOURCES ${DIALOG_PATH}/Dialog.cpp)
set(DIALOG_RESOURCES ${DIALOG_PATH}/Dialog.rc ${DIALOG_PATH}/Dialog.qrc)

# Define AFL source code
#set(AFL_CONVERTER_Q_OBJECT_HEADERS)
set(AFL_CONVERTER_SOURCES ${AFL_CONVERTER_PATH}/Main.cpp)
set(AFL_CONVERTER_RESOURCES ${AFL_CONVERTER_PATH}/AFL-Converter.rc)
#set(AFL_CORE_Q_OBJECT_HEADERS)
set(AFL_CORE_SOURCES ${AFL_CORE_PATH}/AFLCore.cpp)
set(AFL_CORE_RESOURCES ${AFL_CORE_PATH}/AFLCore.rc)

# Define AFS source code
set(AFS_MANAGER_Q_OBJECT_HEADERS ${AFS_MANAGER_PATH}/MainWindow.h ${AFS_MANAGER_PATH}/Unpacker.h)
set(AFS_MANAGER_SOURCES ${AFS_MANAGER_PATH}/Main.cpp ${AFS_MANAGER_PATH}/MainWindow.cpp ${AFS_MANAGER_PATH}/Unpacker.cpp)
set(AFS_MANAGER_RESOURCES ${AFS_MANAGER_PATH}/AFS-Manager.rc ${AFS_MANAGER_PATH}/MainWindow.qrc)
#set(AFS_CORE_Q_OBJECT_HEADERS)
set(AFS_CORE_SOURCES ${AFS_CORE_PATH}/AFSCore.cpp)
set(AFS_CORE_RESOURCES ${AFS_CORE_PATH}/AFSCore.rc)

# Define CTE source code
#set(CTE_PLUGIN_Q_OBJECT_HEADERS)
set(CTE_PLUGIN_SOURCES ${CTE_PLUGIN_PATH}/Main.cpp)
set(CTE_PLUGIN_RESOURCES ${CTE_PLUGIN_PATH}/CTE-Plugin.rc)
#set(CTE_CORE_Q_OBJECT_HEADERS)
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
add_library(Dialog SHARED ${DIALOG_Q_OBJECT_HEADERS} ${DIALOG_SOURCES} ${DIALOG_RESOURCES})
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
add_executable(AFS-Manager ${AFS_MANAGER_Q_OBJECT_HEADERS} ${AFS_MANAGER_SOURCES} ${AFS_MANAGER_RESOURCES})
target_link_libraries(AFS-Manager AFSCore Dialog)

# CTE-Plugin
add_executable(CTE-Plugin ${CTE_PLUGIN_SOURCES} ${CTE_PLUGIN_RESOURCES})
target_link_libraries(CTE-Plugin CTECore Dialog)
