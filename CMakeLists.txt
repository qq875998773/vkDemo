# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8) # cmake 最低版本

project ("VKDemo") # 项目名

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake") # 追加cmake目录

find_package(Vulkan REQUIRED) # vulkan依赖 因为系统安装了vulkan所以不需要把文件放到项目目录

find_package(GLFW3 REQUIRED) # 找到glfw3依赖 因为glfw3包含有lib 所以需要添加依赖

find_package(GLEW REQUIRED) # 找到glew依赖 因为glew包含有lib 所以需要添加依赖

# Include sub-projects.
add_subdirectory("shaders")
add_subdirectory("resources")
add_subdirectory("assets")

add_subdirectory ("app")
