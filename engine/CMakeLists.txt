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
        piping/io/AudioInPipe.cpp
        piping/io/AudioInPipe.h
        piping/AudioPipe.cpp
        piping/AudioPipe.h
        routing_routines.h
        routing_routines.cpp
        piping/io/AudioSendMtx.cpp
        piping/io/AudioSendMtx.h
        piping/io/AudioInMtx.cpp
        piping/io/AudioInMtx.h
        SampleStream.cpp
        SampleStream.h
        piping/io/AudioDirectOut.cpp
        piping/io/AudioDirectOut.h
)

target_include_directories(OALSEngine PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(OALSEngine OpenDSP)
target_link_libraries(OALSEngine oannetutils oancommon)