# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("VKDemo")

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

find_package(Vulkan REQUIRED)

find_package(GLFW3 REQUIRED)

add_custom_target(Resources)

set(RESOURCES_DIR "resources")
set(RESOURCES_SRC ${PROJECT_SOURCE_DIR}/${RESOURCES_DIR})
set(RESOURCES_DST ${CMAKE_BINARY_DIR}/${RESOURCES_DIR})

if (WIN32)
    FILE(TO_NATIVE_PATH ${RESOURCES_SRC} RESOURCES_SRC_WIN)
    FILE(TO_NATIVE_PATH ${RESOURCES_DST} RESOURCES_DST_WIN)
    add_custom_command(TARGET Resources POST_BUILD
        COMMAND IF NOT EXIST ${RESOURCES_DST_WIN} mklink /J ${RESOURCES_DST_WIN} ${RESOURCES_SRC_WIN}
        DEPENDS ${RESOURCES_DST_WIN}
    )
elseif (UNIX)
    add_custom_command(TARGET Resources POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink ${RESOURCES_SRC} ${RESOURCES_DST}
        DEPENDS ${RESOURCES_DST}
    )
endif (WIN32)

# Include sub-projects.
add_subdirectory ("app")
