# Add source to this project's executable.
#set(APP_SRC
#    "oldmain.cpp"
#)
#set(APP_DEMO
#    "main.cpp"
#)
#
#set(VK_DIR "vk")
#set(APPLICATION_DIR "application")
#
#include_directories("vk/"
#                    "application/")
#
#file(GLOB PROJECT_VK ${VK_DIR}/*.h
#                     ${VK_DIR}/*.cpp)
#file(GLOB PROJECT_APPLICATION ${APPLICATION_DIR}/*.h
#                              ${APPLICATION_DIR}/*.cpp)
#
#
#source_group("vk" FILES ${PROJECT_VK})
#source_group("application" FILES ${PROJECT_APPLICATION})
#
#
##add_executable (vk ${APP_SRC})
#
#add_executable (vkDemo ${APP_DEMO}
#	                   ${PROJECT_VK}
#					   ${PROJECT_APPLICATION})
#
##target_include_directories(vk PUBLIC "include") # 
##target_link_libraries(vk Vulkan::Vulkan glfw3::glfw3)
##add_dependencies(vk Shaders Resources)
#
#target_include_directories(vkDemo PUBLIC "include") # 
#target_link_libraries(vkDemo Vulkan::Vulkan glfw3::glfw3 gli::gli)
#add_dependencies(vkDemo Assets)
#
##add_custom_command(TARGET vk POST_BUILD
##    COMMAND ${CMAKE_COMMAND} -E copy_if_different
##        ${GLFW3_DLL}
##        $<TARGET_FILE_DIR:vk>
##)
#
#add_custom_command(TARGET vkDemo POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        ${GLFW3_DLL}
#        $<TARGET_FILE_DIR:vkDemo>
#)

set(SHADER_DIR "assets/shaders")
set(SRC_DIR "app")
set(INCLUDE_DIR "application")
set(DEPS_DIR "include")
#set(GLFW_DIR "3rdparty")

#add_subdirectory(${GLFW_DIR}/glfw3)

add_subdirectory(${DEPS_DIR}/SPIRV-Cross)

#message(STATUS "Using module to find Vulkan")
#find_package(Vulkan)

#if(NOT Vulkan_FOUND)
#    message(STATUS "Vulkan not found through module. Attempting to manually find.")
#    if(WIN32)
#        find_library(Vulkan_LIBRARY NAMES vulkan-1 vulkan HINTS "$ENV{VULKAN_SDK}/Lib/" REQUIRED)
#        if(Vulkan_LIBRARY)
#            set(Vulkan_FOUND ON)
#            message(STATUS "Using bundled Vulkan library version.")
#        endif()
#    endif(WIN32)
#endif()
#
#if (NOT Vulkan_FOUND)
#    message(FATAL_ERROR "Could not find Vulkan library! Maybe you forgot to reboot after changing environment variables or Vulkan update.")
#endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    add_definitions( -DROOTPROJECTDIR="${CMAKE_SOURCE_DIR}" )
    # for some reason, the MSVC compiler's optimizations executes vital Vulkan commands out of order
    string(REPLACE "/O2" "/Od" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++11")
endif()

include_directories("vk/"
                    "include/glm/"
                    "include/SPRIV-Cross/include/"
                    "include/gli/"
                    "include/stb/"
                    "include/tinyobjloader/"
					"3rdparty/glfw3/include"
					"$ENV{VULKAN_SDK}/Include")

file(GLOB PROJECT_HEADERS ${INCLUDE_DIR}/*.h
                          ${INCLUDE_DIR}/*.cpp)
file(GLOB PROJECT_VULKAN_SOURCES ${SRC_DIR}/vk/*.cpp
                                 ${SRC_DIR}/vk/*.h)
file(GLOB PROJECT_SOURCES ${SRC_DIR}/*.cpp)
file(GLOB PROJECT_SHADERS ${SHADER_DIR}/*.comp
                          ${SHADER_DIR}/*.frag
                          ${SHADER_DIR}/*.geom
                          ${SHADER_DIR}/*.vert
                          ${SHADER_DIR}/*.tess
                          ${SHADER_DIR}/*.glsl)

file(GLOB PROJECT_CONFIGS "CMakeLists.txt"
                          "${SHADER_DIR}/shader_info.txt")

# Vulkan shaders need compiling prior to running
if(MSVC)
    # todo: need check if any of them have changed
    # todo: only recompile if any of them have changed
    foreach(SHADER_FILE ${PROJECT_SHADERS})
        string(FIND ${SHADER_FILE} "." DOT_POSITION)
        string(SUBSTRING ${SHADER_FILE} 0 ${DOT_POSITION} SHADER_NAME)
        math(EXPR DOT_POSITION "${DOT_POSITION}+1")
        string(SUBSTRING ${SHADER_FILE} ${DOT_POSITION} 4 SHADER_EXTENSION)

        message(STATUS "Compiling shader: ${SHADER_NAME}.${SHADER_EXTENSION}")
        execute_process(COMMAND CMD /c $ENV{VULKAN_SDK}/Bin/glslangValidator.exe -V -o ${SHADER_NAME}_${SHADER_EXTENSION}.spv ${SHADER_FILE})
    endforeach(SHADER_FILE)
endif()

source_group("application" FILES ${PROJECT_HEADERS})
source_group("shaders" FILES ${PROJECT_SHADERS})
source_group("app" FILES ${PROJECT_SOURCES})
source_group("app\\vk" FILES ${PROJECT_VULKAN_SOURCES})

add_definitions(-DGLFW_INCLUDE_NONE -DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")

add_executable(vkTest  ${PROJECT_SOURCES}
                       ${PROJECT_SOURCES}
                       ${PROJECT_VULKAN_SOURCES}
                       ${PROJECT_HEADERS}
                       ${PROJECT_SHADERS}
                       ${PROJECT_CONFIGS})

target_include_directories(vkTest PUBLIC "include")
#target_link_libraries(vkTest glfw ${GLFW_LIBRARIES} ${Vulkan_LIBRARY} spirv-cross-core spirv-cross-glsl spirv-cross-cpp glfw3::glfw3)
target_link_libraries(vkTest Vulkan::Vulkan glfw ${GLFW_LIBRARIES} glfw3::glfw3)

set_target_properties(vkTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build") # 输出项目