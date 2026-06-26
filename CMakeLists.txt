cmake_minimum_required(VERSION 3.20)
project(PlagiarismDetector)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


if(WIN32)
    set(LLVM_DIR "C:/msys64/ucrt64/lib/cmake/llvm")
    set(Clang_DIR "C:/msys64/ucrt64/lib/cmake/clang")
else()
    set(LLVM_DIR "/usr/lib/llvm-18/lib/cmake/llvm")
    set(Clang_DIR "/usr/lib/llvm-18/lib/cmake/clang")
endif()

find_package(LLVM  REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

set(CLANG_LIBS
    clangTooling
    clangFrontend
    clangSerialization
    clangDriver
    clangParse
    clangSema
    clangAnalysis
    clangEdit
    clangAST
    clangASTMatchers
    clangAPINotes
    clangLex
    clangBasic
    clangSupport
    libclang             
)


set(IO_SOURCES
    src/io/FileScanner.cpp
    src/io/FileReader.cpp
    src/io/Logger.cpp
)

set(CLI_SOURCES
    src/cli/CLI.cpp
)

set(LEXER_SOURCES
    src/lexer/Lexer.cpp
    src/lexer/Normalizer.cpp
)

set(SIMILARITY_SOURCES
    src/similarity/SimilarityEngine.cpp
)

set(AST_SOURCES
    src/ast/ASTNormalizer.cpp
    src/ast/APTEDDistance.cpp
)

set(REPORT_SOURCES
    src/report/ReportGenerator.cpp
)

set(CORE_SOURCES
    src/core/ThreadPool.cpp
    src/core/Pipeline.cpp
)

set(ALL_LIB_SOURCES
    ${IO_SOURCES}
    ${CLI_SOURCES}
    ${LEXER_SOURCES}
    ${SIMILARITY_SOURCES}
    ${AST_SOURCES}
    ${REPORT_SOURCES}
    ${CORE_SOURCES}
)


add_library(detector_lib STATIC ${ALL_LIB_SOURCES})

target_include_directories(detector_lib
    PUBLIC
        include
        ${LLVM_INCLUDE_DIRS}
        ${CLANG_INCLUDE_DIRS}
)

if(WIN32)
    target_link_libraries(detector_lib
        PUBLIC
            ${CLANG_LIBS}
            LLVM
    )
else()
    target_link_libraries(detector_lib
        PUBLIC
            ${CLANG_LIBS}
            LLVM-18
    )
endif()

if(NOT WIN32)
    target_link_options(detector_lib
        PUBLIC
            -Wl,-rpath,/usr/lib/llvm-18/lib
    )
endif()


add_executable(detector src/main.cpp)

target_include_directories(detector PRIVATE include)

target_link_libraries(detector
    PRIVATE
        detector_lib
)


enable_testing()

add_executable(tests tests/test.cpp)

target_include_directories(tests PRIVATE include)

target_link_libraries(tests
    PRIVATE
        detector_lib
)

add_test(NAME unit_tests COMMAND tests)