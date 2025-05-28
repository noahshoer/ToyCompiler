file(GLOB_RECURSE KALEIDOSCOPE_SOURCES *.cpp)

add_library(kaleidoscope_lib ${KALEIDOSCOPE_SOURCES})
target_include_directories(kaleidoscope_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)