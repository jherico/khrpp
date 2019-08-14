set(TARGET_NAME "kspex")

function(dump_cmake_variables)
	get_cmake_property(_variableNames VARIABLES)
	list (SORT _variableNames)
	foreach (_variableName ${_variableNames})
		if (ARGV0)
			unset(MATCHED)
			string(REGEX MATCH ${ARGV0} MATCHED ${_variableName})
			if (NOT MATCHED)
				continue()
			endif()
		endif()
		message(STATUS "${_variableName}=${${_variableName}}")
	endforeach()
endfunction()

macro(add_qt_executable)
	if (NOT Qt5_BIN_DIR) 
		get_property(QMAKE_EXECUTABLE TARGET Qt5::qmake PROPERTY LOCATION)
		get_filename_component(Qt5_BIN_DIR ${QMAKE_EXECUTABLE} DIRECTORY CACHE)
	endif()

	if(WIN32)
		find_program(WINDEPLOYQT_COMMAND windeployqt PATHS ${Qt5_BIN_DIR} NO_DEFAULT_PATH)
		if (NOT WINDEPLOYQT_COMMAND)
			message(FATAL_ERROR "Could not find windeployqt at ${Qt5_BIN_DIR}. windeployqt is required.")
        endif()
    elseif(APPLE)
        find_program(MACDEPLOYQT_COMMAND macdeployqt PATHS ${Qt5_BIN_DIR} NO_DEFAULT_PATH)
        if (NOT MACDEPLOYQT_COMMAND)
            message(FATAL_ERROR "Could not find macdeployqt at ${Qt5_BIN_DIR}. macdeployqt is required.")
        endif()
	endif()

	if (WIN32)
        add_executable(${TARGET_NAME} WIN32 ${TARGET_SRCS})
        set(DEPLOY_COMMANDS ${WINDEPLOYQT_COMMAND} )
        list(APPEND DEPLOY_COMMANDS --no-compiler-runtime --no-opengl-sw --no-angle -no-system-d3d-compiler)
        if (TARGET_QML_DIR)
            list(APPEND DEPLOY_COMMANDS --qmldir "${TARGET_QML_DIR}")
        endif()
        add_custom_command(
            TARGET ${TARGET_NAME}
            POST_BUILD
            COMMAND 
                ${DEPLOY_COMMANDS}
                ${EXTRA_DEPLOY_OPTIONS} 
                $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>,$<CONFIG:RelWithDebInfo>>:--release>
                "$<TARGET_FILE:${TARGET_NAME}>"
        )
	else(APPLE)
		add_executable(${TARGET_NAME} MACOSX_BUNDLE ${INTERFACE_SRCS} ${QM})
		set_target_properties(${TARGET_NAME} PROPERTIES INSTALL_RPATH "@executable_path/../Frameworks")
        add_custom_command(
            TARGET ${TARGET_NAME}
            POST_BUILD
            COMMAND 
                ${MACDEPLOYQT_COMMAND} 
                $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>,$<CONFIG:RelWithDebInfo>>:--release>
                --no-opengl-sw --no-angle
                "$<TARGET_FILE:${TARGET_NAME}>"
        )
	endif()
endmacro()

find_package(Qt5 COMPONENTS Gui Qml Quick QuickControls2)

if (Qt5_FOUND) 
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    project(${TARGET_NAME})

    file(GLOB TARGET_SRCS src/*)
    list(APPEND TARGET_SRCS ${TARGET_NAME}.qrc)
    file(GLOB_RECURSE QML_SRCS qml/*)
    list(APPEND TARGET_SRCS ${QML_SRCS})
    foreach(_FILE ${QML_SRCS})
        if(IS_DIRECTORY ${_FILE})
            continue()
        endif()
        source_group(QML FILES ${_FILE})
    endforeach()
    set(TARGET_QML_DIR "${CMAKE_CURRENT_SOURCE_DIR}/qml")
    file(TO_NATIVE_PATH "${TARGET_QML_DIR}" TARGET_QML_DIR)
	add_qt_executable()
    set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "tools")
    target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
    target_link_libraries(${TARGET_NAME} PRIVATE Qt5::Gui Qt5::Qml Qt5::Quick Qt5::QuickControls2)
    target_glm()
    target_basis()

    set(CMAKE_AUTOMOC OFF)
    set(CMAKE_AUTORCC OFF)
endif()





