add_library(OpenGLRenderer STATIC)
target_sources(OpenGLRenderer PRIVATE
	stb_image.cpp
	Shader.cpp
	Gui/Text.cpp
 )

target_include_directories(OpenGLRenderer PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/Include"
	"${CMAKE_CURRENT_SOURCE_DIR}/Include/Gui"
	"${GLEW_INCLUDE_DIRS}"
	"${GLM_INCLUDE_DIRS}"
	"${FREETYPE_INCLUDE_DIRS}"
)

target_compile_definitions(OpenGLRenderer PUBLIC
    "-DDEFAULT_SHADER_PATH=\"${CMAKE_SOURCE_DIR}/Code/Sandbox/Shaders/\""
)

target_compile_definitions(OpenGLRenderer PUBLIC
    "-DDEFAULT_FONT_PATH=\"${CMAKE_SOURCE_DIR}/Code/Sandbox/Fonts/\""
)
