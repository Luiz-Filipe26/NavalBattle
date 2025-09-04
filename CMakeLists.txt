cmake_minimum_required(VERSION 3.16)
project(NavalBattle)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE) # link time optimization
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Opção de build de produção ---
option(PRODUCTION_BUILD "Make this a production build!" OFF)

if(MSVC) 
    add_compile_options(/arch:AVX2) # SIMD
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# --- Fetch FreeType ---
include(FetchContent)

FetchContent_Declare(
    freetype
    GIT_REPOSITORY https://github.com/freetype/freetype.git
    GIT_TAG VER-2-12-1
)
FetchContent_MakeAvailable(freetype)

# --- Configure SFML para usar FreeType baixado ---
set(SFML_USE_SYSTEM_FREETYPE ON CACHE BOOL "Use system FreeType" FORCE)
set(FREETYPE_INCLUDE_DIRS "${freetype_SOURCE_DIR}/include")
set(FREETYPE_LIBRARIES freetype) # sempre "freetype" no Windows

# --- Disable SFML modules que não usamos ---
set(SFML_BUILD_AUDIO OFF CACHE INTERNAL "")
set(SFML_BUILD_NETWORK OFF CACHE INTERNAL "")
set(SFML_BUILD_TESTING OFF CACHE INTERNAL "")
set(SFML_BUILD_EXAMPLES OFF CACHE INTERNAL "")

# --- Adiciona SFML (depois do FreeType) ---
add_subdirectory(thirdparty/SFML-2.6.1)

# --- Fontes do projeto ---
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable("${CMAKE_PROJECT_NAME}")
set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY CXX_STANDARD 17)
target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${MY_SOURCES})
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

# --- Definições de recursos ---
if(PRODUCTION_BUILD)
    target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="./resources/" PRODUCTION_BUILD=1)
else()
    target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/" PRODUCTION_BUILD=0)
endif()

# --- Link SFML + FreeType explicitamente ---
target_link_libraries("${CMAKE_PROJECT_NAME}" 
    sfml-graphics
    sfml-window
    sfml-system
    freetype
#    sfml-audio
#    sfml-network
)
