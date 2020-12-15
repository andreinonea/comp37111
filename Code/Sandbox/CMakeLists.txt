add_executable(sandbox)
target_sources(sandbox PRIVATE GLFW3Main.cpp)

target_link_libraries(sandbox PRIVATE
	"${OPENGL_LIBRARIES}"
	"${GLEW_LIBRARIES}"
	"${GLFW3_LIBRARIES}"
	"${FREETYPE_LIBRARIES}"
	OpenGLRenderer
	ParticleSystem
)

target_include_directories(sandbox PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}"
	"${OPENGL_INCLUDE_DIRS}"
	"${GLEW_INCLUDE_DIRS}"
	"${GLFW3_INCLUDE_DIRS}"
	"${GLM_INCLUDE_DIRS}"
	"${FREETYPE_INCLUDE_DIRS}"
)

if (WIN32)
	add_custom_command(TARGET sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/_deps/glew-2.1.0/bin/Release/x64/glew32.dll"
        $<TARGET_FILE_DIR:sandbox>
	)
endif(WIN32)

target_compile_definitions(sandbox PRIVATE
    "-DDEFAULT_TEXTURE_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/Textures/\""
)
