cmake_minimum_required(VERSION 3.11)

project(shell-client CXX)

set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

find_package(Boost 1.81.0 REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

FILE(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
set(GCC_OPTIONS -g -O0 -Werror -Wall -Wextra -fsanitize=leak -fsanitize=undefined -fsanitize=address)
target_compile_options(${PROJECT_NAME} PRIVATE ${GCC_OPTIONS})
target_link_options(${PROJECT_NAME} PRIVATE ${GCC_OPTIONS})

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT ${PROJECT_NAME})