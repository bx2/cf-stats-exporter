package cfclient

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
)

const baseURL = "https://api.cloudflare.com/client/v4/graphql"

// CFClient is a client for the Cloudflare GraphQL API
type CFClient struct {
	httpClient *http.Client
	apiToken   string
}

// NewCFClient creates a new Cloudflare API client
func NewCFClient(apiToken string) *CFClient {
	return &CFClient{
		httpClient: &http.Client{},
		apiToken:   apiToken,
	}
}

// FetchMetrics fetches HTTP status metrics from Cloudflare
func (c *CFClient) FetchMetrics(opts CFFetchOptions) ([]CFStatusMetric, error) {
	// Build GraphQL query
	query := fmt.Sprintf(
		`{"query": "{ viewer { zones(filter: {zoneTag: \"%s\"}) { httpRequestsAdaptiveGroups(limit: %d, filter: {datetime_geq: \"%s\", datetime_leq: \"%s\"}, orderBy: [count_DESC]) { count dimensions { edgeResponseStatus } }}}}"}`,
		opts.ZoneID, opts.Limit, opts.StartDate, opts.EndDate,
	)

	// Create HTTP request
	req, err := http.NewRequest("POST", baseURL, bytes.NewBufferString(query))
	if err != nil {
		return nil, fmt.Errorf("failed to create request: %w", err)
	}

	// Set headers
	req.Header.Set("User-Agent", "cf-stats-exporter/1.0")
	req.Header.Set("Content-Type", "application/json")
	req.Header.Set("Authorization", fmt.Sprintf("Bearer %s", c.apiToken))

	// Execute request
	resp, err := c.httpClient.Do(req)
	if err != nil {
		return nil, fmt.Errorf("HTTP request failed: %w", err)
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return nil, fmt.Errorf("HTTP request failed with status %d", resp.StatusCode)
	}

	// Read response body
	body, err := io.ReadAll(resp.Body)
	if err != nil {
		return nil, fmt.Errorf("failed to read response body: %w", err)
	}

	// Parse JSON response
	var cfResp cfResponse
	if err := json.Unmarshal(body, &cfResp); err != nil {
		return nil, fmt.Errorf("failed to parse response: %w", err)
	}

	// Convert to StatusMetric array
	if len(cfResp.Data.Viewer.Zones) == 0 {
		return []CFStatusMetric{}, nil
	}

	rawGroups := cfResp.Data.Viewer.Zones[0].HTTPRequestsAdaptiveGroups
	metrics := make([]CFStatusMetric, 0, len(rawGroups))

	for _, g := range rawGroups {
		metrics = append(metrics, CFStatusMetric{
			Status: g.Dimensions.EdgeResponseStatus,
			Count:  g.Count,
		})
	}

	return metrics, nil
}
