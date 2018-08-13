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
set(QT_PATH_FILE CMakeQtPath${CMAKE_CXX_COMPILER_ID}.txt)
if ("${CMAKE_PREFIX_PATH}" STREQUAL "")
    message(STATUS "Checking Qt path")
    if (EXISTS ${PROJECT_SOURCE_DIR}/${QT_PATH_FILE})
        file(STRINGS ${QT_PATH_FILE} CMAKE_PREFIX_PATH)
    else ()
        file(WRITE ${QT_PATH_FILE})
        message(FATAL_ERROR "Qt path non set\nSet it manually in ${QT_PATH_FILE}")
    endif ()
endif ()

if (EXISTS ${CMAKE_PREFIX_PATH})
    message(STATUS "Qt path: ${CMAKE_PREFIX_PATH}")
else ()
    if (NOT EXISTS ${PROJECT_SOURCE_DIR}/${QT_PATH_FILE})
        file(WRITE ${QT_PATH_FILE})
    endif ()
    message(FATAL_ERROR "Qt path ${CMAKE_PREFIX_PATH} not exists!\nSet a valid one in ${QT_PATH_FILE}")
endif ()

# Define debug postfix/suffix
string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)
if ("${CMAKE_BUILD_TYPE_UPPER}" STREQUAL DEBUG)
    set(CMAKE_EXECUTABLE_SUFFIX d${CMAKE_EXECUTABLE_SUFFIX})
    set(CMAKE_DEBUG_POSTFIX d)
    add_definitions(-DDBZBT3_DEBUG)
endif ()

if (WIN32 AND NOT ("${CMAKE_BUILD_TYPE_UPPER}" STREQUAL DEBUG))
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

# Define Shared paths
set(SHARED_PATH Libraries/Shared)
set(SHARED_FORMS_PATH ${SHARED_PATH}/Forms)
set(SHARED_HEADERS_PATH ${SHARED_PATH}/Headers)
set(SHARED_RESOURCES_PATH ${SHARED_PATH}/Resources)
set(SHARED_SOURCES_PATH ${SHARED_PATH}/Sources)

# Define AFLCore paths
set(AFLCORE_PATH Libraries/AFLCore)
set(AFLCORE_FORMS_PATH ${AFLCORE_PATH}/Forms)
set(AFLCORE_HEADERS_PATH ${AFLCORE_PATH}/Headers)
set(AFLCORE_RESOURCES_PATH ${AFLCORE_PATH}/Resources)
set(AFLCORE_SOURCES_PATH ${AFLCORE_PATH}/Sources)

# Define AFSCore paths
set(AFSCORE_PATH Libraries/AFSCore)
set(AFSCORE_FORMS_PATH ${AFSCORE_PATH}/Forms)
set(AFSCORE_HEADERS_PATH ${AFSCORE_PATH}/Headers)
set(AFSCORE_RESOURCES_PATH ${AFSCORE_PATH}/Resources)
set(AFSCORE_SOURCES_PATH ${AFSCORE_PATH}/Sources)

# Define AFL-Converter paths
set(AFL_CONVERTER_PATH AFL-Converter)
set(AFL_CONVERTER_FORMS_PATH ${AFL_CONVERTER_PATH}/Forms)
set(AFL_CONVERTER_HEADERS_PATH ${AFL_CONVERTER_PATH}/Headers)
set(AFL_CONVERTER_RESOURCES_PATH ${AFL_CONVERTER_PATH}/Resources)
set(AFL_CONVERTER_SOURCES_PATH ${AFL_CONVERTER_PATH}/Sources)

#set(AFS_MANAGER_PATH AFS-Manager)
#set(CTE_PLUGIN_PATH CTE-Plugin)

# Define various paths
include_directories(${SHARED_RESOURCES_PATH})
set(CMAKE_AUTOUIC_SEARCH_PATHS ${SHARED_FORMS_PATH})

# Define Shared files
set(SHARED_Q_OBJECTS ${SHARED_HEADERS_PATH}/Dialog.h)
set(SHARED_RESOURCES ${SHARED_RESOURCES_PATH}/Dialog.qrc ${SHARED_RESOURCES_PATH}/Shared.rc)
set(SHARED_SOURCES ${SHARED_SOURCES_PATH}/Dialog.cpp ${SHARED_SOURCES_PATH}/Shared.cpp)

