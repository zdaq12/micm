################################################################################
# Preamble
cmake_minimum_required(VERSION 3.21)

project(
  micm
  VERSION 3.3.1
  LANGUAGES CXX
)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

message(STATUS "CMake build configuration for micm(${CMAKE_BUILD_TYPE}) ${PROJECT_VERSION}")

################################################################################
# Projet wide setup options

include(cmake/StaticAnalyzers.cmake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/cmake")

# Set up include and lib directories
set(MICM_LIB_DIR "${PROJECT_BINARY_DIR}/lib")

option(ENABLE_CLANG_TIDY "Enable clang-tiday to format source code" OFF)
option(ENABLE_MPI "Enable MPI parallel support" OFF)
option(ENABLE_OPENMP "Enable OpenMP support" OFF)
option(ENABLE_COVERAGE "Enable code coverage output" OFF)
option(ENABLE_MEMCHECK "Enable memory checking in tests" OFF)
option(ENABLE_JSON "Enable json configureation file reading" ON)
option(BUILD_DOCS "Build the documentation" OFF)
option(ENABLE_CUDA "Build with Cuda support" OFF)
option(ENABLE_OPENACC "Build with OpenACC Support" OFF)
option(ENABLE_LLVM "Build with LLVM support for JIT-compiling" OFF)
option(ENABLE_TESTS "Build the tests" ON)
option(ENABLE_EXAMPLES "Build the examples" ON)
set(DEFAULT_VECTOR_MATRIX_SIZE "4" CACHE STRING "Default size for vectorizable matrix types")

include(CMakeDependentOption)
# Option to collect custom OpenACC flags
cmake_dependent_option(ENABLE_GPU_TYPE "Enable custom CUDA flags" OFF
  "ENABLE_OPENACC OR ENABLE_CUDA" ON)

if(ENABLE_GPU_TYPE)
  set(GPU_TYPE "" CACHE STRING "The GPU type being targeted")
endif()

add_compile_definitions(DEFAULT_VECTOR_SIZE=${DEFAULT_VECTOR_MATRIX_SIZE})

# on ubuntu with clang, an incorrect version of the c++ standard library was being linked
if (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Linux" AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # If the compiler is Clang, use libc++
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

################################################################################
# Dependencies

include(cmake/dependencies.cmake)

################################################################################
# micm targets and documentation

add_subdirectory(src)

if(BUILD_DOCS)
  add_subdirectory(docs)
endif()

################################################################################
# Tests

if(PROJECT_IS_TOP_LEVEL AND ENABLE_TESTS)
  # Test code coverage
  if(ENABLE_COVERAGE)
    include(CodeCoverage)
    append_coverage_compiler_flags()
    setup_target_for_coverage_lcov(
        NAME coverage
        EXECUTABLE "ctest"
        EXCLUDE "${PROJECT_SOURCE_DIR}/test/*"
        BASE_DIRECTORY "${PROJECT_SOURCE_DIR}/src")

  endif()

  enable_testing()
  add_subdirectory(test)
endif()

################################################################################
# Packaging

# only include packaging if we are the top level project being built
if(PROJECT_IS_TOP_LEVEL)
  add_subdirectory(packaging)
endif()

################################################################################

# on ubuntu with clang, an incorrect version of the c++ standard library was being linked
if (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Linux" AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # If the compiler is Clang, use libc++
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()

################################################################################
# Command-line driver examples

if(PROJECT_IS_TOP_LEVEL AND ENABLE_EXAMPLES AND ENABLE_JSON)
  add_subdirectory(examples)
endif()

################################################################################
# Copy configuration data

if(PROJECT_IS_TOP_LEVEL AND (ENABLE_TESTS OR ENABLE_EXAMPLES))
  add_custom_target(copy_example_configs ALL ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/examples/configs ${CMAKE_BINARY_DIR}/examples/configs)
endif()
