#include "cf_client.h"
#include "cJSON.h"
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BASE_URL "https://api.cloudflare.com/client/v4/graphql"

struct cf_client {
    char *api_token;
    CURL *curl;
};

/* Memory buffer for curl responses */
typedef struct {
    char *data;
    size_t size;
    size_t capacity;
} memory_buffer_t;

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    memory_buffer_t *mem = (memory_buffer_t *)userp;

    /* Grow buffer if needed */
    while (mem->size + realsize + 1 > mem->capacity) {
        size_t new_capacity = mem->capacity == 0 ? 4096 : mem->capacity * 2;
        char *new_data = realloc(mem->data, new_capacity);
        if (!new_data) {
            return 0; /* Out of memory */
        }
        mem->data = new_data;
        mem->capacity = new_capacity;
    }

    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;

    return realsize;
}

cf_client_t *cf_client_new(const char *api_token) {
    if (!api_token) {
        return NULL;
    }

    cf_client_t *client = malloc(sizeof(cf_client_t));
    if (!client) {
        return NULL;
    }

    client->api_token = strdup(api_token);
    if (!client->api_token) {
        free(client);
        return NULL;
    }

    client->curl = curl_easy_init();
    if (!client->curl) {
        free(client->api_token);
        free(client);
        return NULL;
    }

    return client;
}

void cf_client_free(cf_client_t *client) {
    if (!client) {
        return;
    }

    if (client->curl) {
        curl_easy_cleanup(client->curl);
    }

    free(client->api_token);
    free(client);
}

int cf_client_fetch_metrics(cf_client_t *client,
                           const cf_fetch_options_t *opts,
                           cf_status_metric_t **metrics,
                           size_t *metrics_count) {
    if (!client || !opts || !metrics || !metrics_count) {
        return CF_ERROR_MEMORY;
    }

    *metrics = NULL;
    *metrics_count = 0;

    /* Build authorization header */
    char auth_header[512];
    int auth_len = snprintf(auth_header, sizeof(auth_header),
                           "Authorization: Bearer %s", client->api_token);
    if (auth_len < 0 || (size_t)auth_len >= sizeof(auth_header)) {
        return CF_ERROR_BUFFER_TOO_SMALL;
    }

    /* Build GraphQL query payload */
    char payload[2048];
    int payload_len = snprintf(payload, sizeof(payload),
        "{\"query\": \"{ viewer { zones(filter: {zoneTag: \\\"%s\\\"}) { httpRequestsAdaptiveGroups(limit: %u, filter: {datetime_geq: \\\"%s\\\", datetime_leq: \\\"%s\\\"}, orderBy: [count_DESC]) { count dimensions { edgeResponseStatus } }}}}\"}",
        opts->zone_id, opts->limit, opts->start_date, opts->end_date);

    if (payload_len < 0 || (size_t)payload_len >= sizeof(payload)) {
        return CF_ERROR_BUFFER_TOO_SMALL;
    }

    /* Setup curl headers */
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "User-Agent: cf-stats-exporter/1.0");
    headers = curl_slist_append(headers, auth_header);

    /* Setup response buffer */
    memory_buffer_t response = {0};

    /* Configure curl */
    curl_easy_setopt(client->curl, CURLOPT_URL, BASE_URL);
    curl_easy_setopt(client->curl, CURLOPT_POST, 1L);
    curl_easy_setopt(client->curl, CURLOPT_POSTFIELDS, payload);
    curl_easy_setopt(client->curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(client->curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(client->curl, CURLOPT_WRITEDATA, &response);

    /* Perform request */
    CURLcode res = curl_easy_perform(client->curl);
    curl_slist_free_all(headers);

    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        free(response.data);
        return CF_ERROR_HTTP;
    }

    /* Check HTTP status code */
    long http_code = 0;
    curl_easy_getinfo(client->curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code != 200) {
        fprintf(stderr, "HTTP request failed with status %ld\n", http_code);
        fprintf(stderr, "Response: %s\n", response.data ? response.data : "(null)");
        free(response.data);
        return CF_ERROR_HTTP;
    }

    if (!response.data || response.size == 0) {
        return CF_ERROR_INVALID_RESPONSE;
    }

    /* Parse JSON response */
    cJSON *root = cJSON_Parse(response.data);
    free(response.data);

    if (!root) {
        fprintf(stderr, "Failed to parse JSON response\n");
        return CF_ERROR_JSON;
    }

    /* Navigate: data.viewer.zones[0].httpRequestsAdaptiveGroups */
    cJSON *data = cJSON_GetObjectItem(root, "data");
    if (!data) {
        cJSON_Delete(root);
        return CF_ERROR_INVALID_RESPONSE;
    }

    cJSON *viewer = cJSON_GetObjectItem(data, "viewer");
    if (!viewer) {
        cJSON_Delete(root);
        return CF_ERROR_INVALID_RESPONSE;
    }

    cJSON *zones = cJSON_GetObjectItem(viewer, "zones");
    if (!zones || !cJSON_IsArray(zones) || cJSON_GetArraySize(zones) == 0) {
        cJSON_Delete(root);
        return CF_ERROR_INVALID_RESPONSE;
    }

    cJSON *zone = cJSON_GetArrayItem(zones, 0);
    if (!zone) {
        cJSON_Delete(root);
        return CF_ERROR_INVALID_RESPONSE;
    }

    cJSON *groups = cJSON_GetObjectItem(zone, "httpRequestsAdaptiveGroups");
    if (!groups || !cJSON_IsArray(groups)) {
        cJSON_Delete(root);
        return CF_ERROR_INVALID_RESPONSE;
    }

    /* Allocate metrics array */
    int count = cJSON_GetArraySize(groups);
    if (count == 0) {
        cJSON_Delete(root);
        *metrics_count = 0;
        return CF_OK;
    }

    cf_status_metric_t *result = malloc(count * sizeof(cf_status_metric_t));
    if (!result) {
        cJSON_Delete(root);
        return CF_ERROR_MEMORY;
    }

    /* Extract metrics */
    int idx = 0;
    cJSON *group = NULL;
    cJSON_ArrayForEach(group, groups) {
        cJSON *count_obj = cJSON_GetObjectItem(group, "count");
        cJSON *dimensions = cJSON_GetObjectItem(group, "dimensions");

        if (!count_obj || !dimensions) {
            continue;
        }

        cJSON *status_obj = cJSON_GetObjectItem(dimensions, "edgeResponseStatus");
        if (!status_obj) {
            continue;
        }

        result[idx].status = (uint16_t)cJSON_GetNumberValue(status_obj);
        result[idx].count = (uint64_t)cJSON_GetNumberValue(count_obj);
        idx++;
    }

    cJSON_Delete(root);

    *metrics = result;
    *metrics_count = idx;

    return CF_OK;
}
