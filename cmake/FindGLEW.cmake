set(GLEW_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/glew")

find_library(GLEW_LIB NAMES glew PATHS "${GLEW_DIR}/lib/x64")
find_path(GLEW_INCLUDE_DIR NAMES GL/glew.h PATHS "${GLEW_DIR}/include")

find_package_handle_standard_args(GLEW DEFAULT_MSG GLEW_LIB GLEW_INCLUDE_DIR)

add_library(glew::glew UNKNOWN IMPORTED)
set_target_properties(glew::glew PROPERTIES IMPORTED_LOCATION "${GLEW_LIB}")
set_target_properties(glew::glew PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${GLEW_INCLUDE_DIR}")

get_filename_component(GLEW_DLL ${GLEW_LIB} NAME_WE)
set(GLEW_DLL ${GLEW_DIR}/bin/x64/${GLEW_DLL}.dll)
