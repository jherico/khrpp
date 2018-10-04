macro(TARGET_QT)
    find_package(Qt5 
        COMPONENTS Core Gui 
        PATHS "C:/Qt/5.11.1/msvc2017_64/lib/cmake/" "C:/Qt/5.11.1/msvc2017_64"
        REQUIRED)
    target_link_libraries(${TARGET_NAME} PRIVATE Qt5::Core Qt5::Gui)
    target_include_directories(${TARGET_NAME} PRIVATE ${Qt5Core_INCLUDE_DIRS} ${Qt5Gui_INCLUDE_DIRS})
endmacro()