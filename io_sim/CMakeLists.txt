add_executable(io_simulator
    main.cpp
)

target_include_directories(io_simulator PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(io_simulator oancommon oannetutils)