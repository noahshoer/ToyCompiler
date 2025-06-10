file(GLOB_RECURSE KALEIDOSCOPE_SOURCES *.cpp)

set(PROGRAMS
    look
    reflect)

foreach(exe_src ${PROGRAMS})
    list(REMOVE_ITEM KALEIDOSCOPE_SOURCES "${CMAKE_CURRENT_LIST_DIR}/${exe_src}.cpp")
endforeach()


if(KALEIDOSCOPE_SOURCES)
    add_library(kaleidoscope_lib ${KALEIDOSCOPE_SOURCES})
    target_include_directories(kaleidoscope_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
    )

    set(LLVM_COMPONENTS
        armasmparser
        armcodegen
        armdesc
        armdisassembler
        arminfo
        armutils
        analysis
        asmparser
        asmprinter
        codegen
        core
        debuginfodwarf
        executionengine
        jitlink
        instcombine
        irreader
        mc
        mcjit
        mcparser
        native
        nativecodegen
        object
        orcjit
        orcshared
        orctargetprocess
        passes
        runtimedyld
        scalaropts
        support
        target
        targetparser
        transformutils)

    if (APPLE)
        list(APPEND LLVM_COMPONENTS
            aarch64asmparser
            aarch64codegen
            aarch64desc
            aarch64disassembler
            aarch64info
            aarch64utils)
    else()
        list(APPEND LLVM_COMPONENTS
            x86asmparser
            x86codegen)
    endif()

    llvm_map_components_to_libnames(LLVM_LIBS ${LLVM_COMPONENTS})
    target_link_libraries(kaleidoscope_lib PUBLIC ${LLVM_LIBS})
else()
    add_library(kaleidoscope_lib INTERFACE)
    target_include_directories(kaleidoscope_lib INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
    )
endif()

foreach(exe_src ${PROGRAMS})
    add_executable(${exe_src} ${exe_src}.cpp)
    target_link_libraries(${exe_src} kaleidoscope_lib)
endforeach()