set(CMAKE_CXX_STANDARD 17)
cmake_minimum_required(VERSION 3.0.0)
project(sdlapp VERSION 0.1.0)

set(SOURCES main.cpp events.cpp)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR})
add_executable(sdlapp ${SOURCES})

# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)

set(SDL2PP_WITH_IMAGE ON)
add_subdirectory(libSDL2pp)
target_link_libraries(sdlapp SDL2pp::SDL2pp)