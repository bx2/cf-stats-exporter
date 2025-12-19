use serde::{Deserialize, Serialize};
use std::error::Error;

#[derive(Debug, Serialize, Deserialize)]
pub struct CfStatusMetric {
    pub status: u16,
    pub count: u64,
}

#[derive(Debug, Deserialize)]
struct CfResponse {
    data: CfData,
    #[serde(default)]
    errors: Option<serde_json::Value>,
}

#[derive(Debug, Deserialize)]
struct CfData {
    viewer: CfViewer,
}

#[derive(Debug, Deserialize)]
struct CfViewer {
    zones: Vec<CfZone>,
}

#[derive(Debug, Deserialize)]
struct CfZone {
    #[serde(rename = "httpRequestsAdaptiveGroups")]
    http_requests_adaptive_groups: Vec<CfGroup>,
}

#[derive(Debug, Deserialize)]
struct CfGroup {
    count: u64,
    dimensions: CfDimensions,
}

#[derive(Debug, Deserialize)]
struct CfDimensions {
    #[serde(rename = "edgeResponseStatus")]
    edge_response_status: u16,
}

pub struct CfFetchOptions<'a> {
    pub zone_id: &'a str,
    pub start_date: &'a str,
    pub end_date: &'a str,
    pub limit: u32,
}

pub struct CfClient {
    api_token: String,
    client: reqwest::blocking::Client,
}

impl CfClient {
    const BASE_URL: &'static str = "https://api.cloudflare.com/client/v4/graphql";

    pub fn new(api_token: String) -> Self {
        CfClient {
            api_token,
            client: reqwest::blocking::Client::new(),
        }
    }

    pub fn fetch_metrics(&self, opts: CfFetchOptions) -> Result<Vec<CfStatusMetric>, Box<dyn Error>> {
        let query = format!(
            r#"{{"query": "{{ viewer {{ zones(filter: {{zoneTag: \"{}\"}}) {{ httpRequestsAdaptiveGroups(limit: {}, filter: {{datetime_geq: \"{}\", datetime_leq: \"{}\"}}, orderBy: [count_DESC]) {{ count dimensions {{ edgeResponseStatus }} }}}}}}}}"}}"#,
            opts.zone_id, opts.limit, opts.start_date, opts.end_date
        );

        let auth_header = format!("Bearer {}", self.api_token);

        let response = self.client
            .post(Self::BASE_URL)
            .header("User-Agent", "cf-stats-exporter/1.0")
            .header("Content-Type", "application/json")
            .header("Authorization", auth_header)
            .body(query)
            .send()?;

        if !response.status().is_success() {
            return Err(format!("HTTP request failed with status: {}", response.status()).into());
        }

        let cf_response: CfResponse = response.json()?;

        let metrics = cf_response
            .data
            .viewer
            .zones
            .first()
            .ok_or("No zones in response")?
            .http_requests_adaptive_groups
            .iter()
            .map(|g| CfStatusMetric {
                status: g.dimensions.edge_response_status,
                count: g.count,
            })
            .collect();

        Ok(metrics)
    }
}
