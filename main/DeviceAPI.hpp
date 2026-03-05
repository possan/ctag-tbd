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

#pragma once

#include "esp_http_server.h"
#include "esp_err.h"

namespace CTAG {
    namespace REST {
        /**
         * Device API v2 — configuration, IO capabilities, favorites, system.
         *
         * Replaces 6 individual v1 handlers with 2 (GET + POST).
         *
         * GET  /api/v2/device?action=...
         *   getConfig      — device configuration JSON
         *   getIOCaps      — IO capabilities (triggers, CVs, versions)
         *   getFavorites   — all stored favorites
         *   getAll         — bulk: config + ioCaps + favorites
         *
         * POST /api/v2/device?action=...
         *   setConfig          — body: JSON configuration
         *   reboot             — restart the device
         *   storeFavorite&id=N — body: JSON favorite data
         *   recallFavorite&id=N — activate favorite N
         */
        class DeviceAPI final {
        public:
            DeviceAPI() = delete;
            static esp_err_t device_get_handler(httpd_req_t *req);
            static esp_err_t device_post_handler(httpd_req_t *req);
        };
    }
}
