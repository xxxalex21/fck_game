cmake_minimum_required(VERSION 3.5)

project(fck_game)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML COMPONENTS Graphics System Window Network Audio REQUIRED)
find_package(spdlog REQUIRED)
find_package(tomlplusplus REQUIRED)
find_package(pugixml REQUIRED)
find_package(sol2 REQUIRED)

file(GLOB_RECURSE PROJECT_CPP_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE PROJECT_C_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE PROJECT_H ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)

add_executable(${PROJECT_NAME} ${PROJECT_H} ${PROJECT_CPP_SRC} ${PROJECT_C_SRC})

target_link_libraries(${PROJECT_NAME} PUBLIC
    SFML::Graphics
    SFML::System
    SFML::Window
    SFML::Network
    SFML::Audio
    spdlog::spdlog
    tomlplusplus::tomlplusplus
    pugixml::pugixml
    sol2::sol2)
