cmake_minimum_required(VERSION 3.20)
project(ConvoyLauncher)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Write resource file for icon
file(WRITE ${CMAKE_SOURCE_DIR}/convoy.rc "IDI_ICON1 ICON DISCARDABLE \"convoy.ico\"\n")

add_executable(ConvoyLauncher WIN32
    main.cpp
    convoy.rc
)

target_link_libraries(ConvoyLauncher wininet shell32)
target_compile_options(ConvoyLauncher PRIVATE /W3)
set_target_properties(ConvoyLauncher PROPERTIES OUTPUT_NAME "ConvoyLauncher")
