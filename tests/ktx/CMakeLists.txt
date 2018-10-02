set(TARGET_NAME "ktx-test")

# Pass test files to code as preprocessor macros
file(GLOB KTX_TEST_FILES "${CMAKE_SOURCE_DIR}/external/ktx/tests/testimages/*.ktx")
list(JOIN KTX_TEST_FILES ":" KTX_TEST_FILES)

file(GLOB KTX2_TEST_FILES "${CMAKE_SOURCE_DIR}/external/ktx/tests/testimages/*.ktx2")
list(JOIN KTX2_TEST_FILES ":" KTX2_TEST_FILES)

khrpp_add_test()
target_compile_definitions(${TARGET_NAME} PRIVATE -DKTX_TEST_FILES="${KTX_TEST_FILES}")
target_compile_definitions(${TARGET_NAME} PRIVATE -DKTX2_TEST_FILES="${KTX2_TEST_FILES}")
target_fmt()

