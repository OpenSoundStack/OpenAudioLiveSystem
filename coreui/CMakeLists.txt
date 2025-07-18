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
        pipes/PipeElemLPF.cpp
        pipes/PipeElemLPF.h
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
        pipes/PipeElemNoEdit.cpp
        pipes/PipeElemNoEdit.h
        pipes/ui/GainTrimUI.cpp
        pipes/ui/GainTrimUI.h
        pipes/ui/GainTrimUI.ui
        pipes/ui/FilterVizHPF.cpp
        pipes/ui/FilterVizHPF.h
        pipes/PipeElemSendMtx.cpp
        pipes/PipeElemSendMtx.h
        pipes/ui/FaderSendMtx.cpp
        pipes/ui/FaderSendMtx.h
        pipes/ui/FaderSendMtx.ui
        pipes/PipeElemAudioInMtx.cpp
        pipes/PipeElemAudioInMtx.h
        ui/Fader.cpp
        ui/Fader.h
        ui/Fader.ui
        pipes/ui/FilterVizLPF.cpp
        pipes/ui/FilterVizLPF.h
)

target_link_libraries(OALSCoreUI PRIVATE Qt6::Core Qt6::Widgets)
target_include_directories(OALSCoreUI PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(OALSCoreUI PUBLIC oannetutils oancommon)
target_link_libraries(OALSCoreUI PUBLIC OpenDSP plugin_loader)