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

# Check build arch
if (${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    set(BUILD_ARCH "x64")
else ()
    set(BUILD_ARCH "x86")
endif ()

# Define Qt path
set(QT_PATH_FILE CMakeQtPath${CMAKE_SYSTEM_NAME}${CMAKE_CXX_COMPILER_ID}${BUILD_ARCH}.txt)
if ("${CMAKE_PREFIX_PATH}" STREQUAL "")
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

if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE Default)
endif ()

# Define debug postfix/suffix
string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)
if ("${CMAKE_BUILD_TYPE_UPPER}" STREQUAL DEBUG)
    set(CMAKE_EXECUTABLE_SUFFIX d${CMAKE_EXECUTABLE_SUFFIX})
    set(CMAKE_DEBUG_POSTFIX d)
    add_definitions(-DDBZBT3_DEBUG)
endif ()

if (WIN32 AND NOT ("${CMAKE_BUILD_TYPE_UPPER}" STREQUAL DEBUG))
    set(CMAKE_WIN32_EXECUTABLE ON)
endif ()

message(STATUS "Target: ${CMAKE_SYSTEM_NAME} ${BUILD_ARCH}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_BUILD_TYPE}")

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# QT += widgets
find_package(Qt5 COMPONENTS Widgets REQUIRED)

