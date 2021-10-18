cmake_minimum_required(VERSION 3.15)

project(Fractal)

set(CMAKE_CXX_STANDARD 17)

# Find SFML
set(SFML_STATIC_LIBRARIES true)
find_package(SFML 2.5 COMPONENTS graphics REQUIRED)
if(NOT SFML_FOUND)
	message("\n-> SFML libraries were not found. Set SFML_DIR to SFML's CMake Config path (generally located in \"lib/cmake/SFML\").")
	message("-> Make sure the SFML libraries with the same configuration (Release/Debug, Static/Dynamic) exist.\n")
endif()

# Find SFGUI
if(WIN32)
	SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/cmake/")
endif()
set(SFGUI_STATIC_LIBRARIES true)
find_package(SFGUI 0.4 REQUIRED)
if(NOT SFGUI_FOUND)
	message("\n-> SFGUI library was not found. Set SFGUI_ROOT to SFGUI's source path and ensure CMAKE_MODULES_PATH includes the directory where FindSFGUI.cmake is located.")
	message("-> Make sure the SFGUI library with the same configuration (Release/Debug, Static/Dynamic) exist.\n")
endif()

add_executable(Fractal 
	src/main.cpp
	src/Application.hpp
	src/Application.cpp
	src/Fractal.hpp
	src/Fractal.cpp
)

if(WIN32 AND MSVC)
	set_target_properties(Fractal PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
endif()

# Use static runtime libraries
if(WIN32 AND MSVC)
	set_property(TARGET Fractal PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif(CMAKE_COMPILER_IS_GNUCXX)
	# Note: Doesn't work for TDM compiler, since it's compiling the runtime libs statically by default
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
endif()

target_include_directories(Fractal PRIVATE $<BUILD_INTERFACE:${SFGUI_INCLUDE_DIR}>)
target_link_libraries(Fractal ${SFGUI_LIBRARY} ${SFGUI_DEPENDENCIES} sfml-graphics)

# Install executable
install(TARGETS Fractal
		RUNTIME DESTINATION .)

# Install resources
install(DIRECTORY bin/
		DESTINATION .)