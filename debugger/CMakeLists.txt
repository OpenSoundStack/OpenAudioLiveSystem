find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()
qt_add_executable(debugger
    main.cpp
)

target_link_libraries(debugger PUBLIC Qt6::Core Qt6::Widgets)