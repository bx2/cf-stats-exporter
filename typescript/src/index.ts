import { CFClient } from './cf-client';

function getRequiredEnv(name: string): string {
  const value = process.env[name];
  if (!value) {
    console.error(`${name} environment variable is required`);
    process.exit(1);
  }
  return value;
}

const apiToken = getRequiredEnv('CF_API_TOKEN');
const zoneId = getRequiredEnv('CF_ZONE_ID');

async function main(): Promise<void> {
  const client = new CFClient(apiToken);

  const metrics = await client.fetchMetrics({
    zoneId,
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
