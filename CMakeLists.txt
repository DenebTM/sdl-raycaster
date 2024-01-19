set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Og -ggdb3 -ffast-math")
cmake_minimum_required(VERSION 3.5)
project(sdl-raycaster VERSION 0.1.0)

set(CMAKE_BUILD_DIR ${CMAKE_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR})

set(SOURCES main.cpp events.cpp player.cpp map.cpp render.cpp)
add_executable(sdl-raycaster ${SOURCES})

# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)

set(SDL2PP_WITH_IMAGE ON)
add_subdirectory(libSDL2pp)
target_link_libraries(sdl-raycaster SDL2pp::SDL2pp)
