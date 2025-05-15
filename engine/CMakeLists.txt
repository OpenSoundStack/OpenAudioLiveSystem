add_executable(OALSEngine
    main.cpp
        NetMan.cpp
        NetMan.h
        AudioEngine.cpp
        AudioEngine.h
        log.h
        piping/AudioPlumber.cpp
        piping/AudioPlumber.h
)

target_include_directories(OALSEngine PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(OALSEngine OpenDSP)
target_link_libraries(OALSEngine oannetutils oancommon)