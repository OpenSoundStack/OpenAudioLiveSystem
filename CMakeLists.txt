cmake_minimum_required(VERSION 3.22)
project(OALiveSystem)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(OpenDSP)
add_subdirectory(OpenAudioNetwork)

add_subdirectory(plugins)
add_subdirectory(coreui)
add_subdirectory(engine)
add_subdirectory(io_sim)