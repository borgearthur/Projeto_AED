cmake_minimum_required(VERSION 3.15)
project(AED_2024_2 C)

# Definindo a versão do padrão C usada
set(CMAKE_C_STANDARD 99)

# Encontra a biblioteca Raylib
find_package(raylib REQUIRED)

if (raylib_FOUND)
    include_directories(${raylib_INCLUDE_DIRS})
    message(STATUS "Raylib include dirs: ${raylib_INCLUDE_DIRS}")
    message(STATUS "Raylib libraries: ${raylib_LIBRARIES}")
else()
    message(FATAL_ERROR "Raylib not found! Please install Raylib.")
endif()

# Adicionando o executável e o arquivo fonte necessário
add_executable(AED_2024_2 main.c)  # Substitua "JOGO_AED.c" pelo nome correto do arquivo fonte principal

# Liga a biblioteca Raylib ao seu executável
target_link_libraries(AED_2024_2 ${raylib_LIBRARIES})

# Para evitar problemas de compatibilidade em algumas plataformas
if(APPLE)
    target_link_libraries(AED_2024_2 "-framework OpenGL" "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
endif()
