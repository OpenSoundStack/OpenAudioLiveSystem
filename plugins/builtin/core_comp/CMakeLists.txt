set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_library(CoreCompPlugin SHARED
        core_comp.cpp
        core_comp.h
        CoreCompPipe.cpp
        CoreCompPipe.h
        CoreCompElem.cpp
        CoreCompElem.h
        CoreComp_UI.cpp
        CoreComp_UI.h
        CompViz.cpp
        CompViz.h
        CompControl.cpp
        CompControl.h
        CompControl.ui
)

target_include_directories(CoreCompPlugin PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(CoreCompPlugin OpenDSP)
target_link_libraries(CoreCompPlugin oancommon oannetutils)
target_link_libraries(CoreCompPlugin plugin_loader)

install(
        TARGETS CoreCompPlugin
        DESTINATION ${ENGINE_PLUGIN_SYSLOCATION}/
)
