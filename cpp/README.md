# Cloudflare Stats Exporter - C++ Implementation

C++ port of the Cloudflare metrics exporter.

## Requirements

- CMake 3.14 or later
- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- Internet connection (for FetchContent to download dependencies)

## Dependencies

Dependencies are automatically downloaded and built via CMake FetchContent:
- [cpr](https://github.com/libcpr/cpr) - C++ HTTP requests library
- [nlohmann/json](https://github.com/nlohmann/json) - JSON parsing library

## Building

```bash
cd cpp
mkdir build
cd build
cmake ..
cmake --build .
```

## Running

```bash
./cf-stats-exporter
```

The program will:
1. Query Cloudflare GraphQL API for HTTP request metrics
2. Output a JSON array of status code metrics to stdout
3. Debug messages are printed to stderr

## Output Format

```json
[
  {"status": 200, "count": 123456},
  {"status": 304, "count": 45678},
  {"status": 404, "count": 1234}
]
```

## Configuration

Configuration via environment variables:
- `CF_API_TOKEN` - Cloudflare API token
- `CF_ZONE_ID` - Zone ID to query

## Project Structure

```
cpp/
├── CMakeLists.txt           # Build configuration
├── include/
│   └── cf_client.hpp        # CFClient class and structs
└── src/
    ├── cf_client.cpp        # CFClient implementation
    └── main.cpp             # Entry point
```
