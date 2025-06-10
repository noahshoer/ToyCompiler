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

# === LLVM Integration for macOS ===
if(APPLE)
  # Homebrew default path (adjust for Intel if needed)
  set(LLVM_DIR "/opt/homebrew/opt/llvm/lib/cmake/llvm" CACHE PATH "Path to LLVMConfig.cmake")
  find_package(LLVM REQUIRED CONFIG)

  message(STATUS "Found LLVM version ${LLVM_PACKAGE_VERSION} on macOS")
  message(STATUS "LLVM include dirs: ${LLVM_INCLUDE_DIRS}")
  message(STATUS "LLVM libraries: ${LLVM_LIBRARY_DIRS}")

  include_directories(${LLVM_INCLUDE_DIRS})
  add_definitions(${LLVM_DEFINITIONS})
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
add_subdirectory(Kaleidoscope)

# add_subdirectory(Toy)