file(GLOB_RECURSE KALEIDOSCOPE_SOURCES *.cpp)
list(REMOVE_ITEM KALEIDOSCOPE_SOURCES "${CMAKE_CURRENT_LIST_DIR}/parser_cli.cpp")

if(KALEIDOSCOPE_SOURCES)
    add_library(kaleidoscope_lib ${KALEIDOSCOPE_SOURCES})
    target_include_directories(kaleidoscope_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
    )

    set(LLVM_COMPONENTS
        core
        native
        orcjit)

    # if (APPLE)
    #     list(APPEND LLVM_COMPONENTS
    #         aarch64asmparser
    #         aarch64codegen)
    # else()
    #     list(APPEND LLVM_COMPONENTS
    #         x86asmparser
    #         x86codegen)
    # endif()

    llvm_map_components_to_libnames(LLVM_LIBS ${LLVM_COMPONENTS})
    target_link_libraries(kaleidoscope_lib PUBLIC ${LLVM_LIBS})
else()
    add_library(kaleidoscope_lib INTERFACE)
    target_include_directories(kaleidoscope_lib INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
    )
endif()

add_executable(parser parser_cli.cpp)
target_link_libraries(parser kaleidoscope_lib)