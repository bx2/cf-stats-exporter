from dataclasses import dataclass
from typing import List
import requests


@dataclass
class CFStatusMetric:
    status: int
    count: int


@dataclass
class CFFetchOptions:
    zone_id: str
    start_date: str
    end_date: str
    limit: int


class CFClient:
    BASE_URL = "https://api.cloudflare.com/client/v4/graphql"

    def __init__(self, api_token: str):
        self.api_token = api_token

    def fetch_metrics(self, opts: CFFetchOptions) -> List[CFStatusMetric]:
        """Fetch HTTP status metrics from Cloudflare GraphQL API."""
        # Build GraphQL query
        query = (
            f'{{"query": "{{ viewer {{ zones(filter: {{zoneTag: \\"{opts.zone_id}\\"}}) '
            f"{{ httpRequestsAdaptiveGroups(limit: {opts.limit}, "
            f'filter: {{datetime_geq: \\"{opts.start_date}\\", datetime_leq: \\"{opts.end_date}\\"}}, '
            f"orderBy: [count_DESC]) {{ count dimensions {{ edgeResponseStatus }} }}}}}}}}\"}}"
        )

        # Build headers
        headers = {
            "User-Agent": "cf-stats-exporter/1.0",
            "Content-Type": "application/json",
            "Authorization": f"Bearer {self.api_token}",
        }

        # Send request
        response = requests.post(
            self.BASE_URL,
            data=query,
            headers=headers,
        )

        if response.status_code != 200:
            raise Exception(f"HTTP request failed with status {response.status_code}")

        # Parse response
        data = response.json()
        raw_groups = data["data"]["viewer"]["zones"][0]["httpRequestsAdaptiveGroups"]

        # Convert to CFStatusMetric list
        metrics = []
        for group in raw_groups:
            metrics.append(
                CFStatusMetric(
                    status=group["dimensions"]["edgeResponseStatus"],
                    count=group["count"],
                )
            )

        return metrics
