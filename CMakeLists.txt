cmake_minimum_required(VERSION 3.12)
project(MTMS)

set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread")

find_package(PNGwriter 0.7.0)
FIND_PACKAGE(Boost 1.68 COMPONENTS program_options REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

add_executable(MTMS main.cpp mandelbrot.hpp)

if(PNGwriter_FOUND)
    target_link_libraries(MTMS PRIVATE PNGwriter::PNGwriter ${Boost_LIBRARIES})
endif(PNGwriter_FOUND)
