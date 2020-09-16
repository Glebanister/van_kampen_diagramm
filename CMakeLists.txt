cmake_minimum_required(VERSION 3.0.0)
project(van_kampen_diagramm)

add_compile_options(-Wall -Wextra)

set(EXE
    ${CMAKE_PROJECT_NAME}
)
set(SOURCES
    src/main.cpp
    src/Graph.cpp
    src/Group.cpp
    src/GroupRepresentationParser.cpp
)

add_executable(${EXE} ${SOURCES})

target_include_directories(${EXE} PRIVATE include)

set_property(TARGET ${EXE}
             PROPERTY CXX_STANDARD 17)
