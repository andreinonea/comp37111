# Find the GLFW3 library
#
# The following variables will be defined in case the library is found
#
#   GLFW3_FOUND
#   GLFW3_INCLUDE_DIRS
#   GLFW3_LIBRARIES

find_path(GLFW3_INCLUDE_DIRS GLFW/glfw3.h)
find_library(GLFW3_LIBRARIES NAMES glfw glfw3)

mark_as_advanced(GLFW3_INCLUDE_DIRS GLFW3_LIBRARIES)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLFW3 DEFAULT_MSG
	GLFW3_LIBRARIES
	GLFW3_INCLUDE_DIRS
)
