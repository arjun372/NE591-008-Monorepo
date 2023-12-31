# vim: set foldmarker={{{,}}} foldlevel=0 foldmethod=marker:
cmake_minimum_required(VERSION 3.14)
find_package(MPI REQUIRED)

project(ne591_008)
include(helpers.cmake) # Include helper methods
set(CMAKE_DISABLE_SOURCE_CHANGES ON) # In-source build prevention.
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

######################### Options ################################################## {{{
option(BUILD_TESTS "Build the tests" ON)  # Influences CTest.
option(CODE_COVERAGE "Perform code coverage analysis" OFF)
option(PORTABLE "Build without CPU specific optimizations" OFF)
option(ENABLE_PGO "Enable Profile-Guided Optimization (PGO)" OFF)
option(ENABLE_LTO "Enable Link-Time Optimization (LTO)" ON)
option(ENABLE_CODE_QUALITY_CHECKS "Enable extra compile-time warnings" ON)
option(REQUIRE_MPI "Enable and require the use of MPI" ON)
######################### Options ################################################## }}}

# Check for OpenMP support
find_package(OpenMP)

if(OpenMP_CXX_FOUND)
    message(STATUS "OpenMP found")
    add_compile_options(${OpenMP_CXX_FLAGS})
    # If OpenMP GPU offloading support is required, check for a specific version
    if(OpenMP_CXX_VERSION VERSION_GREATER_EQUAL "4.5")
        message(STATUS "OpenMP 4.5 or later found, GPU offloading is supported")
        # Add OpenMP flags to the compile options
        add_compile_options(${OpenMP_CXX_FLAGS})
        # Link the OpenMP libraries to your targets as needed
        # For example:
        # target_link_libraries(your_target_name PRIVATE OpenMP::OpenMP_CXX)
    else()
        message(STATUS "OpenMP version is less than 4.5, GPU offloading may not be supported")
    endif()
else()
    message(STATUS "OpenMP not found")
endif()

######################### Compiler Config ########################################## {{{
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   # Needed for Clang Tooling.
set(CMAKE_CXX_STANDARD 17)              # Default to C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
######################## Global CXX Flags ########################### {{{
if (ENABLE_CODE_QUALITY_CHECKS)
    add_compile_options(-Wall -Wextra -Werror -Wsign-compare
    -Wmissing-field-initializers -Wnon-virtual-dtor -Wno-old-style-cast
    -Wno-cast-function-type -Wno-unused-parameter -Wno-unused-function
    -Wshadow -Wformat -Wuninitialized -Wcast-align -Wconversion
    -Wlogical-op -Wduplicated-cond
    -Wduplicated-branches -Wnull-dereference -Wmisleading-indentation)
endif()
# Ignore -Wuseless-cast -Wdouble-promotion
######################## Global CXX Flags ########################### }}}

######################## CXX Optimizations ########################## {{{
######################## Link-Time Optimization ############## {{{
set(CMAKE_POSITION_INDEPENDENT_CODE OFF) # OFF is ok for static libs
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ${ENABLE_LTO})
######################## Link-Time Optimization ############## }}}

######################## Profile-Guided Optimization ######### {{{
if(ENABLE_PGO)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-fprofile-generate)
        add_link_options(-fprofile-generate)
   elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        add_compile_options(-fprofile-use -fprofile-correction)
        add_link_options(-fprofile-use -fprofile-correction)
    endif()
endif()
######################## Profile-Guided Optimization ######### }}}

######################## Non-Portable Optimizations ########## {{{
set(NON_PORTABLE_CXX_FLAGS)
mark_as_advanced(NON_PORTABLE_CXX_FLAGS)
CHECK_COMPILER_FLAG_SUPPORT("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE NON_PORTABLE_CXX_FLAGS)
if (NOT PORTABLE)
    add_compile_options(${NON_PORTABLE_CXX_FLAGS})
endif ()
######################## Non-Portable Optimizations ########## }}}

######################## CXX Optimizations ########################## }}}
message(STATUS "Compile flags: ${CMAKE_CXX_FLAGS}")
########################### Compiler Config ######################################## }}}

########################### Testing ################################################ {{{
if (BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(external/googletest)
endif ()
########################### Testing ################################################ }}}

########################### Coverage ############################################### {{{
if (CODE_COVERAGE)
    if(CMAKE_COMPILER_IS_GNUCXX)
        message(STATUS "Enabling coverage for GCC")
        add_compile_options(--coverage)
        add_link_options(--coverage)
        add_subdirectory(coverage)
    else ()
        message(WARNING "Coverage only available for GCC ")
    endif ()
endif ()
########################### Coverage ############################################### }}}

########################### Dependencies ########################################### {{{
# Include BOOST
set(BOOST_MIN_VERSION "1.66.0" COMPONENTS program_options serialization) # COMPONENTS program_options
SET(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
SET(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost ${BOOST_MIN_VERSION} REQUIRED)
message(STATUS "Boost Include directory: ${Boost_INCLUDE_DIR}")
message(STATUS "Boost Library directories: ${Boost_LIBRARY_DIRS}")
list(APPEND LIBS ${Boost_LIBRARIES})
list(APPEND LIBS ${CMAKE_DL_LIBS})
message(STATUS "Libraries: ${LIBS}")
include_directories(SYSTEM "${Boost_INCLUDE_DIR}")

########################### Dependencies ########################################### }}}

########################### Includes ############################################### {{{
# Add the emitted folders to Module Search Path
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# add the binary dir include directory for project-config.h
include_directories("${PROJECT_BINARY_DIR}/include")
include_directories("${PROJECT_SOURCE_DIR}/src")
########################### Includes ############################################### {{{

########################### Sources ################################################ {{{
include_directories("${CMAKE_SOURCE_DIR}")  # Include the core headers via "src".
add_subdirectory(src)
########################### Sources ################################################ }}}

########################### Runtime Configs ######################################## {{{
# Needed to print file paths.
add_compile_definitions(PROJECT_SOURCE_DIR="${PROJECT_SOURCE_DIR}")
# Configure a header file to pass some of the CMake settings to the source code
configure_file(
        "${PROJECT_SOURCE_DIR}/project-config.h.in"
        "${PROJECT_BINARY_DIR}/include/project-config.h"
)
########################### Runtime Configs ######################################## }}}
