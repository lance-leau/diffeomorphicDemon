# Minimum CMake version required
cmake_minimum_required(VERSION 3.12)

# Project name
project(DiffeoDemon)

# Find MATLAB
# This will find the MATLAB installation on your system.
find_package(Matlab REQUIRED)

# Define the source files for the project.
set(MEX_SOURCES
    diffeoDemon.c
    demon.c
    tools.c
)

# --- CORRECTED USAGE of matlab_add_mex ---
# The function expects the source files to be listed directly as arguments
# after the NAME specifier. It does not use a "SOURCES" or "HEADERS" keyword.
matlab_add_mex(
    NAME diffeoDemon
    ${MEX_SOURCES}
)

# Set compiler flags for the MEX target
# -g adds debugging information
# -O enables optimizations
target_compile_options(diffeoDemon PRIVATE "-g" "-O")

# Add strict warning flags for the MSVC compiler
if(MSVC)
    # /W4 sets the warning level to 4
    # /WX treats warnings as errors
    target_compile_options(diffeoDemon PRIVATE "/W4" "/WX")
endif()

# The built-in 'clean' target will remove the main build artifacts (mexw64, pdb).
# We create a separate custom target to clean other files, like .pgm files.
add_custom_target(clean_pgm
    # Use CMake's cross-platform remove command
    COMMAND ${CMAKE_COMMAND} -E rm -f "*.pgm"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Removing *.pgm files"
)

# Custom "remake" target that mimics the Makefile's behavior.
# It first calls the standard clean, then our custom pgm clean, then builds.
add_custom_target(remake
    COMMAND ${CMAKE_COMMAND} --build . --target clean
    COMMAND ${CMAKE_COMMAND} --build . --target clean_pgm
    COMMAND ${CMAKE_COMMAND} --build .
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Cleaning all artifacts and then rebuilding the project"
)