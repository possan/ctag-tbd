/***************
CTAG TBD >>to be determined<< is an open source eurorack synthesizer module.

A project conceived within the Creative Technologies Arbeitsgruppe of
Kiel University of Applied Sciences: https://www.creative-technologies.de

(c) 2020 by Robert Manzke. All rights reserved.

The CTAG TBD software is licensed under the GNU General Public License
(GPL 3.0), available here: https://www.gnu.org/licenses/gpl-3.0.txt

The CTAG TBD hardware design is released under the Creative Commons
Attribution-NonCommercial-ShareAlike 4.0 International (CC BY-NC-SA 4.0).
Details here: https://creativecommons.org/licenses/by-nc-sa/4.0/

CTAG TBD is provided "as is" without any express or implied warranties.

License and copyright details for specific submodules are included in their
respective component folders / files if different from this license.
***************/

#include "DeviceAPI.hpp"
#include "SPManager.hpp"
#include "Favorites.hpp"
#include <cstring>
#include <string>
#include "esp_log.h"
#include "esp_heap_caps.h"

using namespace CTAG::REST;
using namespace std;

static const char *TAG = "DeviceAPI";

/*
 * IOCapabilities.hpp declares `string const s(...)` at whatever scope
 * it's included in.  Include once at file scope so both handle_get_iocaps
 * and handle_get_all can reference it.
 */
#include "IOCapabilities.hpp"

/* ── Helpers ──────────────────────────────────────────────────────── */

static void set_api_headers(httpd_req_t *req) {
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
}

static esp_err_t send_json(httpd_req_t *req, const char *json) {
    set_api_headers(req);
    httpd_resp_set_type(req, "application/json");
    if (json) httpd_resp_sendstr(req, json);
    else httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

static esp_err_t send_ok(httpd_req_t *req) {
    return send_json(req, "{\"ok\":true}");
}

/* ── GET action handlers ──────────────────────────────────────────── */

/** action=getConfig — device configuration */
static esp_err_t handle_get_config(httpd_req_t *req) {
    return send_json(req,
        CTAG::AUDIO::SoundProcessorManager::GetCStrJSONConfiguration());
}

/** action=getIOCaps — IO capabilities (triggers, CVs, versions) */
static esp_err_t handle_get_iocaps(httpd_req_t *req) {
    set_api_headers(req);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, s.c_str());
    return ESP_OK;
}

/** action=getFavorites — all stored favorites */
static esp_err_t handle_get_favorites(httpd_req_t *req) {
    string favs = CTAG::FAV::Favorites::GetAllFavorites();
    return send_json(req, favs.c_str());
}

/**
 * action=getAll — bulk response: config + ioCaps + favorites.
 * Uses chunked transfer to avoid a huge allocation.
 *
 * Response shape:
 * {
 *   "config":    {...},
 *   "ioCaps":    {...},
 *   "favorites": [...]
 * }
 */
static esp_err_t handle_get_all(httpd_req_t *req) {
    set_api_headers(req);
    httpd_resp_set_type(req, "application/json");

    #define CHUNK(s) httpd_resp_send_chunk(req, (s), strlen(s))

    CHUNK("{\"config\":");
    const char *config =
        CTAG::AUDIO::SoundProcessorManager::GetCStrJSONConfiguration();
    CHUNK(config ? config : "{}");

    CHUNK(",\"ioCaps\":");
    CHUNK(s.c_str());

    CHUNK(",\"favorites\":");
    string favs = CTAG::FAV::Favorites::GetAllFavorites();
    CHUNK(favs.c_str());

    CHUNK("}");
    httpd_resp_send_chunk(req, NULL, 0);  // terminate

    #undef CHUNK
    return ESP_OK;
}

/* ── POST action handlers ─────────────────────────────────────────── */

/** action=setConfig  (body = JSON configuration) */
static esp_err_t handle_set_config(httpd_req_t *req) {
    char *content = (char *)heap_caps_malloc(
        req->content_len + 1, MALLOC_CAP_SPIRAM);
    if (!content) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "alloc");
        return ESP_FAIL;
    }
    int ret = httpd_req_recv(req, content, req->content_len);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) httpd_resp_send_408(req);
        heap_caps_free(content);
        return ESP_FAIL;
    }
    content[req->content_len] = 0;
    CTAG::AUDIO::SoundProcessorManager::SetConfigurationFromJSON(string(content));
    heap_caps_free(content);
    return send_ok(req);
}

/** action=reboot — restart the device */
static esp_err_t handle_reboot(httpd_req_t *req) {
    ESP_LOGW(TAG, "Reboot requested");
    send_ok(req);
    esp_restart();
    return ESP_OK;  // unreachable
}

/** action=storeFavorite&id=N  (body = JSON favorite data) */
static esp_err_t handle_store_favorite(httpd_req_t *req, const char *query) {
    char idStr[4] = {0};
    httpd_query_key_value(query, "id", idStr, sizeof(idStr));
    int id = atoi(idStr);

    char *content = (char *)heap_caps_malloc(
        req->content_len + 1, MALLOC_CAP_SPIRAM);
    if (!content) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "alloc");
        return ESP_FAIL;
    }
    int ret = httpd_req_recv(req, content, req->content_len);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) httpd_resp_send_408(req);
        heap_caps_free(content);
        return ESP_FAIL;
    }
    content[req->content_len] = 0;
    ESP_LOGI(TAG, "storeFavorite id=%d", id);
    CTAG::FAV::Favorites::StoreFavorite(id, string(content));
    heap_caps_free(content);
    return send_ok(req);
}

/** action=recallFavorite&id=N */
static esp_err_t handle_recall_favorite(httpd_req_t *req, const char *query) {
    char idStr[4] = {0};
    httpd_query_key_value(query, "id", idStr, sizeof(idStr));
    int id = atoi(idStr);
    ESP_LOGI(TAG, "recallFavorite id=%d", id);
    CTAG::FAV::Favorites::ActivateFavorite(id);
    return send_ok(req);
}

/* ══════════════════════════════════════════════════════════════════════
 *  Main dispatch entry points
 * ══════════════════════════════════════════════════════════════════════ */

esp_err_t DeviceAPI::device_get_handler(httpd_req_t *req) {
    ESP_LOGD(TAG, "GET %s", req->uri);

    char query[128] = {0};
    char action[32] = {0};
    httpd_req_get_url_query_str(req, query, sizeof(query));
    httpd_query_key_value(query, "action", action, sizeof(action));

    if (strcmp(action, "getConfig") == 0)    return handle_get_config(req);
    if (strcmp(action, "getIOCaps") == 0)    return handle_get_iocaps(req);
    if (strcmp(action, "getFavorites") == 0) return handle_get_favorites(req);
    if (strcmp(action, "getAll") == 0)       return handle_get_all(req);

    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "unknown action");
    return ESP_FAIL;
}

esp_err_t DeviceAPI::device_post_handler(httpd_req_t *req) {
    ESP_LOGD(TAG, "POST %s", req->uri);

    char query[128] = {0};
    char action[32] = {0};
    httpd_req_get_url_query_str(req, query, sizeof(query));
    httpd_query_key_value(query, "action", action, sizeof(action));

    if (strcmp(action, "setConfig") == 0)       return handle_set_config(req);
    if (strcmp(action, "reboot") == 0)          return handle_reboot(req);
    if (strcmp(action, "storeFavorite") == 0)   return handle_store_favorite(req, query);
    if (strcmp(action, "recallFavorite") == 0)  return handle_recall_favorite(req, query);

    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "unknown action");
    return ESP_FAIL;
}
