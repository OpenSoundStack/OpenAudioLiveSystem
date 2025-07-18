find_package(Qt6 REQUIRED Core Widgets)
qt_standard_project_setup()

set(AUTO_UIC ON)
set(AUTO_MOC ON)
set(AUTO_RCC ON)

qt_add_library(plugin_loader SHARED
    PluginLoader.cpp
    PluginLoader.h
    AudioPipe.cpp
    AudioPipe.h
    PluginInterface.h
    PipeDesc.cpp
    PipeDesc.h
    ElemControlData.cpp
    ElemControlData.h
    PluginInterface.cpp
    ui/FilterEditBase.cpp
    ui/FilterEditBase.h
    ui/FilterEditBase.ui
    ui/VizUtils.cpp
    ui/VizUtils.h
)

target_include_directories(plugin_loader PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(plugin_loader Qt6::Core Qt6::Widgets)
target_link_libraries(plugin_loader oancommon oannetutils)