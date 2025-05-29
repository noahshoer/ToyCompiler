file(GLOB_RECURSE KALEIDOSCOPE_SOURCES *.cpp)
list(REMOVE_ITEM KALEIDOSCOPE_SOURCES "${CMAKE_CURRENT_LIST_DIR}/parser_cli.cpp")

if(KALEIDOSCOPE_SOURCES)
    add_library(kaleidoscope_lib ${KALEIDOSCOPE_SOURCES})
    target_include_directories(kaleidoscope_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
    )
else()
    add_library(kaleidoscope_lib INTERFACE)
    target_include_directories(kaleidoscope_lib INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
    )
endif()

add_executable(parser parser_cli.cpp)
target_link_libraries(parser kaleidoscope_lib)