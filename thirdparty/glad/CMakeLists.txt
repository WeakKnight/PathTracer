project(glad)

include_directories(include/)
file(GLOB SOURCES src/*.c)

add_library(glad ${SOURCES})