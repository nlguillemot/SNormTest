cmake_minimum_required(VERSION 3.11)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

find_package(GLEW REQUIRED)

find_package(OpenGL REQUIRED)

add_executable(SNormTest main.cpp)

target_include_directories(SNormTest PRIVATE ${GLFW_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS})
target_link_libraries(SNormTest ${GLFW_LIBRARIES} ${GLEW_LIBRARIES} OpenGL::GL)
