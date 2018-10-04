set(TARGET_NAME "kspex")

project(${TARGET_NAME})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

file(GLOB TARGET_SRCS src/*)
add_executable(${TARGET_NAME} ${TARGET_SRCS})
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "tools")
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_vulkan()
target_vma()
target_qt()
target_glm()

set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTORCC OFF)
