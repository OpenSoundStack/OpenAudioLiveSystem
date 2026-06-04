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

# UID autoconfiguration: on by default for Linux production + host-backend
# dev. Off elsewhere (e.g. firmware-style toolchain where the build picks up
# this top-level CMake — unusual, but the firmwares vendor OAN directly and
# do their own flag wiring in phase 3).
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR OAN_HOST_BACKENDS)
    set(_OAN_UID_AUTOCONF_DEFAULT ON)
else()
    set(_OAN_UID_AUTOCONF_DEFAULT OFF)
endif()
option(OAN_UID_AUTOCONF "Enable UID autoconfiguration" ${_OAN_UID_AUTOCONF_DEFAULT})
if(OAN_UID_AUTOCONF)
    add_compile_definitions(OAN_UID_AUTOCONF)
endif()
message(STATUS "OAN_UID_AUTOCONF = ${OAN_UID_AUTOCONF}")

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