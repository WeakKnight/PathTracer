cmake_minimum_required(VERSION 3.0.0)
project(RayTracer VERSION 0.1.0)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wvla-extension -std=c++11")
endif()
# include(CTest)
# enable_testing()

include_directories(include/
                    lib/minifb/include/
                    lib/spdlog/include/spdlog/)

option(USE_METAL_API ON)
add_subdirectory(lib/minifb)
add_subdirectory(lib/spdlog)

file(GLOB HEADERS include/*.h)
file(GLOB SOURCES src/*.cpp)

add_executable(RayTracer ${SOURCES} ${HEADERS})
target_link_libraries(RayTracer minifb spdlog)
# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)

if (MSVC)
elseif (MINGW)
elseif (APPLE)
    target_link_libraries(RayTracer "-framework Cocoa")
    target_link_libraries(RayTracer "-framework QuartzCore")
    target_link_libraries(RayTracer "-framework Metal")
    target_link_libraries(RayTracer "-framework MetalKit")
elseif (UNIX)
    if(USE_WAYLAND_API)
        target_link_libraries(RayTracer -lwayland-client -lwayland-cursor)
    else()
        target_link_libraries(RayTracer -lX11)
    endif()
endif()