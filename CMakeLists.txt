cmake_minimum_required(VERSION 3.10)
project(QUADTREE)

# include directories

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

option(GTEST "Use Google Test" OFF)

if (GTEST)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)
    
    enable_testing()
    add_subdirectory(test)  # 假設你的測試程式在 tests 資料夾
endif()

# add source files

set(SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/test/main.cpp
)

# set compiler version
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# add executable and debug flag
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -Wpedantic -Werror")
add_executable(QUADTREE ${SOURCES})
