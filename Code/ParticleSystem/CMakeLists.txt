add_library(ParticleSystem STATIC)
target_sources(ParticleSystem PRIVATE
	ParticleSystem.cpp
	Primitives/Cube.cpp
	Primitives/Quad.cpp
 )

target_link_libraries(ParticleSystem PRIVATE
	OpenGLRenderer
)

target_include_directories(ParticleSystem
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Include"
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Primitives/Include"
	PUBLIC "${GLM_INCLUDE_DIRS}"
)

