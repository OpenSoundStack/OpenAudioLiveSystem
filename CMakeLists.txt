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

# Phase 1 of UID autoconfiguration: structs and algorithm compile in
# unconditionally; the algorithm only runs when this flag is set.
# Default OFF — phase 2 will flip it on for Linux + host-backend builds.
option(OAN_UID_AUTOCONF "Enable UID autoconfiguration" OFF)
if(OAN_UID_AUTOCONF)
    add_compile_definitions(OAN_UID_AUTOCONF)
endif()

if(APPLE)
    # macOS root volume is read-only (SIP), so the Linux target's /core_plugins
    # path can't be created. Use a per-user dir under $HOME instead — keeps
    # the dev loop sudo-free.
    set(ENGINE_PLUGIN_SYSLOCATION "$ENV{HOME}/.osst/core_plugins")
else()
    set(ENGINE_PLUGIN_SYSLOCATION "/core_plugins")
endif()
add_compile_definitions(ENGINE_PLUGIN_SYSLOCATION="${ENGINE_PLUGIN_SYSLOCATION}")

enable_testing()

add_subdirectory(OpenDSP)
add_subdirectory(OpenAudioNetwork)

add_subdirectory(plugins)
add_subdirectory(coreui)
add_subdirectory(engine)
add_subdirectory(io_sim)
add_subdirectory(debugger)
add_subdirectory(tools/sim_switch)
add_subdirectory(tools/oaninspect)