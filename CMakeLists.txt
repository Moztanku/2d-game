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

FetchContent_Declare(mSFML
    URL https://github.com/Moztanku/minimal-dependencies/releases/download/v1.0/sfml-3.1.0-linux.tar.gz
    URL_HASH SHA256=62ba089cf031a6eb1c4919ad47f331db6365a13402a2a2fdeff910eb1fc5158e)
FetchContent_MakeAvailable(mSFML)

FetchContent_Declare(EnTT
    URL https://github.com/skypjack/entt/archive/refs/tags/v3.16.0.zip
    URL_HASH SHA256=b34b11f00ea9abbdda63c4cfdcee319047882376abf3c3ef95436ed138293bd1)
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
    mSFML::Graphics
    mSFML::Window
    mSFML::System
    EnTT::EnTT
)
