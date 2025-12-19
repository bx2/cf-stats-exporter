#include "cf_client.h"
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    const char *api_token = getenv("CF_API_TOKEN");
    const char *zone_id = getenv("CF_ZONE_ID");

    if (!api_token) {
        fprintf(stderr, "CF_API_TOKEN environment variable is required\n");
        return 1;
    }
    if (!zone_id) {
        fprintf(stderr, "CF_ZONE_ID environment variable is required\n");
        return 1;
    }

    cf_client_t *client = cf_client_new(api_token);
    if (!client) {
        fprintf(stderr, "Failed to create CF client\n");
        return 1;
    }

    cf_fetch_options_t opts = {
        .zone_id = zone_id,
        .start_date = "2025-12-17T15:00:00Z",
        .end_date = "2025-12-18T15:00:00Z",
        .limit = 10,
    };

    cf_status_metric_t *metrics = NULL;
    size_t metrics_count = 0;

    int result = cf_client_fetch_metrics(client, &opts, &metrics, &metrics_count);
    if (result != CF_OK) {
        fprintf(stderr, "Failed to fetch metrics: error code %d\n", result);
        cf_client_free(client);
        return 1;
    }

    cJSON *json_array = cJSON_CreateArray();
    if (!json_array) {
        fprintf(stderr, "Failed to create JSON array\n");
        free(metrics);
        cf_client_free(client);
        return 1;
    }

    for (size_t i = 0; i < metrics_count; i++) {
        cJSON *obj = cJSON_CreateObject();
        if (!obj) {
            fprintf(stderr, "Failed to create JSON object\n");
            cJSON_Delete(json_array);
            free(metrics);
            cf_client_free(client);
            return 1;
        }

        cJSON_AddNumberToObject(obj, "status", metrics[i].status);
        cJSON_AddNumberToObject(obj, "count", metrics[i].count);
        cJSON_AddItemToArray(json_array, obj);
    }

    char *json_string = cJSON_Print(json_array);
    if (json_string) {
        printf("%s\n", json_string);
        free(json_string);
    }

    cJSON_Delete(json_array);
    free(metrics);
    cf_client_free(client);

    return 0;
}
