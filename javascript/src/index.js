import { CFClient } from './cf-client.js';

const apiToken = process.env.CF_API_TOKEN;
const zoneId = process.env.CF_ZONE_ID;

if (!apiToken) {
  console.error('CF_API_TOKEN environment variable is required');
  process.exit(1);
}
if (!zoneId) {
  console.error('CF_ZONE_ID environment variable is required');
  process.exit(1);
}

async function main() {
  const client = new CFClient(apiToken);

  const metrics = await client.fetchMetrics({
    zoneId: zoneId,
    startDate: '2025-12-17T15:00:00Z',
    endDate: '2025-12-18T15:00:00Z',
    limit: 10
  });

  console.log(JSON.stringify(metrics, null, 2));
}

main().catch(error => {
  console.error('Error:', error.message);
  process.exit(1);
});
