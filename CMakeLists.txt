cmake_minimum_required(VERSION 3.20)

project(PlagiarismDetector)

set(CMAKE_CXX_STANDARD 20)

add_executable(detector

    src/main.cpp
    src/io/FileScanner.cpp
    src/io/FileReader.cpp
    src/utils/Logger.cpp
)

target_include_directories(detector
    PRIVATE
        include
)

find_package(LLVM REQUIRED CONFIG)

find_package(Clang REQUIRED CONFIG)

target_link_libraries(detector

clangTooling

clangFrontend

clangSerialization

clangDriver

clangParse

clangSema

clangAnalysis

clangEdit

clangAST

clangLex

clangBasic

)