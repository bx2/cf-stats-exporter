cmake_minimum_required(VERSION 3.10)
project(cf-stats-exporter C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find libcurl
find_package(CURL REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CURL_INCLUDE_DIRS}
)

# cJSON library
add_library(cjson STATIC
    src/cJSON.c
)

# CF client library
add_library(cfclient STATIC
    src/cf_client.c
)

target_link_libraries(cfclient
    cjson
    ${CURL_LIBRARIES}
)

# Main executable
add_executable(cf-exporter
    src/main.c
)

target_link_libraries(cf-exporter
    cfclient
    cjson
    ${CURL_LIBRARIES}
)

# Compiler warnings
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(cfclient PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(cf-exporter PRIVATE -Wall -Wextra -Wpedantic)
endif()