# Define output path
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/Binaries/${CMAKE_SYSTEM_NAME}/${CMAKE_BUILD_TYPE}/${CMAKE_CXX_COMPILER_ID}/${BUILD_ARCH}")
set(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/Binaries/${CMAKE_SYSTEM_NAME}/${CMAKE_BUILD_TYPE}/${CMAKE_CXX_COMPILER_ID}/${BUILD_ARCH}")

# Define Shared paths
set(SHARED_PATH Libraries/Shared)
set(SHARED_FORMS_PATH ${SHARED_PATH}/Forms)
set(SHARED_HEADERS_PATH ${SHARED_PATH}/Headers)
set(SHARED_RESOURCES_PATH ${SHARED_PATH}/Resources)
set(SHARED_SOURCES_PATH ${SHARED_PATH}/Sources)

# Define MessageBox paths
set(MESSAGEBOX_PATH Libraries/MessageBox)
set(MESSAGEBOX_FORMS_PATH ${MESSAGEBOX_PATH}/Forms)
set(MESSAGEBOX_HEADERS_PATH ${MESSAGEBOX_PATH}/Headers)
set(MESSAGEBOX_RESOURCES_PATH ${MESSAGEBOX_PATH}/Resources)
set(MESSAGEBOX_SOURCES_PATH ${MESSAGEBOX_PATH}/Sources)

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

# TODO: Define CTECore paths

# Define AFL-Converter paths
set(AFL_CONVERTER_PATH AFL-Converter)
set(AFL_CONVERTER_FORMS_PATH ${AFL_CONVERTER_PATH}/Forms)
set(AFL_CONVERTER_HEADERS_PATH ${AFL_CONVERTER_PATH}/Headers)
set(AFL_CONVERTER_RESOURCES_PATH ${AFL_CONVERTER_PATH}/Resources)
set(AFL_CONVERTER_SOURCES_PATH ${AFL_CONVERTER_PATH}/Sources)

# Define AFS-Manager paths
set(AFS_MANAGER_PATH AFS-Manager)
set(AFS_MANAGER_FORMS_PATH ${AFS_MANAGER_PATH}/Forms)
set(AFS_MANAGER_HEADERS_PATH ${AFS_MANAGER_PATH}/Headers)
set(AFS_MANAGER_RESOURCES_PATH ${AFS_MANAGER_PATH}/Resources)
set(AFS_MANAGER_SOURCES_PATH ${AFS_MANAGER_PATH}/Sources)

# TODO: Define CTE-Plugin paths

# Define various paths
include_directories(${SHARED_RESOURCES_PATH} ${MESSAGEBOX_RESOURCES_PATH})
set(CMAKE_AUTOUIC_SEARCH_PATHS ${AFS_MANAGER_FORMS_PATH})

# Define Shared files
set(SHARED_RESOURCES ${SHARED_RESOURCES_PATH}/Shared.rc)
set(SHARED_SOURCES ${SHARED_SOURCES_PATH}/Shared.cpp)

# Define MessageBox files
set(MESSAGEBOX_RESOURCES ${MESSAGEBOX_RESOURCES_PATH}/MessageBox.rc ${MESSAGEBOX_RESOURCES_PATH}/MessageBox.qrc)
set(MESSAGEBOX_SOURCES ${MESSAGEBOX_SOURCES_PATH}/MessageBox.cpp)

# Define AFLCore files
set(AFLCORE_RESOURCES ${AFLCORE_RESOURCES_PATH}/AFLCore.rc)
set(AFLCORE_SOURCES ${AFLCORE_SOURCES_PATH}/AFLCore.cpp)

# Define AFSCore files
set(AFSCORE_RESOURCES ${AFSCORE_RESOURCES_PATH}/AFSCore.rc)
set(AFSCORE_SOURCES ${AFSCORE_SOURCES_PATH}/AFSCore.cpp)

# Define AFL-Converter files
set(AFL_CONVERTER_RESOURCES ${AFL_CONVERTER_RESOURCES_PATH}/AFL-Converter.rc)
set(AFL_CONVERTER_SOURCES ${AFL_CONVERTER_SOURCES_PATH}/Main.cpp)

# Define AFS-Manager files
set(AFS_MANAGER_Q_OBJECTS ${AFS_MANAGER_HEADERS_PATH}/MainWindow.h ${AFS_MANAGER_HEADERS_PATH}/AboutDialog.h ${AFS_MANAGER_HEADERS_PATH}/ProgressDialog.h ${AFS_MANAGER_HEADERS_PATH}/ReservedSpaceDialog.h ${AFS_MANAGER_HEADERS_PATH}/Worker.h)
set(AFS_MANAGER_FORMS ${AFS_MANAGER_FORMS_PATH}/MainWindow.ui ${AFS_MANAGER_FORMS_PATH}/AboutDialog.ui ${AFS_MANAGER_FORMS_PATH}/ProgressDialog.ui ${AFS_MANAGER_FORMS_PATH}/ReservedSpaceDialog.ui)
set(AFS_MANAGER_RESOURCES ${AFS_MANAGER_RESOURCES_PATH}/AFS-Manager.rc ${AFS_MANAGER_RESOURCES_PATH}/MainWindow.qrc)
set(AFS_MANAGER_SOURCES ${AFS_MANAGER_SOURCES_PATH}/Main.cpp ${AFS_MANAGER_SOURCES_PATH}/MainWindow.cpp ${AFS_MANAGER_SOURCES_PATH}/AboutDialog.cpp ${AFS_MANAGER_SOURCES_PATH}/ProgressDialog.cpp ${AFS_MANAGER_SOURCES_PATH}/ReservedSpaceDialog.cpp ${AFS_MANAGER_SOURCES_PATH}/TableWidgetItem.cpp ${AFS_MANAGER_SOURCES_PATH}/Worker.cpp)

# TODO: Define CTE-Plugin files

# Shared
add_library(Shared SHARED ${SHARED_SOURCES} ${SHARED_RESOURCES})
target_include_directories(Shared PUBLIC ${SHARED_HEADERS_PATH})
if (MINGW)
    target_link_libraries(Shared psapi)
endif ()

# MessageBox
add_library(MessageBox SHARED ${MESSAGEBOX_SOURCES} ${MESSAGEBOX_RESOURCES})
target_include_directories(MessageBox PUBLIC ${MESSAGEBOX_HEADERS_PATH})
target_link_libraries(MessageBox Qt5::Widgets)

# AFLCore
add_library(AFLCore SHARED ${AFLCORE_SOURCES} ${AFLCORE_RESOURCES})
target_include_directories(AFLCore PUBLIC ${AFLCORE_HEADERS_PATH})
target_link_libraries(AFLCore Shared)

# AFSCore
add_library(AFSCore SHARED ${AFSCORE_SOURCES} ${AFSCORE_RESOURCES})
target_include_directories(AFSCore PUBLIC ${AFSCORE_HEADERS_PATH})
target_link_libraries(AFSCore AFLCore)

# TODO: CTECore

# AFL-Converter
add_executable(AFL-Converter ${AFL_CONVERTER_SOURCES} ${AFL_CONVERTER_RESOURCES})
target_include_directories(AFL-Converter PUBLIC ${AFL_CONVERTER_HEADERS_PATH})
target_link_libraries(AFL-Converter MessageBox AFLCore)

# AFS-Manager
add_executable(AFS-Manager ${AFS_MANAGER_Q_OBJECTS} ${AFS_MANAGER_FORMS} ${AFS_MANAGER_SOURCES} ${AFS_MANAGER_RESOURCES})
target_include_directories(AFS-Manager PUBLIC ${AFS_MANAGER_HEADERS_PATH})
target_link_libraries(AFS-Manager MessageBox AFSCore)

# TODO: CTE-Plugin
