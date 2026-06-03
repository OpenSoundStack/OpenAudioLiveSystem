add_executable(sim_switch
    main.cpp
    Switch.cpp
    DiscoveryPeek.cpp
    Tui.cpp
)

target_compile_features(sim_switch PRIVATE cxx_std_20)

target_include_directories(sim_switch PRIVATE
    ${CMAKE_SOURCE_DIR}/OpenAudioNetwork
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Smoke test (only when OAN_HOST_BACKENDS so SimTransport is built into oannetutils)
if(OAN_HOST_BACKENDS)
    find_package(GTest QUIET)
    if(GTest_FOUND)
        enable_testing()
        add_executable(sim_switch_test test/test_sim_switch.cpp)
        target_compile_features(sim_switch_test PRIVATE cxx_std_20)
        target_include_directories(sim_switch_test PRIVATE
            ${CMAKE_SOURCE_DIR}/OpenAudioNetwork
            ${CMAKE_CURRENT_SOURCE_DIR}
        )
        target_link_libraries(sim_switch_test PRIVATE
            GTest::gtest GTest::gtest_main
            oannetutils
        )
        # Tests need the daemon binary in PATH; expose its build location.
        add_test(NAME sim_switch_smoke COMMAND sim_switch_test)
        set_tests_properties(sim_switch_smoke PROPERTIES
            ENVIRONMENT "SIM_SWITCH_BIN=$<TARGET_FILE:sim_switch>"
        )
    else()
        message(STATUS "sim_switch: GTest not found, skipping sim_switch_test")
    endif()
endif()
