project(tinyxml)

include_directories(include/tinyxml/)
file(GLOB SOURCES src/*.cpp)

add_library(tinyxml ${SOURCES})