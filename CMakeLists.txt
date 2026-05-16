cmake_minimum_required(VERSION 4.0)
project(2d-game)

# Compile options
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compile flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -DDEBUG")
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DNDEBUG")
endif()

# Dependencies
include(FetchContent)

FetchContent_Declare(
    SFML
    URL https://github.com/SFML/SFML/releases/download/3.1.0/SFML-3.1.0-linux-gcc-64-bit.tar.gz
    URL_HASH SHA256=d2ac8bb63aee77b833d84fcdbc489dbdf17f57f2c8fe2216e6b02996875377e3
)
FetchContent_Declare(
    EnTT
    URL https://github.com/skypjack/entt/archive/refs/tags/v3.16.0.zip
    URL_HASH SHA256=b34b11f00ea9abbdda63c4cfdcee319047882376abf3c3ef95436ed138293bd1
)

FetchContent_MakeAvailable(SFML)
FetchContent_MakeAvailable(EnTT)

# Executable
add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PRIVATE
    game/main.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
    game/
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    sfml-graphics
    sfml-window
    sfml-system
    EnTT::EnTT
)
