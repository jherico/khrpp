set(TARGET_NAME "ktx-test")
khrpp_add_test()

# Needs EGL includes
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/external/egl/api)
