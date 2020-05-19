# only allow out-of-source builds
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt."
  )
endif()

cmake_minimum_required(VERSION 3.15)

# define project name and current version
project(schemecpp VERSION 0.1.0)

# link the files to be included
add_executable(scheme src/main.cpp src/eval.cpp src/memory.cpp src/printer.cpp
                      src/reader.cpp src/selftest.cpp)

# tell clang to use c++17 standard
target_compile_features(scheme PRIVATE cxx_std_17)

# include header directories
target_include_directories(scheme PUBLIC src include)

# set warning levels for compilation this is different for windows machines
if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(
    -Wall
    -Wcast-align
    -Wconversion
    -Wdouble-promotion
    -Weffc++
    -Werror
    -Wextra
    -Wformat=2
    -Wold-style-cast
    -Woverload-cirtual
    -Wpedantic
    -Wshadow
    -Wsign-conversion
    -Wunused)
endif()
