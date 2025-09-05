cmake_minimum_required(VERSION 3.16)
project(NavalBattle)

# --- CONFIGURAÇÃO DE BUILD ---
set(BUILD_SHARED_LIBS OFF) 
set(CMAKE_CXX_STANDARD 17)

# Desativando LTO para evitar problemas de linkagem com bibliotecas estáticas
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE) 

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(PRODUCTION_BUILD "Make this a production build!" OFF)

if(MSVC)
    add_compile_options(/arch:AVX2)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# --- COMPILA O FREETYPE SEPARADAMENTE ---
# Garanta que a pasta com os fontes do FreeType está em thirdparty/FreeType
add_subdirectory(thirdparty/FreeType)

# --- CONFIGURA O SFML ---
set(SFML_USE_STATIC_LIBS TRUE)
# Desligamos isso para que o SFML não tente usar seu próprio FreeType
set(SFML_USE_EXTLIBS OFF) 

# --- DESATIVA MÓDULOS DO SFML ---
set(SFML_BUILD_AUDIO OFF CACHE INTERNAL "")
set(SFML_BUILD_NETWORK OFF CACHE INTERNAL "")
set(SFML_BUILD_TESTING OFF CACHE INTERNAL "")
set(SFML_BUILD_EXAMPLES OFF CACHE INTERNAL "")

# --- Adiciona o SFML ---
add_subdirectory(thirdparty/SFML-2.6.1)

# --- Fontes do projeto ---
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable("${CMAKE_PROJECT_NAME}")
set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY CXX_STANDARD 17)
target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${MY_SOURCES})
target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC SFML_STATIC)

if(PRODUCTION_BUILD)
    target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="./resources/" PRODUCTION_BUILD=1)
else()
    target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/" PRODUCTION_BUILD=0)
endif()

# --- LINKAGEM FINAL CORRIGIDA ---
# Linkamos o seu jogo com o SFML E com o FreeType que compilamos manualmente.
target_link_libraries("${CMAKE_PROJECT_NAME}" 
    PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
    freetype # <- Este é o alvo FreeType que criamos
)