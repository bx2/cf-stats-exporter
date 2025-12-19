#ifndef CF_CLIENT_H
#define CF_CLIENT_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Error codes */
typedef enum {
    CF_OK = 0,
    CF_ERROR_MEMORY = -1,
    CF_ERROR_HTTP = -2,
    CF_ERROR_JSON = -3,
    CF_ERROR_BUFFER_TOO_SMALL = -4,
    CF_ERROR_INVALID_RESPONSE = -5,
} cf_error_t;

/* Status metric structure */
typedef struct {
    uint16_t status;
    uint64_t count;
} cf_status_metric_t;

/* Fetch options */
typedef struct {
    const char *zone_id;
    const char *start_date;
    const char *end_date;
    uint32_t limit;
} cf_fetch_options_t;

/* Opaque client handle */
typedef struct cf_client cf_client_t;

/* Create a new Cloudflare API client */
cf_client_t *cf_client_new(const char *api_token);

/* Fetch metrics from Cloudflare API
 * Returns number of metrics on success, negative error code on failure
 * Caller must free metrics array with free() when done
 */
int cf_client_fetch_metrics(cf_client_t *client,
                            const cf_fetch_options_t *opts,
                            cf_status_metric_t **metrics,
                            size_t *metrics_count);

/* Free the client and associated resources */
void cf_client_free(cf_client_t *client);

#ifdef __cplusplus
}
#endif

#endif /* CF_CLIENT_H */
