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
        pipes/PipeElemLPF.cpp
        pipes/PipeElemLPF.h
        core/PipeDesc.cpp
        ui/VizUtils.h
        ui/VizUtils.cpp
        pipes/PipeElemHPF.cpp
        pipes/PipeElemHPF.h
        core/ShowManager.cpp
        core/ShowManager.h
        pipes/PipeElemAudioIn.cpp
        pipes/PipeElemAudioIn.h
        core/AudioRouterQt.cpp
        core/AudioRouterQt.h
        core/DSPManager.cpp
        core/DSPManager.h
        core/NetworkConfig.h
)

target_link_libraries(OALSCoreUI PRIVATE Qt6::Core Qt6::Widgets)
target_include_directories(OALSCoreUI PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(OALSCoreUI PUBLIC oannetutils oancommon)