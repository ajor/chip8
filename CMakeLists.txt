cmake_minimum_required(VERSION 2.8.12)
project(Chip8)
add_executable(chip8 main.cpp chip8.cpp)

target_compile_options(chip8 PRIVATE "--std=c++11")
target_compile_options(chip8 PRIVATE "-Wall")
target_compile_options(chip8 PRIVATE "-pedantic")

find_package(OpenGL REQUIRED)
if (OPENGL_FOUND)
  include_directories(${OPENGL_INCLUDE_DIR})
  target_link_libraries(chip8 ${OPENGL_LIBRARIES})
endif()

find_package(GLEW REQUIRED)
if (GLEW_FOUND)
  include_directories(${GLEW_INCLUDE_DIRS})
  target_link_libraries(chip8 ${GLEW_LIBRARIES})
endif()

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

if (GLFW_FOUND)
  include_directories(${GLFW_INCLUDE_DIRS})
  target_link_libraries(chip8 ${GLFW_LIBRARIES})
endif()

target_link_libraries(chip8 pthread)