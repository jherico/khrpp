
macro(KHRPP_ADD_TEST)
    project(${TARGET_NAME})
    file(GLOB TARGET_SRCS src/*)
    add_executable(${TARGET_NAME} ${TARGET_SRCS})
    set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "tests")

    target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(${TARGET_NAME} PRIVATE GTest::GTest GTest::Main)
    gtest_add_tests(TARGET ${TARGET_NAME} AUTO)
endmacro()

