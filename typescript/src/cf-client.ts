export interface CFStatusMetric {
  status: number;
  count: number;
}

export interface CFFetchOptions {
  zoneId: string;
  startDate: string;
  endDate: string;
  limit: number;
}

interface GraphQLResponse {
  errors?: { message: string }[];
  data?: {
    viewer?: {
      zones?: {
        httpRequestsAdaptiveGroups: {
          count: number;
          dimensions: {
            edgeResponseStatus: number;
          };
        }[];
      }[];
    };
  };
}

export class CFClient {
  private static readonly BASE_URL = 'https://api.cloudflare.com/client/v4/graphql';
  private readonly apiToken: string;

  constructor(apiToken: string) {
    this.apiToken = apiToken;
  }

  async fetchMetrics(options: CFFetchOptions): Promise<CFStatusMetric[]> {
    const { zoneId, startDate, endDate, limit } = options;

    const query = {
      query: `{ viewer { zones(filter: {zoneTag: "${zoneId}"}) { httpRequestsAdaptiveGroups(limit: ${limit}, filter: {datetime_geq: "${startDate}", datetime_leq: "${endDate}"}, orderBy: [count_DESC]) { count dimensions { edgeResponseStatus } }}}}`
    };

    const response = await fetch(CFClient.BASE_URL, {
      method: 'POST',
      headers: {
        'User-Agent': 'cf-stats-exporter/1.0',
        'Content-Type': 'application/json',
        'Authorization': `Bearer ${this.apiToken}`
      },
      body: JSON.stringify(query)
    });

    if (!response.ok) {
      throw new Error(`HTTP request failed with status ${response.status}`);
    }

    const data: GraphQLResponse = await response.json();

    if (data.errors && data.errors.length > 0) {
      throw new Error(`GraphQL errors: ${JSON.stringify(data.errors)}`);
    }

    if (!data.data?.viewer?.zones || data.data.viewer.zones.length === 0) {
      throw new Error(`Invalid response structure: ${JSON.stringify(data)}`);
    }

    const rawGroups = data.data.viewer.zones[0].httpRequestsAdaptiveGroups;

    return rawGroups.map(group => ({
      status: group.dimensions.edgeResponseStatus,
      count: group.count
    }));
  }
}
