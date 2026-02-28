#include "MacroAPI.hpp"
#include "SPManager.hpp"
#include "RestServer.hpp"
#include <cstring>
#include <string>
#include <vector>
#include <inttypes.h>
#include <dirent.h>
#include <sys/stat.h>
#include <cstdio>
#include "esp_vfs_fat.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"

using namespace CTAG::REST;
using namespace rapidjson;


/** Send a JSON string as HTTP response */
static esp_err_t send_json(httpd_req_t *req, const char *json) {
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_sendstr(req, json);
    return ESP_OK;
}

/** Send a simple JSON ok response */
static esp_err_t send_ok(httpd_req_t *req) {
    return send_json(req, "{\"ok\":true}");
}



esp_err_t MacroAPI::macroapi_get_handler(httpd_req_t *req) {
    ESP_LOGI("get_trackstatus_handler", "1: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
             heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
    // RestServer::set_cors_headers(req);

    char query[128];
    httpd_req_get_url_query_str(req, query, 128);
    httpd_resp_set_type(req, "application/json");

    std::string outputjson;
    CTAG::AUDIO::SoundProcessorManager::macroTranslator->SerializeStateJSON(&outputjson);
    return send_json(req, outputjson.c_str());
}

static esp_err_t handle_reload(httpd_req_t *req) {
    CTAG::AUDIO::SoundProcessorManager::DisablePluginProcessing();
    CTAG::AUDIO::SoundProcessorManager::RefreshMacros();
    CTAG::AUDIO::SoundProcessorManager::EnablePluginProcessing();

    return send_ok(req);
}

static esp_err_t handle_set_track_macro(httpd_req_t *req) {
    char *content = (char *) heap_caps_malloc(req->content_len + 1, MALLOC_CAP_SPIRAM);
    int ret = httpd_req_recv(req, content, req->content_len);
    if (ret <= 0) {  /* 0 return value indicates connection closed */
        /* Check if timeout occurred */
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            /* In case of timeout one can choose to retry calling
             * httpd_req_recv(), but to keep it simple, here we
             * respond with an HTTP 408 (Request Timeout) error */
            httpd_resp_send_408(req);
        }
        /* In case of error, returning ESP_FAIL will
         * ensure that the underlying socket is closed */
        return ESP_FAIL;
    }
    content[req->content_len] = 0;

    ESP_LOGI("set_track_macro_handler", "Received set track macro command, data: %s", content);
    CTAG::AUDIO::SoundProcessorManager::SetTrackParametersFromJSON(content);
    free(content);

    return send_ok(req);
}

esp_err_t MacroAPI::macroapi_post_handler(httpd_req_t *req) {
    ESP_LOGI("put_trackinfo_handler", "1: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
             heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
             heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

    // RestServer::set_cors_headers(req);

    /* Destination buffer for content of HTTP POST request.
     * httpd_req_recv() accepts char* only, but content could
     * as well be any binary data (needs type casting).
     * In case of string data, null termination will be absent, and
     * content length would give length of string */

    size_t qlen = httpd_req_get_url_query_len(req);
    char action[32] = {0};

    if (qlen > 0) {
        char *query = (char *)malloc(qlen + 1);
        httpd_req_get_url_query_str(req, query, qlen + 1);
        httpd_query_key_value(query, "action", action, sizeof(action));
        free(query);
    }

    if (strcmp(action, "reload") == 0) {
        return handle_reload(req);
    }
    else if (strcmp(action, "set_track_parameters") == 0) {
        return handle_set_track_macro(req);
    }
    else if (strcmp(action, "update_track") == 0) {
        return handle_set_track_macro(req);
    }

    // int trackindex = -1;
    // char query[128];
    // char pluginID[64];
    // httpd_req_get_url_query_str(req, query, 128);
    // httpd_resp_set_type(req, "application/json");
    // char *pLastSlash = strrchr(req->uri, '/');
    // if (pLastSlash) {
    //     strcpy(pluginID, pLastSlash + 1);
    //     ESP_LOGD(REST_TAG, "Sending sound preset for id %s", pluginID);
    //     trackindex = atoi(pluginID);
    // }

    // char *content = (char *) heap_caps_malloc(req->content_len + 1, MALLOC_CAP_SPIRAM);
    // int ret = httpd_req_recv(req, content, req->content_len);
    // if (ret <= 0) {  /* 0 return value indicates connection closed */
    //     /* Check if timeout occurred */
    //     if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
    //         /* In case of timeout one can choose to retry calling
    //          * httpd_req_recv(), but to keep it simple, here we
    //          * respond with an HTTP 408 (Request Timeout) error */
    //         httpd_resp_send_408(req);
    //     }
    //     /* In case of error, returning ESP_FAIL will
    //      * ensure that the underlying socket is closed */
    //     return ESP_FAIL;
    // }
    // content[req->content_len] = 0;

    // ESP_LOGI("put_trackinfo_handler", "Received track %d command, data: %s", trackindex, content);
    // CTAG::AUDIO::SoundProcessorManager::SetTrackParametersFromJSON(trackindex, content);
    // free(content);

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}


// static esp_err_t MacroAPI::post_reloadmacros_handler(httpd_req_t *req) {
//      ESP_LOGI("put_trackinfo_handler", "1: Mem freesize internal %d, largest block %d, free SPIRAM %d, largest block SPIRAM %d!",
//              heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
//              heap_caps_get_largest_free_block(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
//              heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
//              heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

//     RestServer::set_cors_headers(req);
//     /* Destination buffer for content of HTTP POST request.
//      * httpd_req_recv() accepts char* only, but content could
//      * as well be any binary data (needs type casting).
//      * In case of string data, null termination will be absent, and
//      * content length would give length of string */

//     // int trackindex = -1;
//     // char query[128];
//     // char pluginID[64];
//     // httpd_req_get_url_query_str(req, query, 128);
//     // httpd_resp_set_type(req, "application/json");
//     // char *pLastSlash = strrchr(req->uri, '/');
//     // if (pLastSlash) {
//     //     strcpy(pluginID, pLastSlash + 1);
//     //     ESP_LOGD(REST_TAG, "Sending sound preset for id %s", pluginID);
//     //     trackindex = atoi(pluginID);
//     // }

//     // char *content = (char *) heap_caps_malloc(req->content_len + 1, MALLOC_CAP_SPIRAM);
//     // int ret = httpd_req_recv(req, content, req->content_len);
//     // if (ret <= 0) {  /* 0 return value indicates connection closed */
//     //     /* Check if timeout occurred */
//     //     if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
//     //         /* In case of timeout one can choose to retry calling
//     //          * httpd_req_recv(), but to keep it simple, here we
//     //          * respond with an HTTP 408 (Request Timeout) error */
//     //         httpd_resp_send_408(req);
//     //     }
//     //     /* In case of error, returning ESP_FAIL will
//     //      * ensure that the underlying socket is closed */
//     //     return ESP_FAIL;
//     // }
//     // content[req->content_len] = 0;

//     // ESP_LOGI("put_trackinfo_handler", "Received track %d command, data: %s", trackindex, content);
//     // CTAG::AUDIO::SoundProcessorManager::SetTrackParametersFromJSON(trackindex, content);
//     // free(content);

//     httpd_resp_set_type(req, "text/html");
//     httpd_resp_send(req, NULL, 0);
//     return ESP_OK;
// };


