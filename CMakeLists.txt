cmake_minimum_required(VERSION 3.26)
project(GameOfLife C)

set(CMAKE_C_STANDARD 17)
set(SDL_DIR "C:/SDL3")

include_directories("${SDL_DIR}/include")

# Link SDL3 library
link_directories("${SDL_DIR}/lib")
find_library(SDL3_LIBRARY SDL3 PATHS "${SDL_DIR}/lib" REQUIRED)

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")
endif()


# Create executable - Step 1
file(GLOB_RECURSE SOURCES # Collect all source files
        "src/*.c"
        "src/**/*.c"
)
file(GLOB_RECURSE HEADERS # Collect all header files
        "src/*.h"
        "src/**/*.h"
)

# Create executable - Step 2
add_executable(GameOfLife ${SOURCES} ${HEADERS})
target_link_libraries(GameOfLife ${SDL3_LIBRARY})

# Ensure SDL3.dll is present
set(SDL_DLL_PATH "${SDL_DIR}/bin/SDL3.dll")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
add_custom_command(
        TARGET GameOfLife POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL_DLL_PATH}"
        "${EXECUTABLE_OUTPUT_PATH}"
)
