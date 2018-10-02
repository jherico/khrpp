# If we're running under the gradle build, HIFI_ANDROID will be set here, but
# ANDROID will not be set until after the `project` statement.  This is the *ONLY*
# place you need to use `HIFI_ANDROID` instead of `ANDROID`
cmake_minimum_required(VERSION 3.11)

project(hkrpp)

include("cmake/init.cmake")

include("cmake/compiler.cmake")

set(BUILD_CLIENT_OPTION ON)
set(BUILD_SERVER_OPTION ON)
set(BUILD_TESTS_OPTION ON)
set(BUILD_TOOLS_OPTION ON)
set(BUILD_INSTALLER_OPTION ON)
set(GLES_OPTION OFF)
set(DISABLE_QML_OPTION OFF)
set(DOWNLOAD_SERVERLESS_CONTENT_OPTION OFF)

if (ANDROID OR UWP)
  set(BUILD_SERVER_OPTION OFF)
  set(BUILD_TOOLS_OPTION OFF)
  set(BUILD_INSTALLER OFF)
endif()

if (CLIENT_ONLY)
  set(BUILD_SERVER_OPTION OFF)
endif()

if (SERVER_ONLY)
  set(BUILD_CLIENT_OPTION OFF)
  set(BUILD_TESTS_OPTION OFF)
endif()

if (ANDROID)
  set(GLES_OPTION ON)
  set(PLATFORM_QT_COMPONENTS AndroidExtras WebView)
else ()
  set(PLATFORM_QT_COMPONENTS WebEngine WebEngineWidgets)
endif ()

if (USE_GLES AND (NOT ANDROID))
  set(DISABLE_QML_OPTION ON)
endif()

option(BUILD_TESTS "Build tests" ON)
option(BUILD_INSTALLER "Build installer" OFF)

MESSAGE(STATUS "Build tests:           " ${BUILD_TESTS})
MESSAGE(STATUS "Build installer:       " ${BUILD_INSTALLER})

#
# Helper projects
#
file(GLOB_RECURSE CMAKE_SRC cmake/*.cmake cmake/CMakeLists.txt)
add_custom_target(cmake SOURCES ${CMAKE_SRC})
GroupSources("cmake")
unset(CMAKE_SRC)

file(GLOB_RECURSE KHR_SRC include/*.hpp include/*.cpp)
add_custom_target(khr SOURCES ${KHR_SRC})
GroupSources("khr")
unset(KHR_SRC)


find_package(Threads)

set(EXTERNAL_PROJECT_PREFIX "project")
set_property(DIRECTORY PROPERTY EP_PREFIX ${EXTERNAL_PROJECT_PREFIX})
setup_externals_binary_dir()

if (BUILD_CLIENT OR BUILD_SERVER)
  add_subdirectory(plugins)
endif()


if (BUILD_TESTS)
    include(CTest)
    enable_testing()
    include(GoogleTest)
    set(GTEST_ROOT "D:/GoogleTest")
    find_package(GTest REQUIRED)
    add_subdirectory(tests)
endif()
