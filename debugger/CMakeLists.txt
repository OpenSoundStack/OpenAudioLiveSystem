find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()
qt_add_executable(debugger
    main.cpp
        DebuggerWindow.cpp
        DebuggerWindow.h
        DebuggerWindow.ui
)

target_link_libraries(debugger PUBLIC Qt6::Core Qt6::Widgets)