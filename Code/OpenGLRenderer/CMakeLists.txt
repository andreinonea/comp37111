add_library(OpenGLRenderer STATIC)
target_sources(OpenGLRenderer PRIVATE
	Shader.cpp
	stb_image.cpp
 )

target_link_libraries(OpenGLRenderer
	PRIVATE fmt::fmt
)

target_include_directories(OpenGLRenderer
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Include"
	PUBLIC "${GLEW_INCLUDE_DIRS}"
	PUBLIC "${GLM_INCLUDE_DIRS}"
)
