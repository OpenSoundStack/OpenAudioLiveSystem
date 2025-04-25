find_package(Qt6 REQUIRED Core Widgets)
qt_standard_project_setup()

set(AUTO_UIC ON)
set(AUTO_MOC ON)
set(AUTO_RCC ON)

qt_add_executable(OALSCoreUI
        main.cpp
        ui/SignalWindow.cpp
        ui/SignalWindow.h
        ui/SignalWindow.ui
        ui/SetupWindow.cpp
        ui/SetupWindow.h
        ui/SetupWindow.ui
        ui/PipeVisualizer.cpp
        ui/PipeVisualizer.h
        ui/PipeVisualizer.ui
        core/PipeDesc.h
        core/PipeElemLPF.cpp
        core/PipeElemLPF.h
        core/PipeDesc.cpp
)

target_link_libraries(OALSCoreUI PRIVATE Qt6::Core Qt6::Widgets)