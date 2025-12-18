cmake_minimum_required(VERSION 3.26)
project(SwitchController C CXX)

find_package(SDL3 REQUIRED)

add_executable(main)

target_sources(main
PRIVATE
    main.c
    joycon.c
)

target_link_libraries(main SDL3::SDL3)
