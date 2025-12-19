# Cloudflare Stats Exporter - C Implementation

C port of the Cloudflare metrics exporter using libcurl and cJSON.

## Directory Structure

```
c/
├── include/
│   └── cf_client.h         # Public API header
├── src/
│   ├── cf_client.c         # CF client implementation
│   └── main.c              # Entry point
├── vendor/
│   ├── cJSON.h             # cJSON library header
│   └── cJSON.c             # cJSON library implementation
├── CMakeLists.txt          # Build configuration
└── README.md               # This file
```

## Requirements

- CMake 3.10 or higher
- C11 compatible compiler
- libcurl development libraries

### Installing Dependencies

**macOS:**
```bash
brew install cmake curl
```

**Ubuntu/Debian:**
```bash
sudo apt-get install cmake libcurl4-openssl-dev
```

**Fedora/RHEL:**
```bash
sudo dnf install cmake libcurl-devel
```

## Building

```bash
cd c
mkdir build
cd build
cmake ..
make
```

The executable `cf-exporter` will be created in the `build` directory.

## Running

```bash
./build/cf-exporter
```

Output is a JSON array of status code metrics:
```json
[
  {"status": 200, "count": 12345},
  {"status": 404, "count": 678},
  ...
]
```

## API Usage

```c
#include "cf_client.h"

// Create client
cf_client_t *client = cf_client_new("your_api_token");

// Setup fetch options
cf_fetch_options_t opts = {
    .zone_id = "your_zone_id",
    .start_date = "2025-12-17T15:00:00Z",
    .end_date = "2025-12-18T15:00:00Z",
    .limit = 10,
};

// Fetch metrics
cf_status_metric_t *metrics = NULL;
size_t metrics_count = 0;
int result = cf_client_fetch_metrics(client, &opts, &metrics, &metrics_count);

if (result == CF_OK) {
    // Process metrics
    for (size_t i = 0; i < metrics_count; i++) {
        printf("Status: %u, Count: %lu\n",
               metrics[i].status, metrics[i].count);
    }
    free(metrics);
}

// Cleanup
cf_client_free(client);
```

## Error Codes

- `CF_OK` (0): Success
- `CF_ERROR_MEMORY` (-1): Memory allocation failure
- `CF_ERROR_HTTP` (-2): HTTP request failed
- `CF_ERROR_JSON` (-3): JSON parsing error
- `CF_ERROR_BUFFER_TOO_SMALL` (-4): Buffer overflow
- `CF_ERROR_INVALID_RESPONSE` (-5): Invalid API response

## Implementation Notes

- Uses libcurl for HTTP requests
- Uses cJSON for JSON parsing (vendored in project)
- Manual memory management following C idioms
- Thread-safe with proper resource cleanup
- Compatible with C11 standard