# Define AFLCore files
set(AFLCORE_Q_OBJECTS)
set(AFLCORE_SOURCES ${AFLCORE_SOURCES_PATH}/AFLCore.cpp)
set(AFLCORE_RESOURCES ${AFLCORE_RESOURCES_PATH}/AFLCore.rc)

# Define AFSCore files
set(AFSCORE_Q_OBJECTS)
set(AFSCORE_SOURCES ${AFSCORE_SOURCES_PATH}/AFSCore.cpp)
set(AFSCORE_RESOURCES ${AFSCORE_RESOURCES_PATH}/AFSCore.rc)

# Define AFL-Converter files
set(SHARED_Q_OBJECTS ${SHARED_HEADERS_PATH}/Dialog.h)
set(AFL_CONVERTER_SOURCES ${AFL_CONVERTER_SOURCES_PATH}/Main.cpp)
set(AFL_CONVERTER_RESOURCES ${AFL_CONVERTER_RESOURCES_PATH}/AFL-Converter.rc)

#[[
# Define AFS source code
set(AFS_MANAGER_SOURCES ${AFS_MANAGER_PATH}/Main.cpp ${AFS_MANAGER_PATH}/MainWindow.cpp ${AFS_MANAGER_PATH}/Unpacker.cpp)
set(AFS_MANAGER_RESOURCES ${AFS_MANAGER_PATH}/AFS-Manager.rc ${AFS_MANAGER_PATH}/MainWindow.qrc)

# Define CTE source code
set(CTE_PLUGIN_SOURCES ${CTE_PLUGIN_PATH}/Main.cpp)
set(CTE_PLUGIN_RESOURCES ${CTE_PLUGIN_PATH}/CTE-Plugin.rc)
set(CTE_CORE_SOURCES ${CTE_CORE_PATH}/CTECore.cpp)
set(CTE_CORE_RESOURCES ${CTE_CORE_PATH}/CTECore.rc)
]]

# Shared
set(SHARED_LIBRARIES Qt5::Widgets)
if (MINGW)
    set(SHARED_LIBRARIES ${SHARED_LIBRARIES} psapi)
endif ()
add_library(Shared SHARED ${SHARED_SOURCES} ${SHARED_RESOURCES} ${SHARED_Q_OBJECTS})
target_include_directories(Shared PUBLIC ${SHARED_HEADERS_PATH})
target_link_libraries(Shared ${SHARED_LIBRARIES})

# AFLCore
add_library(AFLCore SHARED ${AFLCORE_SOURCES} ${AFLCORE_RESOURCES})
target_include_directories(AFLCore PUBLIC ${AFLCORE_HEADERS_PATH})
target_link_libraries(AFLCore Shared)

# AFSCore
add_library(AFSCore SHARED ${AFSCORE_SOURCES} ${AFSCORE_RESOURCES})
target_include_directories(AFSCore PUBLIC ${AFSCORE_HEADERS_PATH})
target_link_libraries(AFSCore AFLCore)

# AFL-Converter
add_executable(AFL-Converter ${AFL_CONVERTER_SOURCES} ${AFL_CONVERTER_RESOURCES})
target_include_directories(AFL-Converter PUBLIC ${AFL_CONVERTER_HEADERS_PATH})
target_link_libraries(AFL-Converter AFLCore)

#[[
# CTECore
add_library(CTECore SHARED ${CTE_CORE_SOURCES} ${CTE_CORE_RESOURCES})
target_link_libraries(CTECore Shared)

# AFS-Manager
add_executable(AFS-Manager ${AFS_MANAGER_SOURCES} ${AFS_MANAGER_RESOURCES})
target_link_libraries(AFS-Manager AFSCore Dialog)

# CTE-Plugin
add_executable(CTE-Plugin ${CTE_PLUGIN_SOURCES} ${CTE_PLUGIN_RESOURCES})
target_link_libraries(CTE-Plugin CTECore Dialog)
]]
