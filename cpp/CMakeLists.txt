cmake_minimum_required(VERSION 3.14)
project(cf-stats-exporter-cpp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# Fetch cpr library for HTTP requests
FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG 1.11.1
)
set(CPR_ENABLE_SSL ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(cpr)

# Fetch nlohmann/json for JSON parsing
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# Main executable
add_executable(cf-stats-exporter
    src/main.cpp
    src/cf_client.cpp
)

target_include_directories(cf-stats-exporter PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(cf-stats-exporter PRIVATE
    cpr::cpr
    nlohmann_json::nlohmann_json
)

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(cf-stats-exporter PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(cf-stats-exporter PRIVATE
        /W4
    )
endif()
