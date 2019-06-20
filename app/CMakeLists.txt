# Add source to this project's executable.
set(SHADER_DIR ${PROJECT_SOURCE_DIR}/assets/shaders)
set(DEPS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(${DEPS_DIR}/SPIRV-Cross)

add_definitions( -DROOTPROJECTDIR="${CMAKE_SOURCE_DIR}" )  # 定义ROOTPROJECTDIR为项目根路径

include_directories("vk/"
                    "include/glm/"
                    "include/SPRIV-Cross/include/"
                    "include/gli/"
                    "include/stb/"
                    "include/imgui/"
                    "include/tinyobjloader/"
                    "$ENV{VULKAN_SDK}/Include")

# 更改这种批量添加的方式,后期新增文件容易遗漏,改用set设置路径
#file(GLOB PROJECT_HEADERS ${INCLUDE_DIR}/*.h
#                          ${INCLUDE_DIR}/*.cpp)
#file(GLOB PROJECT_VULKAN_SOURCES ${SRC_DIR}/vk/*.cpp
#                                 ${SRC_DIR}/vk/*.h)
#file(GLOB PROJECT_SOURCES ${SRC_DIR}/main.cpp)
#file(GLOB PROJECT_SHADERS ${SHADER_DIR}/*.comp
#                          ${SHADER_DIR}/*.frag
#                          ${SHADER_DIR}/*.geom
#                          ${SHADER_DIR}/*.vert
#                          ${SHADER_DIR}/*.tess
#                          ${SHADER_DIR}/*.glsl)

#file(GLOB PROJECT_CONFIGS "CMakeLists.txt"
#                          "${SHADER_DIR}/shader_info.txt")


set(PROJECT_HEADERS application/camera.cpp
                    application/camera.h
                    application/deferredRenderer.cpp
                    application/deferredRenderer.h
                    application/entity.cpp
                    application/entity.h
                    application/glfwWindow.cpp
                    application/glfwWindow.h
                    application/inputManager.cpp
                    application/inputManager.h
                    application/light.cpp
                    application/light.h
                    application/material.cpp
                    application/material.h
                    application/mesh.cpp
                    application/mesh.h
                    application/model.cpp
                    application/model.h
                    application/modelManager.cpp
                    application/modelManager.h
                    application/scene.cpp
                    application/scene.h
                    application/settings.cpp
                    application/settings.h
                    application/skyBox.cpp
                    application/skyBox.h
                    application/textureManager.cpp
                    application/textureManager.h
                    application/application.cpp
                    application/application.h)

set(PROJECT_VULKAN_SOURCES vk/vkBuffer.cpp
                           vk/vkBuffer.h
                           vk/vkDevice.cpp
                           vk/vkDevice.h
                           vk/vkImage.cpp
                           vk/vkImage.h
                           vk/vkImageView.cpp
                           vk/vkImageView.h
                           vk/vkPipeline.cpp
                           vk/vkPipeline.h
                           vk/vkRenderPass.cpp
                           vk/vkRenderPass.h
                           vk/vkSampler.cpp
                           vk/vkSampler.h
                           vk/vkShaderModule.cpp
                           vk/vkShaderModule.h
                           vk/vkSwapChain.cpp
                           vk/vkSwapChain.h
                           vk/vkUtils.hpp)

set(PROJECT_SOURCES main.cpp)

set(PROJECT_SHADERS ${SHADER_DIR}/shader_info.txt
                    ${SHADER_DIR}/dummy.frag
                    ${SHADER_DIR}/dummy.vert
                    ${SHADER_DIR}/PBR_IBL.frag
                    ${SHADER_DIR}/PBR_IBL.vert
                    ${SHADER_DIR}/phong.frag
                    ${SHADER_DIR}/phong.vert
                    ${SHADER_DIR}/skybox.frag
                    ${SHADER_DIR}/skybox.vert
                    ${SHADER_DIR}/triangle.frag
                    ${SHADER_DIR}/triangle.vert)

set(IMGUI_SORUCES
    include/imgui/imconfig.h
    include/imgui/imgui.cpp
    include/imgui/imgui.h
    include/imgui/imgui_demo.cpp
    include/imgui/imgui_draw.cpp
    include/imgui/imgui_internal.h
    include/imgui/imgui_widgets.cpp
    include/imgui/imstb_rectpack.h
    include/imgui/imstb_textedit.h
    include/imgui/imstb_truetype.h)


# Vulkan shaders need compiling prior to running
#if(MSVC)
#    # todo: need check if any of them have changed
#    # todo: only recompile if any of them have changed
#    foreach(SHADER_FILE ${PROJECT_SHADERS})
#        string(FIND ${SHADER_FILE} "." DOT_POSITION)
#        string(SUBSTRING ${SHADER_FILE} 0 ${DOT_POSITION} SHADER_NAME)
#        math(EXPR DOT_POSITION "${DOT_POSITION}+1")
#        string(SUBSTRING ${SHADER_FILE} ${DOT_POSITION} 4 SHADER_EXTENSION)
#
#        message(STATUS "Compiling shader: ${SHADER_NAME}.${SHADER_EXTENSION}")
#        execute_process(COMMAND CMD /c $ENV{VULKAN_SDK}/Bin/glslangValidator.exe -V -o ${SHADER_NAME}_${SHADER_EXTENSION}.spv ${SHADER_FILE})
#    endforeach(SHADER_FILE)
#endif()

source_group("application" FILES ${PROJECT_HEADERS})
source_group("shaders" FILES ${PROJECT_SHADERS})
source_group("app" FILES ${PROJECT_SOURCES})
source_group("app\\vk" FILES ${PROJECT_VULKAN_SOURCES})
source_group("app\\include\\imgui" FILES ${IMGUI_SORUCES})

add_definitions(-DGLFW_INCLUDE_NONE -DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")

add_executable(vkTest  ${PROJECT_SOURCES}
                       ${PROJECT_VULKAN_SOURCES}
                       ${PROJECT_HEADERS}
                       ${PROJECT_SHADERS}
					   ${IMGUI_SORUCES})

target_include_directories(vkTest PUBLIC "include")
target_include_directories(vkTest PUBLIC "application")
target_link_libraries(vkTest Vulkan::Vulkan spirv-cross-core spirv-cross-glsl spirv-cross-cpp  glfw3::glfw3)

#set_target_properties(vkTest PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build") # 输出项目exe


add_custom_command(TARGET vkTest POST_BUILD        # 拷贝DLL到运行目录
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${GLFW3_DLL}
        $<TARGET_FILE_DIR:vkTest>)