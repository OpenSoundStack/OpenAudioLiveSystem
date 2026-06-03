cmake_minimum_required(VERSION 3.22)
project(OALiveSystem)

set(CMAKE_CXX_STANDARD 20)

if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "aarch64" AND NOT APPLE)
    message("Added arm gcc optim")
    add_compile_options(-ftree-vectorize)
endif()

option(OAN_HOST_BACKENDS "Build host dev transports + RT shim" OFF)
if(APPLE)
    set(OAN_HOST_BACKENDS ON CACHE BOOL "Build host dev transports + RT shim" FORCE)
endif()
if(OAN_HOST_BACKENDS)
    add_compile_definitions(OAN_HOST_BACKENDS)
endif()

set(ENGINE_PLUGIN_SYSLOCATION "/core_plugins")
add_compile_definitions(ENGINE_PLUGIN_SYSLOCATION="${ENGINE_PLUGIN_SYSLOCATION}")

add_subdirectory(OpenDSP)
add_subdirectory(OpenAudioNetwork)

add_subdirectory(plugins)
add_subdirectory(coreui)
add_subdirectory(engine)
add_subdirectory(io_sim)
add_subdirectory(debugger)