#include "MacroAPI.hpp"
#include "SPManager.hpp"
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

static const char *MACRO_TAG = "MacroAPI";
static const char *MACRODEFS_DIR  = "/sdcard/data/macrodefinitions";
static const char *PRESETS_DIR    = "/sdcard/data/macrosoundpresets";

static void set_api_headers(httpd_req_t *req) {
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
}

/** Send a JSON string as HTTP response */
static esp_err_t send_json(httpd_req_t *req, const char *json) {
    set_api_headers(req);
    httpd_resp_set_type(req, "application/json");
    if (json) httpd_resp_sendstr(req, json);
    else httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

/** Send a simple JSON ok response */
static esp_err_t send_ok(httpd_req_t *req) {
    return send_json(req, "{\"ok\":true}");
}

/**
 * Read all .json files in a directory and return them as a rapidjson Array.
 * Each element is the parsed JSON object from that file.
 * Uses SPIRAM for the read buffer to keep internal heap free.
 */
static void read_all_json_in_dir(const char *dirPath, Value &outArray,
                                  Document::AllocatorType &alloc) {
    DIR *dir = opendir(dirPath);
    if (!dir) {
        ESP_LOGW(MACRO_TAG, "Cannot open dir %s", dirPath);
        return;
    }

    char pathBuf[256];
    char *fileBuf = (char *)heap_caps_malloc(8192, MALLOC_CAP_SPIRAM);
    if (!fileBuf) {
        closedir(dir);
        ESP_LOGE(MACRO_TAG, "SPIRAM alloc failed for file buffer");
        return;
    }

    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        // Only .json files
        size_t nlen = strlen(ent->d_name);
        if (nlen < 6 || strcasecmp(ent->d_name + nlen - 5, ".json") != 0)
            continue;

        snprintf(pathBuf, sizeof(pathBuf), "%s/%s", dirPath, ent->d_name);
        FILE *fp = fopen(pathBuf, "r");
        if (!fp) continue;

        FileReadStream is(fp, fileBuf, 8192);
        Document doc;
        doc.ParseStream(is);
        fclose(fp);

        if (doc.HasParseError() || !doc.IsObject()) {
            ESP_LOGW(MACRO_TAG, "Skip bad JSON: %s", ent->d_name);
            continue;
        }

        Value copy(doc, alloc);
        outArray.PushBack(copy, alloc);
    }

    heap_caps_free(fileBuf);
    closedir(dir);
}


/**
 * GET /api/v2/macros — dispatched by ?action= query parameter.
 *
 *   (no action / default) → track status (original behaviour)
 *   ?action=getall        → bulk: { macroDefs:[], soundPresets:[], tracks:[] }
 *
 * The "getall" action replaces dozens of individual config-file fetches with
 * a single HTTP round-trip, keeping total API calls within the ESP32 limit.
 */
esp_err_t MacroAPI::macroapi_get_handler(httpd_req_t *req) {
    ESP_LOGI(MACRO_TAG, "GET Mem free int %d, SPIRAM %d",
             heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    char query[128] = {0};
    char action[32] = {0};
    httpd_req_get_url_query_str(req, query, sizeof(query));
    httpd_query_key_value(query, "action", action, sizeof(action));

    /* ── action=getall ── bulk fetch all macro data in one response ── */
    if (strcmp(action, "getall") == 0) {
        Document resp(kObjectType);
        auto &alloc = resp.GetAllocator();

        // 1) All macro definitions
        Value defs(kArrayType);
        read_all_json_in_dir(MACRODEFS_DIR, defs, alloc);
        resp.AddMember("macroDefs", defs, alloc);

        // 2) All sound presets
        Value presets(kArrayType);
        read_all_json_in_dir(PRESETS_DIR, presets, alloc);
        resp.AddMember("soundPresets", presets, alloc);

        // 3) Current track state
        Document trackDoc(kObjectType);
        CTAG::AUDIO::SoundProcessorManager::macroTranslator->SerializeStateInto(trackDoc);
        if (trackDoc.HasMember("tracks"))
            resp.AddMember("tracks", trackDoc["tracks"], alloc);

        StringBuffer sb;
        Writer<StringBuffer> writer(sb);
        resp.Accept(writer);
        return send_json(req, sb.GetString());
    }

    /* ── default: return current track state ── */
    httpd_resp_set_type(req, "application/json");
    Document resp(kObjectType);
    auto &alloc = resp.GetAllocator();

    Document doc2(kObjectType);
    CTAG::AUDIO::SoundProcessorManager::macroTranslator->SerializeStateInto(doc2);
    resp.AddMember("tracks", doc2["tracks"], alloc);

    StringBuffer sb;
    Writer<StringBuffer> writer(sb);
    resp.Accept(writer);
    return send_json(req, sb.GetString());
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
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }
    content[req->content_len] = 0;

    ESP_LOGI(MACRO_TAG, "set_track_macro data: %s", content);
    CTAG::AUDIO::SoundProcessorManager::SetTrackParametersFromJSON(content);
    free(content);

    return send_ok(req);
}

esp_err_t MacroAPI::macroapi_post_handler(httpd_req_t *req) {
    ESP_LOGI(MACRO_TAG, "POST Mem free int %d, SPIRAM %d",
             heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

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

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}
