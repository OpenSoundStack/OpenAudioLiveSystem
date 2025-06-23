add_executable(io_simulator
    main.cpp
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(ALSA REQUIRED alsa)
pkg_check_modules(SNDFILE REQUIRED sndfile)

target_include_directories(io_simulator PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(io_simulator oancommon oannetutils)
target_link_libraries(io_simulator ${ALSA_LIBRARIES})
target_link_libraries(io_simulator ${SNDFILE_LIBRARIES})