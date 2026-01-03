# cf-stats-exporter

Cloudflare analytics exporter for Prometheus/Netdata.

**This repository is made for me only. I use it to help me evaluate different
programming language implementations but more importantly how much more
productive I get by using all the syntax sugar available. This code is
not safe, not tested, and not intended for production use, ever.**

## Overview

Polls Cloudflare GraphQL API for HTTP request metrics and exposes them in Prometheus text format on `/metrics`.

## Implementations

This project includes implementations in 8 languages:

| Language | Lines | Core Logic | Binary Size | Build Command |
|----------|-------|------------|-------------|---------------|
| C | 361 | ~243 | 72 KB | `cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build` |
| C++ | 166 | ~101 | 175 KB | `cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build` |
| Zig | 131 | ~76 | 990 KB | `zig build -Doptimize=ReleaseFast` |
| Rust | 137 | ~67 | 3.3 MB | `cargo build --release` |
| Go | 156 | ~81 | 5.4 MB | `go build -ldflags="-s -w" -o cf-exporter ./cmd` |
| Python | 90 | ~56 | N/A | Interpreted |
| JavaScript | 95 | ~55 | N/A | Interpreted (Node.js 18+) |
| TypeScript | 106 | ~55 | N/A | Interpreted (Bun) |

## Configuration

All configuration via environment variables:

| Variable | Required | Default | Description |
|----------|----------|---------|-------------|
| `CF_API_TOKEN` | Yes | - | Cloudflare API bearer token |
| `CF_ZONE_ID` | Yes | - | Zone tag to query |

## Metrics Exposed

```
# HELP cf_http_requests_total HTTP requests by status code
# TYPE cf_http_requests_total counter
cf_http_requests_total{status="200"} 12345
cf_http_requests_total{status="204"} 67890
```

## Build and Run

### Zig
```bash
cd zig
zig build -Doptimize=ReleaseFast
CF_API_TOKEN="..." CF_ZONE_ID="..." ./zig-out/bin/cf_stats_exporter
```

### Rust
```bash
cd rust
cargo build --release
CF_API_TOKEN="..." CF_ZONE_ID="..." ./target/release/cf-stats-exporter
```

### Go
```bash
cd go
go build -ldflags="-s -w" -o cf-exporter ./cmd
CF_API_TOKEN="..." CF_ZONE_ID="..." ./cf-exporter
```

### C
```bash
cd c
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build
CF_API_TOKEN="..." CF_ZONE_ID="..." ./build/cf-exporter
```

### C++
```bash
cd cpp
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build
CF_API_TOKEN="..." CF_ZONE_ID="..." ./build/cf-stats-exporter
```

### Python
```bash
cd python
pip install -r requirements.txt
CF_API_TOKEN="..." CF_ZONE_ID="..." python __main__.py
```

### JavaScript
```bash
cd javascript
CF_API_TOKEN="..." CF_ZONE_ID="..." node src/index.js
```

### TypeScript
```bash
cd typescript
CF_API_TOKEN="..." CF_ZONE_ID="..." bun run src/index.ts
```

## Assumptions and Limitations

- Cloudflare analytics are delayed 1-5 minutes; real-time data not available
- Polling faster than 30-60 seconds provides no benefit
- Single zone support (one zone ID per instance)
- Metrics are held in memory only; no persistence
