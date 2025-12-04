cmake_minimum_required(VERSION 3.10)
project(AdvancedVertexSkinning)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenGL REQUIRED)

# Try to find GLFW and GLEW using find_package first, fallback to manual linking
find_package(glfw3 QUIET)
find_package(GLEW QUIET)
find_package(assimp QUIET)

# Platform-specific library search paths
if(NOT glfw3_FOUND)
    if(APPLE)
        # macOS with Homebrew
        find_path(GLFW_INCLUDE_DIR GLFW/glfw3.h
            PATHS /opt/homebrew/include /usr/local/include)
        find_library(GLFW_LIBRARY glfw
            PATHS /opt/homebrew/lib /usr/local/lib)
    elseif(WIN32)
        # Windows - common paths for vcpkg, manual installs
        find_path(GLFW_INCLUDE_DIR GLFW/glfw3.h
            PATHS 
                "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include"
                "${CMAKE_CURRENT_SOURCE_DIR}/libs/include"
                "C:/vcpkg/installed/x64-windows/include"
                "C:/Program Files/GLFW/include")
        find_library(GLFW_LIBRARY 
            NAMES glfw3 glfw
            PATHS 
                "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib"
                "${CMAKE_CURRENT_SOURCE_DIR}/libs/lib"
                "C:/vcpkg/installed/x64-windows/lib"
                "C:/Program Files/GLFW/lib")
    else()
        # Linux
        find_path(GLFW_INCLUDE_DIR GLFW/glfw3.h
            PATHS /usr/include /usr/local/include)
        find_library(GLFW_LIBRARY glfw
            PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu)
    endif()
    
    if(GLFW_INCLUDE_DIR AND GLFW_LIBRARY)
        set(GLFW_FOUND TRUE)
    endif()
endif()

if(NOT GLEW_FOUND)
    if(APPLE)
        find_path(GLEW_INCLUDE_DIR GL/glew.h
            PATHS /opt/homebrew/include /usr/local/include)
        find_library(GLEW_LIBRARY GLEW
            PATHS /opt/homebrew/lib /usr/local/lib)
    elseif(WIN32)
        find_path(GLEW_INCLUDE_DIR GL/glew.h
            PATHS 
                "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include"
                "${CMAKE_CURRENT_SOURCE_DIR}/libs/include"
                "C:/vcpkg/installed/x64-windows/include"
                "C:/Program Files/GLEW/include")
        find_library(GLEW_LIBRARY 
            NAMES glew32 GLEW
            PATHS 
                "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib"
                "${CMAKE_CURRENT_SOURCE_DIR}/libs/lib"
                "C:/vcpkg/installed/x64-windows/lib"
                "C:/Program Files/GLEW/lib")
    else()
        find_path(GLEW_INCLUDE_DIR GL/glew.h
            PATHS /usr/include /usr/local/include)
        find_library(GLEW_LIBRARY GLEW
            PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu)
    endif()
    
    if(GLEW_INCLUDE_DIR AND GLEW_LIBRARY)
        set(GLEW_FOUND TRUE)
    endif()
endif()

if(NOT assimp_FOUND)
    if(APPLE)
        find_path(ASSIMP_INCLUDE_DIR assimp/scene.h
            PATHS /opt/homebrew/include /usr/local/include)
        find_library(ASSIMP_LIBRARY assimp
            PATHS /opt/homebrew/lib /usr/local/lib)
    elseif(WIN32)
        find_path(ASSIMP_INCLUDE_DIR assimp/scene.h
            PATHS 
                "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/include"
                "${CMAKE_CURRENT_SOURCE_DIR}/libs/include"
                "C:/vcpkg/installed/x64-windows/include"
                "C:/Program Files/Assimp/include")
        find_library(ASSIMP_LIBRARY 
            NAMES assimp assimp-vc143-mt
            PATHS 
                "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/lib"
                "${CMAKE_CURRENT_SOURCE_DIR}/libs/lib"
                "C:/vcpkg/installed/x64-windows/lib"
                "C:/Program Files/Assimp/lib")
    else()
        find_path(ASSIMP_INCLUDE_DIR assimp/scene.h
            PATHS /usr/include /usr/local/include)
        find_library(ASSIMP_LIBRARY assimp
            PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu)
    endif()
    
    if(ASSIMP_INCLUDE_DIR AND ASSIMP_LIBRARY)
        set(ASSIMP_FOUND TRUE)
    endif()
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/vendor)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/vendor/glm)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/vendor/imgui)

# Source files
set(SOURCES
    src/Application.cpp
    src/Lamp.cpp
    src/Mesh.cpp
    src/Model.cpp
    src/Skeleton.cpp
    src/vendor/imgui/imgui.cpp
    src/vendor/imgui/imgui_demo.cpp
    src/vendor/imgui/imgui_draw.cpp
    src/vendor/imgui/imgui_impl_glfw.cpp
    src/vendor/imgui/imgui_impl_opengl3.cpp
    src/vendor/imgui/imgui_widgets.cpp
    src/vendor/glm/detail/glm.cpp
)

# Header files
set(HEADERS
    src/Camera.h
    src/Lamp.h
    src/Log.h
    src/Mesh.h
    src/Model.h
    src/Shader.h
    src/Skeleton.h
    src/stb_image.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
if(glfw3_FOUND)
    target_link_libraries(${PROJECT_NAME} OpenGL::GL glfw)
elseif(GLFW_FOUND)
    target_link_libraries(${PROJECT_NAME} OpenGL::GL ${GLFW_LIBRARY})
    target_include_directories(${PROJECT_NAME} PRIVATE ${GLFW_INCLUDE_DIR})
else()
    message(FATAL_ERROR "GLFW not found. Please install GLFW.")
endif()

if(GLEW_FOUND AND TARGET GLEW::GLEW)
    target_link_libraries(${PROJECT_NAME} GLEW::GLEW)
elseif(GLEW_FOUND)
    target_link_libraries(${PROJECT_NAME} ${GLEW_LIBRARY})
    target_include_directories(${PROJECT_NAME} PRIVATE ${GLEW_INCLUDE_DIR})
else()
    message(FATAL_ERROR "GLEW not found. Please install GLEW.")
endif()

if(assimp_FOUND AND TARGET assimp::assimp)
    target_link_libraries(${PROJECT_NAME} assimp::assimp)
elseif(ASSIMP_FOUND)
    target_link_libraries(${PROJECT_NAME} ${ASSIMP_LIBRARY})
    target_include_directories(${PROJECT_NAME} PRIVATE ${ASSIMP_INCLUDE_DIR})
else()
    message(FATAL_ERROR "Assimp not found. Please install Assimp.")
endif()

# Windows-specific additional libraries
if(WIN32)
    target_link_libraries(${PROJECT_NAME} opengl32)
endif()

# Copy shader files to build directory maintaining directory structure
file(GLOB SHADER_FILES "res/shaders/*")
foreach(SHADER ${SHADER_FILES})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    configure_file(${SHADER} ${CMAKE_BINARY_DIR}/res/shaders/${SHADER_NAME} COPYONLY)
endforeach()

# Create res/object directory structure (for model files)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/res/asset/test)

# Set working directory for shaders
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")