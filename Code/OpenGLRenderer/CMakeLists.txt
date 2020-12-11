find_package(fmt REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

add_library(OpenGLRenderer STATIC)
target_sources(OpenGLRenderer
	PRIVATE "Shader.cpp" "stb_image.cpp"
 "Include/Platform.h" "Include/Camera.h")

target_link_libraries(OpenGLRenderer
	PRIVATE fmt::fmt
)

target_include_directories(OpenGLRenderer
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Include"
	PRIVATE "${GLEW_INCLUDE_DIRS}"
	PRIVATE "${GLM_INCLUDE_DIRS}"
)
