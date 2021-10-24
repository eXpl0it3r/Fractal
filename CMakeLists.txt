cmake_minimum_required(VERSION 3.15)

project(Fractal)

option(FRACTAL_STATIC_LINKING "Link the Fractal dependencies statically" ON)

# Find SFML
if(FRACTAL_STATIC_LINKING)
    set(SFML_STATIC_LIBRARIES true)
endif()
find_package(SFML 2.5 COMPONENTS graphics REQUIRED)
if(NOT SFML_FOUND)
    message("\n-> SFML libraries were not found. Set SFML_DIR to SFML's CMake Config path (generally located in \"lib/cmake/SFML\").")
    message("-> Make sure the SFML libraries with the same configuration (Release/Debug, Static/Dynamic) exist.\n")
endif()

add_executable(${PROJECT_NAME} 
    src/main.cpp
    src/Application.hpp
    src/Application.cpp
    src/Fractal.hpp
    src/Fractal.cpp
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${PROJECT_NAME} PRIVATE /std:c++latest)
else()
    target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
endif()

# Use static runtime libraries
if(FRACTAL_STATIC_LINKING)
    if(WIN32 AND MSVC)
        set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    elseif(CMAKE_COMPILER_IS_GNUCXX)
        # Note: Doesn't work for TDM compiler, since it's compiling the runtime libs statically by default
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
    endif()
endif()

target_link_libraries(${PROJECT_NAME} sfml-graphics)

# Install executable
install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION .)

# Install resources
install(DIRECTORY bin/
        DESTINATION .)