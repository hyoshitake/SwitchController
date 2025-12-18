cmake_minimum_required(VERSION 3.26)
project(hello C CXX)

find_package(SDL3 REQUIRED)

add_executable(gamepad-polling)

target_sources(gamepad-polling
PRIVATE
    gamepad-polling.c
)

target_link_libraries(gamepad-polling SDL3::SDL3)