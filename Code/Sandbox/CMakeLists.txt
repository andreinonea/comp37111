find_package(fmt REQUIRED)
#find_package(spdlog REQUIRED)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLFW3 REQUIRED)


add_executable(sandbox)
target_sources(sandbox PRIVATE GLFW3Main.cpp)

target_link_libraries(sandbox
	PRIVATE fmt::fmt
	PRIVATE ${OPENGL_LIBRARIES}
	PRIVATE ${GLEW_LIBRARIES}
	PRIVATE ${GLFW3_LIBRARIES}
	PRIVATE OpenGLRenderer
	PRIVATE ParticleSystem
)

target_include_directories(sandbox
	PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
	PRIVATE ${OPENGL_INCLUDE_DIRS}
	PRIVATE ${GLEW_INCLUDE_DIRS}
	PRIVATE ${GLFW3_INCLUDE_DIRS}
)
