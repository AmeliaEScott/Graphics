cmake_minimum_required(VERSION 2.8)

project(OpenGLIntro)

option (GLFW_BUILD_EXAMPLES OFF)
option (GLFW_BUILD_TESTS OFF)
option (GLFW_INSTALL OFF)
option (USE_MSVC_RUNTIME_LIBRARY_DLL OFF)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenGL)

if (OPENGL_FOUND)
	message("OpenGL Correctly Found in ${OPENGL_INCLUDE_DIR}")
	include_directories(${OPENGL_INCLUDE_DIR})
else (OPENGL_FOUND)
	message("OpenGL environment missing")
endif (OPENGL_FOUND)

##############################################################
# Generating OpenGL profile
##############################################################
message("Executing gl3w_gen.py")
execute_process(COMMAND python gl3w_gen.py --root ${PROJECT_BINARY_DIR}/libs/gl3w WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/libs/gl3w)

##############################################################
# Includes
##############################################################
include_directories(libs/glfw/include)
include_directories(libs/gl3w/include)
include_directories(libs/glm)
include_directories(libs/imgui)

##############################################################
# Externals
##############################################################
add_subdirectory (libs/glfw)
add_subdirectory (libs/gl3w)

#####################################################################
# imgui
#####################################################################
set(IMGUI_DIR libs/imgui)
file(GLOB IMGUI_SOURCES ${IMGUI_DIR}/*.cpp ${IMGUI_DIR}/*.h )
add_library(imgui ${IMGUI_SOURCES} ${IMGUI_DIR}/extra_fonts/binary_to_compressed_c.cpp ${IMGUI_DIR}/examples/opengl3_example/imgui_impl_glfw_gl3.cpp)

##############################################################
# Targets
##############################################################
file(GLOB_RECURSE SOURCES sources/*.cpp src/*.h)
add_executable(Lighting ${SOURCES} sources/Bezier.cpp sources/Bezier.h)

##############################################################
# Linkage
##############################################################
target_link_libraries(Lighting gl3w glfw imgui ${OPENGL_LIBRARIES})
