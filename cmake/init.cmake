set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakeTargets")
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (CMAKE_BUILD_TYPE)
  string(TOUPPER ${CMAKE_BUILD_TYPE} UPPER_CMAKE_BUILD_TYPE)
else ()
  set(UPPER_CMAKE_BUILD_TYPE DEBUG)
endif ()

# CMAKE_CURRENT_SOURCE_DIR is the parent folder here
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

set(HF_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(MACRO_DIR "${HF_CMAKE_DIR}/macros")
set(EXTERNAL_PROJECT_DIR "${HF_CMAKE_DIR}/externals")

file(GLOB CUSTOM_MACROS "cmake/macros/*.cmake")
foreach(CUSTOM_MACRO ${CUSTOM_MACROS})
  include(${CUSTOM_MACRO})
endforeach()
unset(CUSTOM_MACROS) 
