cmake_minimum_required(VERSION 3.10)
project(OrbitSimulation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML REQUIRED COMPONENTS graphics window system)

set(SOURCES
    main.cpp
    engine/engine.cpp
    physics/physics.cpp
)
add_executable(orbitsim ${SOURCES})

target_include_directories(orbitsim PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(orbitsim sfml-graphics sfml-window sfml-system)

# Копируем файлы ресурсов из папки graphics/
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/graphics/G.ttf   ${CMAKE_CURRENT_BINARY_DIR}/G.ttf   COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/graphics/F.jpg   ${CMAKE_CURRENT_BINARY_DIR}/F.jpg   COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.json      ${CMAKE_CURRENT_BINARY_DIR}/config.json COPYONLY)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/data)

message(STATUS "Build dir: ${CMAKE_CURRENT_BINARY_DIR}")