#include "cf_client.hpp"
#include <nlohmann/json.hpp>
#include <iostream>
#include <cstdlib>

using json = nlohmann::json;

int main() {
    const char* api_token = std::getenv("CF_API_TOKEN");
    const char* zone_id = std::getenv("CF_ZONE_ID");

    if (!api_token) {
        std::cerr << "CF_API_TOKEN environment variable is required" << std::endl;
        return 1;
    }
    if (!zone_id) {
        std::cerr << "CF_ZONE_ID environment variable is required" << std::endl;
        return 1;
    }

    try {
        cf::CFClient client(api_token);

        auto metrics = client.fetchMetrics({
            .zone_id = zone_id,
            .start_date = "2025-12-17T15:00:00Z",
            .end_date = "2025-12-18T15:00:00Z",
            .limit = 10
        });

        json output = json::array();
        for (const auto& metric : metrics) {
            output.push_back({
                {"status", metric.status},
                {"count", metric.count}
            });
        }

        std::cout << output.dump() << std::endl;

        return 0;
    } catch (const cf::CFClientError& e) {
        std::cerr << "CF Client Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
