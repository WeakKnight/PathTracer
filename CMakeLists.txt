cmake_minimum_required(VERSION 3.0.0)
project(RayTracer VERSION 0.1.0)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wvla-extension -std=c++11")
endif()
# include(CTest)
# enable_testing()
 
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_SOURCE_DIR}/bin>)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG $<1:${CMAKE_SOURCE_DIR}/bin>)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets
     DESTINATION ${CMAKE_SOURCE_DIR}/bin)

include_directories(include/
                    thirdparty/glfw/include/
                    thirdparty/glad/include/
                    thirdparty/spdlog/include/
                    thirdparty/imgui/)

option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory(thirdparty/glfw)

add_subdirectory(thirdparty/spdlog)

file(GLOB HEADERS include/*.h)
file(GLOB SOURCES src/*.cpp)
file(GLOB GLAD_SOURCES thirdparty/glad/src/glad.c)
file(GLOB IMGUI_SOURCES thirdparty/imgui/*.cpp)

add_executable(RayTracer ${SOURCES} ${HEADERS} ${GLAD_SOURCES} ${IMGUI_SOURCES})
target_link_libraries(RayTracer spdlog glfw  ${GLFW_LIBRARIES} ${GLAD_LIBRARIES})
# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)

# if (MSVC)
# elseif (MINGW)
# elseif (APPLE)
#     target_link_libraries(RayTracer "-framework Cocoa")
#     target_link_libraries(RayTracer "-framework QuartzCore")
#     target_link_libraries(RayTracer "-framework Metal")
#     target_link_libraries(RayTracer "-framework MetalKit")
# elseif (UNIX)
#     if(USE_WAYLAND_API)
#         target_link_libraries(RayTracer -lwayland-client -lwayland-cursor)
#     else()
#         target_link_libraries(RayTracer -lX11)
#     endif()
# endif()