add_executable(oaninspect
    main.cpp
    Filter.cpp
    Decoder.cpp
)

target_compile_features(oaninspect PRIVATE cxx_std_20)

target_include_directories(oaninspect PRIVATE
    ${CMAKE_SOURCE_DIR}/OpenAudioNetwork
    ${CMAKE_SOURCE_DIR}/tools/sim_switch  # for sim_proto.h
)
