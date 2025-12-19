use cf_stats_exporter::{CfClient, CfFetchOptions};
use std::{env, process};

fn main() {
    let api_token = env::var("CF_API_TOKEN").unwrap_or_else(|_| {
        eprintln!("CF_API_TOKEN environment variable is required");
        process::exit(1);
    });

    let zone_id = env::var("CF_ZONE_ID").unwrap_or_else(|_| {
        eprintln!("CF_ZONE_ID environment variable is required");
        process::exit(1);
    });

    let client = CfClient::new(api_token);

    let metrics = client.fetch_metrics(CfFetchOptions {
        zone_id: &zone_id,
        start_date: "2025-12-17T15:00:00Z",
        end_date: "2025-12-18T15:00:00Z",
        limit: 10,
    });

    match metrics {
        Ok(metrics) => {
            match serde_json::to_string(&metrics) {
                Ok(json) => println!("{}", json),
                Err(e) => {
                    eprintln!("Failed to serialize metrics: {}", e);
                    process::exit(1);
                }
            }
        }
        Err(e) => {
            eprintln!("Failed to fetch metrics: {}", e);
            process::exit(1);
        }
    }
}
