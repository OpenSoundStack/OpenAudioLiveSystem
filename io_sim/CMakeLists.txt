add_executable(io_simulator
    main.cpp
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SNDFILE REQUIRED sndfile)

include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")
FetchContent_MakeAvailable(nlohmann_json)

target_include_directories(io_simulator PUBLIC ${PROJECT_SOURCE_DIR} ${SNDFILE_INCLUDE_DIRS})
target_link_directories(io_simulator PUBLIC ${SNDFILE_LIBRARY_DIRS})
target_link_libraries(io_simulator oancommon oannetutils nlohmann_json::nlohmann_json)
target_link_libraries(io_simulator ${SNDFILE_LIBRARIES})