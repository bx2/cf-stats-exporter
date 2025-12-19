import json
import os
import sys
from cf_client import CFClient, CFFetchOptions


def main():
    api_token = os.environ.get("CF_API_TOKEN")
    zone_id = os.environ.get("CF_ZONE_ID")

    if not api_token:
        print("CF_API_TOKEN environment variable is required", file=sys.stderr)
        sys.exit(1)
    if not zone_id:
        print("CF_ZONE_ID environment variable is required", file=sys.stderr)
        sys.exit(1)

    client = CFClient(api_token=api_token)

    metrics = client.fetch_metrics(
        CFFetchOptions(
            zone_id=zone_id,
            start_date="2025-12-17T15:00:00Z",
            end_date="2025-12-18T15:00:00Z",
            limit=10,
        )
    )

    # Convert to list of dicts for JSON output
    output = [{"status": m.status, "count": m.count} for m in metrics]
    print(json.dumps(output))


if __name__ == "__main__":
    main()
