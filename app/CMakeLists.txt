# Add source to this project's executable.
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
target_link_libraries(vkDemo Vulkan::Vulkan glfw3::glfw3)
add_dependencies(vkDemo Shaders Resources)


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