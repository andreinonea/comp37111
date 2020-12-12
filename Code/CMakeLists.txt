find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(glm REQUIRED)

add_subdirectory(OpenGLRenderer)
add_subdirectory(ParticleSystem)
add_subdirectory(Sandbox)
