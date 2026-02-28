#pragma once

#include "esp_http_server.h"
#include "esp_err.h"

namespace CTAG {
    namespace REST {
        class MacroAPI final {
        public:
            MacroAPI() = delete;

            static esp_err_t macroapi_get_handler(httpd_req_t *req);
            static esp_err_t macroapi_post_handler(httpd_req_t *req);
        };
    }
}
