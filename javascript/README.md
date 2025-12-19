# Cloudflare Stats Exporter (JavaScript)

Node.js port of the Cloudflare HTTP request metrics exporter.

## Requirements

- Node.js 18 or higher (uses native fetch API)
- No external dependencies

## Usage

```bash
npm start
```

Or run directly:

```bash
node src/index.js
```

## Output

Outputs a JSON array of HTTP status metrics to stdout:

```json
[
  {
    "status": 204,
    "count": 790116848
  },
  {
    "status": 201,
    "count": 17670738
  }
]
```

## Implementation

- `src/cf-client.js` - CFClient class with async fetchMetrics() method
- `src/index.js` - Entry point with hardcoded credentials and parameters
- Uses native fetch (Node.js 18+), no external dependencies
- JSDoc type annotations for better IDE support
- ES modules (type: module in package.json)

## API

### CFClient

```javascript
const client = new CFClient(apiToken);

const metrics = await client.fetchMetrics({
  zoneId: 'zone-id',
  startDate: '2025-12-17T15:00:00Z',
  endDate: '2025-12-18T15:00:00Z',
  limit: 10
});
```

Returns an array of `{status: number, count: number}` objects ordered by count descending.
