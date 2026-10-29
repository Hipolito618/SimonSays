cmake_minimum_required(VERSION 3.10)
project(SimonDice)

set(CMAKE_CXX_STANDARD 17)

# Incluir carpetas locales
include_directories(. Librerias)

# Archivos fuente
file(GLOB SOURCES "*.cpp")

# Si tienen glad.c dentro de Librerias/glad/
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Librerias/glad/glad.c")
    list(APPEND SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Librerias/glad/glad.c")
endif()

add_executable(SimonSays ${SOURCES})

# Enlaces según el sistema operativo
if(WIN32)
    find_package(OpenGL REQUIRED)
    target_link_libraries(SimonSays PRIVATE OpenGL::GL)
    # En Windows necesita la librería de GLFW (glfw3)
    target_link_libraries(SimonSays PRIVATE glfw)
else()
    target_link_libraries(SimonSays PRIVATE GL glfw dl pthread)
endif()