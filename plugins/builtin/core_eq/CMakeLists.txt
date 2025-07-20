set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_library(CoreEqPlugin SHARED
        core_eq.cpp
        core_eq.h
        CoreEqElem.cpp
        CoreEqElem.h
        CoreEqPipe.cpp
        CoreEqPipe.h
        CoreEqControlUI.cpp
        CoreEqControlUI.h
        common.h
        FilterControl.cpp
        FilterControl.h
        FilterControl.ui
        CoreEQ_UI.cpp
        CoreEQ_UI.h
)

target_include_directories(CoreEqPlugin PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(CoreEqPlugin OpenDSP)
target_link_libraries(CoreEqPlugin oancommon oannetutils)
target_link_libraries(CoreEqPlugin plugin_loader)

install(
        TARGETS CoreEqPlugin
        DESTINATION ${ENGINE_PLUGIN_SYSLOCATION}/
)