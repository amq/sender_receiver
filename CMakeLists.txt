cmake_minimum_required(VERSION 2.8.7)
find_package(Doxygen)

project(sender_receiver)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -Wextra -Wstrict-prototypes -pedantic")
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -fprofile-arcs -ftest-coverage")
set(CMAKE_EXE_LINKER_FLAGS="-fprofile-arcs -ftest-coverage")

add_executable(sender src/sender.c src/shared.c src/shared.h)
add_executable(receiver src/receiver.c src/shared.c src/shared.h)

if(DOXYGEN_FOUND)
    add_custom_target(doc
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMAND ${DOXYGEN_EXECUTABLE})
endif()