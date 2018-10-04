# If we're running under the gradle build, HIFI_ANDROID will be set here, but
# ANDROID will not be set until after the `project` statement.  This is the *ONLY*
# place you need to use `HIFI_ANDROID` instead of `ANDROID`
cmake_minimum_required(VERSION 3.11)

project(hkrpp)

include("cmake/init.cmake")

include("cmake/compiler.cmake")

option(BUILD_TESTS "Build tests" ON)
option(BUILD_TOOLS "Build tools" ON)
#option(BUILD_INSTALLER "Build installer" OFF)

MESSAGE(STATUS "Build tests:           " ${BUILD_TESTS})
MESSAGE(STATUS "Build tools:           " ${BUILD_TOOLS})
#MESSAGE(STATUS "Build installer:       " ${BUILD_INSTALLER})

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

if (BUILD_TOOLS)
  add_subdirectory(tools)
endif()

if (BUILD_TESTS)
    include(CTest)
    enable_testing()
    include(GoogleTest)
    set(GTEST_ROOT "D:/GoogleTest")
    find_package(GTest REQUIRED)
    add_subdirectory(tests)
endif()
