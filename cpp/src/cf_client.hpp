#pragma once

#include <string>
#include <vector>
#include <stdexcept>

namespace cf {

struct CFStatusMetric {
    uint16_t status;
    uint64_t count;
};

struct CFFetchOptions {
    std::string zone_id;
    std::string start_date;
    std::string end_date;
    uint32_t limit;
};

class CFClient {
public:
    explicit CFClient(std::string api_token);
    ~CFClient() = default;

    CFClient(const CFClient&) = delete;
    CFClient& operator=(const CFClient&) = delete;
    CFClient(CFClient&&) = default;
    CFClient& operator=(CFClient&&) = default;

    std::vector<CFStatusMetric> fetchMetrics(const CFFetchOptions& opts);

private:
    std::string api_token_;
    static constexpr const char* base_url_ = "https://api.cloudflare.com/client/v4/graphql";

    std::string buildGraphQLQuery(const CFFetchOptions& opts);
};

class CFClientError : public std::runtime_error {
public:
    explicit CFClientError(const std::string& message)
        : std::runtime_error(message) {}
};

} // namespace cf
