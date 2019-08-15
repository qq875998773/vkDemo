set(ASSIMP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/assimp")

file(GLOB ASSIMP_INCLUDE_DIR ${ASSIMP_DIR}/include/*.h
                             ${ASSIMP_DIR}/include/*.hpp
				             ${ASSIMP_DIR}/include/*.inl
				             ${ASSIMP_DIR}/include/Compiler/*.h
				             ${ASSIMP_DIR}/include/AndroidJNI/*.h)

find_library(ASSIMP_LIB NAMES assimp PATHS "${ASSIMP_DIR}/lib")

find_package_handle_standard_args(ASSIMP DEFAULT_MSG ASSIMP_LIB ASSIMP_INCLUDE_DIR)

add_library(assimp::assimp UNKNOWN IMPORTED)
set_target_properties(assimp::assimp PROPERTIES IMPORTED_LOCATION "${ASSIMP_LIB}")
set_target_properties(assimp::assimp PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${ASSIMP_INCLUDE_DIR}")

get_filename_component(ASSIMP_DLL ${GLFW3_LIB} NAME_WE)
set(ASSIMP_DLL ${ASSIMP_DIR}/bin/assimp-vc140-mt.dll)
