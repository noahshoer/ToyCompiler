# Unit tests

file(GLOB_RECURSE UNITTEST_SOURCES "*.cpp")

foreach(test_src ${UNITTEST_SOURCES})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    target_link_libraries(${test_name} PRIVATE kaleidoscope_lib GTest::gtest_main GTest::gmock GTest::gmock_main)

    message(STATUS "Adding test: ${test_name}")
    gtest_discover_tests(${test_name})
endforeach()
