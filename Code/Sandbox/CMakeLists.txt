find_package(fmt REQUIRED)
#find_package(spdlog REQUIRED)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(glm REQUIRED)

set(CMAKE_BUILD_TYPE Debug)

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
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/default.vert"
        $<TARGET_FILE_DIR:sandbox>
)

add_custom_command(TARGET sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/Shaders/default.frag"
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
