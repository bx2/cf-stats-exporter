/**
 * @typedef {Object} CFStatusMetric
 * @property {number} status - HTTP status code
 * @property {number} count - Number of requests with this status
 */

/**
 * @typedef {Object} CFFetchOptions
 * @property {string} zoneId - Cloudflare zone ID
 * @property {string} startDate - Start date in ISO 8601 format
 * @property {string} endDate - End date in ISO 8601 format
 * @property {number} limit - Maximum number of results to return
 */

/**
 * Cloudflare API client for fetching HTTP request metrics
 */
export class CFClient {
  static BASE_URL = 'https://api.cloudflare.com/client/v4/graphql';

  /**
   * @param {string} apiToken - Cloudflare API token
   */
  constructor(apiToken) {
    this.apiToken = apiToken;
  }

  /**
   * Fetch HTTP request metrics from Cloudflare GraphQL API
   * @param {CFFetchOptions} options - Fetch options
   * @returns {Promise<CFStatusMetric[]>} Array of status metrics
   */
  async fetchMetrics(options) {
    const { zoneId, startDate, endDate, limit } = options;

    // Build GraphQL query
    const query = {
      query: `{ viewer { zones(filter: {zoneTag: "${zoneId}"}) { httpRequestsAdaptiveGroups(limit: ${limit}, filter: {datetime_geq: "${startDate}", datetime_leq: "${endDate}"}, orderBy: [count_DESC]) { count dimensions { edgeResponseStatus } }}}}`
    };

    // Make HTTP request
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

    const data = await response.json();

    // Check for errors in response
    if (data.errors && data.errors.length > 0) {
      throw new Error(`GraphQL errors: ${JSON.stringify(data.errors)}`);
    }

    if (!data.data || !data.data.viewer || !data.data.viewer.zones || data.data.viewer.zones.length === 0) {
      throw new Error(`Invalid response structure: ${JSON.stringify(data)}`);
    }

    // Extract metrics from nested response structure
    const rawGroups = data.data.viewer.zones[0].httpRequestsAdaptiveGroups;

    // Convert to status metric array
    return rawGroups.map(group => ({
      status: group.dimensions.edgeResponseStatus,
      count: group.count
    }));
  }
}
