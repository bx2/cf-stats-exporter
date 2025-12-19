package main

import (
	"encoding/json"
	"fmt"
	"log"
	"os"

	"cf-stats-exporter/cfclient"
)

func main() {
	apiToken := os.Getenv("CF_API_TOKEN")
	zoneID := os.Getenv("CF_ZONE_ID")

	if apiToken == "" {
		log.Fatal("CF_API_TOKEN environment variable is required")
	}
	if zoneID == "" {
		log.Fatal("CF_ZONE_ID environment variable is required")
	}

	client := cfclient.NewCFClient(apiToken)

	metrics, err := client.FetchMetrics(cfclient.CFFetchOptions{
		ZoneID:    zoneID,
		StartDate: "2025-12-17T15:00:00Z",
		EndDate:   "2025-12-18T15:00:00Z",
		Limit:     10,
	})
	if err != nil {
		log.Fatalf("Failed to fetch metrics: %v", err)
	}

	// Output JSON array to stdout
	encoder := json.NewEncoder(os.Stdout)
	encoder.SetIndent("", "  ")
	if err := encoder.Encode(metrics); err != nil {
		log.Fatalf("Failed to encode JSON: %v", err)
	}

	fmt.Println()
}
