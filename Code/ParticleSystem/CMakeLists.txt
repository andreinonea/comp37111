add_library(ParticleSystem STATIC)
target_sources(ParticleSystem PRIVATE ParticleSystem.cpp)

target_include_directories(ParticleSystem
	PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Include"
	PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"
)
