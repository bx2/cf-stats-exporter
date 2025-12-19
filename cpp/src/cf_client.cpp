#include "cf_client.hpp"
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <sstream>
#include <iostream>

using json = nlohmann::json;

namespace cf {

CFClient::CFClient(std::string api_token)
    : api_token_(std::move(api_token)) {}

std::string CFClient::buildGraphQLQuery(const CFFetchOptions& opts) {
    std::ostringstream oss;
    oss << R"({"query": "{ viewer { zones(filter: {zoneTag: \")"
        << opts.zone_id
        << R"(\"}) { httpRequestsAdaptiveGroups(limit: )"
        << opts.limit
        << R"(, filter: {datetime_geq: \")"
        << opts.start_date
        << R"(\", datetime_leq: \")"
        << opts.end_date
        << R"(\"}, orderBy: [count_DESC]) { count dimensions { edgeResponseStatus } }}}}"})";;
    return oss.str();
}

std::vector<CFStatusMetric> CFClient::fetchMetrics(const CFFetchOptions& opts) {
    std::string auth_header = "Bearer " + api_token_;
    std::string payload = buildGraphQLQuery(opts);

    cpr::Response response = cpr::Post(
        cpr::Url{base_url_},
        cpr::Header{
            {"User-Agent", "cf-stats-exporter/1.0"},
            {"Content-Type", "application/json"},
            {"Authorization", auth_header}
        },
        cpr::Body{payload}
    );

    if (response.status_code != 200) {
        std::ostringstream err;
        err << "HTTP request failed with status " << response.status_code;
        throw CFClientError(err.str());
    }

    // Parse JSON response
    json parsed;
    try {
        parsed = json::parse(response.text);
    } catch (const json::parse_error& e) {
        throw CFClientError(std::string("JSON parse error: ") + e.what());
    }

    // Extract metrics from nested structure
    std::vector<CFStatusMetric> metrics;

    try {
        const auto& viewer = parsed.at("data").at("viewer");
        const auto& zones = viewer.at("zones");

        if (zones.empty()) {
            return metrics;
        }

        const auto& groups = zones[0].at("httpRequestsAdaptiveGroups");

        for (const auto& group : groups) {
            CFStatusMetric metric;
            metric.count = group.at("count").get<uint64_t>();
            metric.status = group.at("dimensions").at("edgeResponseStatus").get<uint16_t>();
            metrics.push_back(metric);
        }
    } catch (const json::exception& e) {
        throw CFClientError(std::string("JSON structure error: ") + e.what());
    }

    return metrics;
}

} // namespace cf
