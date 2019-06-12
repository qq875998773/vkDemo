# Add source to this project's executable.
set(APP_SRC
    "main.cpp"
)

include_directories("vk/")

add_executable (vk ${APP_SRC})

target_include_directories(vk PUBLIC "include") # 
target_link_libraries(vk Vulkan::Vulkan glfw3::glfw3)
add_dependencies(vk Shaders Resources)

add_custom_command(TARGET vk POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${GLFW3_DLL}
        $<TARGET_FILE_DIR:vk>
)