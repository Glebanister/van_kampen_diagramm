cmake_minimum_required(VERSION 3.0.0)
project(vankamp-vis
        LANGUAGES CXX
        VERSION 1.0.0)

add_compile_options(-Wall -Wextra -Wshadow -O2 -DNDEBUG)

set(EXE
    ${CMAKE_PROJECT_NAME}
)
set(SOURCES
    src/main.cpp
    src/Graph.cpp
    src/Group.cpp
    src/GroupRepresentationParser.cpp
    src/VanKampenUtils.cpp
)

add_executable(${EXE} ${SOURCES})

target_include_directories(${EXE} PRIVATE include)
target_include_directories(${EXE} PRIVATE extern/cxxopts/include)

set_property(TARGET ${EXE}
             PROPERTY CXX_STANDARD 17)
