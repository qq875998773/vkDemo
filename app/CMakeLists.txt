# Add source to this project's executable.
#set(APPLICATION_SOURCES
#    application/camera.cpp
#    application/camera.h
#	application/deferredRenderer.cpp
#	application/deferredRenderer.h
#	application/entity.cpp
#	application/entity.h
#	application/glfwWindow.cpp
#	application/glfwWindow.h
#	application/inputManager.cpp
#	application/inputManager.h
#	application/light.cpp
#	application/light.h
#	application/material.cpp
#	application/material.h
#	application/mesh.cpp
#	application/mesh.h
#	application/model.cpp
#	application/model.h
#	application/modelManager.cpp
#	application/modelManager.h
#	application/scene.cpp
#	application/scene.h
#	application/settings.cpp
#	application/settings.h
#	application/skyBox.cpp
#	application/skyBox.h
#	application/textureManager.cpp
#	application/textureManager.h
#	application/virtualVistaEngine.cpp
#	application/virtualVistaEngine.h)
#
#set(VK_SOURCES
#    vk/vkBuffer.cpp
#	vk/vkBuffer.h
#	vk/vkDevice.cpp
#	vk/vkDevice.h
#	vk/vkImage.cpp
#	vk/vkImage.h
#	vk/vkImageView.cpp
#	vk/vkImageView.h
#	vk/vkPipeline.cpp
#	vk/vkPipeline.h
#	vk/vkRenderPass.cpp
#	vk/vkRenderPass.h
#	vk/vkSampler,cpp
#	vk/vkSampler.h
#	vk/vkShaderModule.cpp
#	vk/vkShaderModule.h
#	vk/vkSwapChain.cpp
#	vk/vkSwapChain.h
#	vk/vkUtils.hpp)
#
#set(MAIN_SOURCES main.cpp)
#
#set(SOURCES
#    ${APPLICATION_SOURCES}
#	${VK_SOURCES}
#	${MAIN_SOURCES})
#
#source_group("application" FILES ${APPLICATION_SOURCES})
#source_group("vk" FILES ${VK_SOURCES})

set(APP_SRC
    "oldmain.cpp"
)
set(APP_DEMO
    "main.cpp"
)

include_directories("vk/"
                    "application/")

add_executable (vk ${APP_SRC})

add_executable (vkDemo ${APP_DEMO})

target_include_directories(vk PUBLIC "include") # 
target_link_libraries(vk Vulkan::Vulkan glfw3::glfw3)
add_dependencies(vk Shaders Resources)

target_include_directories(vkDemo PUBLIC "include") # 
target_link_libraries(vkDemo Vulkan::Vulkan glfw3::glfw3 gli::gli)
add_dependencies(vkDemo Assets)


add_custom_command(TARGET vk POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${GLFW3_DLL}
        $<TARGET_FILE_DIR:vk>
)

add_custom_command(TARGET vkDemo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${GLFW3_DLL}
        $<TARGET_FILE_DIR:vkDemo>
)