cmake_minimum_required(VERSION 3.17)
project(reader_obj C)

set(CMAKE_C_STANDARD 99)

add_executable(reader_obj main.c)
target_link_libraries(reader_obj libglu32.a libfreeglut.a libopengl32.a libglut32.a libglaux.a)