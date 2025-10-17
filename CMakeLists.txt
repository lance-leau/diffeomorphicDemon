cmake_minimum_required(VERSION 3.12)

project(DiffeoDemon)

find_package(Matlab REQUIRED)

# Set the output to root
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

set(MEX_SOURCES
    diffeoDemon.c
    demon.c
    tools.c
)

# Add the MEX target
matlab_add_mex(
    NAME diffeoDemon
    ${MEX_SOURCES}
)

target_compile_options(diffeoDemon PRIVATE "-g" "-O")

if(MSVC)
    target_compile_options(diffeoDemon PRIVATE "/W4" "/WX")
endif()

add_custom_target(clean_pgm
    COMMAND ${CMAKE_COMMAND} -E rm -f "*.pgm"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Removing *.pgm files from the root directory"
)

add_custom_target(remake
    COMMAND ${CMAKE_COMMAND} --build . --target clean
    COMMAND ${CMAKE_COMMAND} --build . --target clean_pgm
    COMMAND ${CMAKE_COMMAND} --build .
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Cleaning all artifacts and then rebuilding the project"
)