cmake_minimum_required(VERSION 3.12)

option(BUILD_TESTS "Build tests" ON)
option(BUILD_TOOLS "Build tools" ON)
option(BUILD_INSTALLER "Build installer" OFF)

MESSAGE(STATUS "Build tests:           " ${BUILD_TESTS})
MESSAGE(STATUS "Build tools:           " ${BUILD_TOOLS})
MESSAGE(STATUS "Build installer:       " ${BUILD_INSTALLER})

if (BUILD_TESTS OR BUILD_TOOLS)
    set(VCPKG_PACKAGES egl-registry fmt opengl-registry glm gtest basisu)
    if (BUILD_TOOLS)
        #list(APPEND VCPKG_PACKAGES qt5-base)
    endif()
    include(${CMAKE_SOURCE_DIR}/cmake/ezvcpkg/ezvcpkg.cmake)

    ezvcpkg_fetch(
        PACKAGES ${VCPKG_PACKAGES}
        OUTPUT EZVCPKG_DIR
        UPDATE_TOOLCHAIN
        USE_HOST_VCPKG
    )
endif()

project(hkrpp)

include("cmake/init.cmake")
include("cmake/compiler.cmake")

#
# Helper projects
#
file(GLOB_RECURSE CMAKE_SRC cmake/*.cmake cmake/CMakeLists.txt)
add_custom_target(cmake SOURCES ${CMAKE_SRC})
GroupSources(${CMAKE_SOURCE_DIR} ${SOURCE}})
unset(CMAKE_SRC)

file(GLOB_RECURSE KHR_SRC include/*.hpp include/*.cpp)
add_custom_target(khrpp_source SOURCES ${KHR_SRC})
GroupSources("khr")
unset(KHR_SRC)
add_library(khrpp INTERFACE)
target_include_directories(khrpp INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

find_package(Threads)

if (BUILD_TOOLS)
  add_subdirectory(tools)
endif()

if (BUILD_TESTS)
    include(CTest)
    enable_testing()
    find_package(GTest MODULE REQUIRED)                                      
    find_package(GTest REQUIRED)
    add_subdirectory(tests)
endif()
                                                                         