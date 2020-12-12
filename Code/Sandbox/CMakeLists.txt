add_executable(sandbox)
target_sources(sandbox PRIVATE GLFW3Main.cpp)

target_link_libraries(sandbox
	PRIVATE fmt::fmt
	PRIVATE "${OPENGL_LIBRARIES}"
	PRIVATE "${GLEW_LIBRARIES}"
	PRIVATE "${GLFW3_LIBRARIES}"
	PRIVATE "${GLM_LIBRARIES}"
	PRIVATE OpenGLRenderer
	PRIVATE ParticleSystem
)

target_include_directories(sandbox
	PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
	PRIVATE "${OPENGL_INCLUDE_DIRS}"
	PRIVATE "${GLEW_INCLUDE_DIRS}"
	PRIVATE "${GLFW3_INCLUDE_DIRS}"
	PRIVATE "${GLM_INCLUDE_DIRS}"
)

add_custom_command(TARGET sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/"
        $<TARGET_FILE_DIR:sandbox>
)

add_custom_command(TARGET sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Textures/"
        $<TARGET_FILE_DIR:sandbox>
)

if (WIN32)
	add_custom_command(TARGET sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/_deps/glew-2.1.0/bin/Release/x64/glew32.dll"
        $<TARGET_FILE_DIR:sandbox>
	)
endif(WIN32)
