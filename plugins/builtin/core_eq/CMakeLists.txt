add_library(CoreEqPlugin SHARED
        core_eq.cpp
        core_eq.h
)

target_include_directories(CoreEqPlugin PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(CoreEqPlugin OpenDSP)
target_link_libraries(CoreEqPlugin oancommon oannetutils)
target_link_libraries(CoreEqPlugin plugin_loader)

install(
        TARGETS CoreEqPlugin
        DESTINATION ${ENGINE_PLUGIN_SYSLOCATION}/
)