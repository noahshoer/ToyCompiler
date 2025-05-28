cmake_minimum_required(VERSION 3.16)
project(ToyCompiler LANGUAGES CXX)

include(FetchContent)

cmake_policy(SET CMP0135 NEW)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/c00fd25b71a17e645e4567fcb465c3fa532827d2.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Silence the deprecated header warning in gtest/gmock
foreach(target gtest gtest_main gmock gmock_main)
  if(TARGET ${target})
    target_compile_options(${target} PRIVATE -Wno-deprecated)
  endif()
endforeach()

include(GoogleTest)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${gtest_SOURCE_DIR}/include
    ${gmock_SOURCE_DIR}/include)

enable_testing()
include(GoogleTest)

add_compile_options(-Wno-deprecated)

add_subdirectory(Kaleidoscope)
# add_subdirectory(Toy)