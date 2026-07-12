cmake_minimum_required(VERSION 3.22)
project(OALiveSystem)

set(CMAKE_CXX_STANDARD 20)

if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64")
    message("Added arm gcc optim")
    add_compile_options(-ftree-vectorize)
endif()

set(ENGINE_PLUGIN_SYSLOCATION "/core_plugins")
add_compile_definitions(ENGINE_PLUGIN_SYSLOCATION="${ENGINE_PLUGIN_SYSLOCATION}")

option(BUILD_DEBUG_TOOLS "Enable building testing and debug tools" ON)

add_subdirectory(OpenDSP)
add_subdirectory(OpenAudioNetwork)

add_subdirectory(plugins)
add_subdirectory(coreui)
add_subdirectory(engine)

if(BUILD_DEBUG_TOOLS)
    message("Building debug tools")

    add_subdirectory(io_sim)
    add_subdirectory(debugger)
endif ()