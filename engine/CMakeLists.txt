add_executable(OALSEngine
    main.cpp
        NetMan.cpp
        NetMan.h
        AudioEngine.cpp
        AudioEngine.h
        log.h
        piping/AudioPlumber.cpp
        piping/AudioPlumber.h
        piping/filtering/FiltHPFPipe.cpp
        piping/filtering/FiltHPFPipe.h
        piping/filtering/FiltLPFPipe.cpp
        piping/filtering/FiltLPFPipe.h
        piping/feedback/LevelMeasurePipe.cpp
        piping/feedback/LevelMeasurePipe.h
        routing_routines.h
        routing_routines.cpp
)

target_include_directories(OALSEngine PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(OALSEngine OpenDSP)
target_link_libraries(OALSEngine oannetutils oancommon)