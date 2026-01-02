find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SNDFILE REQUIRED sndfile)

qt_standard_project_setup()
qt_add_executable(debugger
    main.cpp
        DebuggerWindow.cpp
        DebuggerWindow.h
        DebuggerWindow.ui
)

target_include_directories(debugger PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(debugger PUBLIC Qt6::Core Qt6::Widgets)
target_link_libraries(debugger PUBLIC oancommon oannetutils)
target_link_libraries(debugger PUBLIC ${SNDFILE_LIBRARIES})