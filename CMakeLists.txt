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
add_executable(scheme 
  src/main.cpp 
  src/scheme.cpp 
  src/memory.cpp 
  src/parse.cpp 
  src/evaluate.cpp 
  src/operations.cpp
  src/trampoline.cpp
  src/environment.cpp
  src/test.cpp
  src/repl.cpp
  src/setup.cpp
  include/loguru.cpp
  )

# tell clang to use c++17 standard
target_compile_features(scheme PRIVATE cxx_std_17)

# include header directories
target_include_directories(scheme PUBLIC src include)

# copy other files to build directory
configure_file("${CMAKE_SOURCE_DIR}/src/std.scm" "${CMAKE_BINARY_DIR}/std.scm" COPYONLY)

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
    -Wunused
    )
endif()

target_link_libraries(scheme pthread)