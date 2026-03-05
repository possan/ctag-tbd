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
         * Plugin API v2 — consolidated GET/POST with ?action= dispatch.
         *
         * Replaces 11 individual v1 handlers with 2 (GET + POST).
         *
         * GET  /api/v2/plugins?action=...
         *   list              — all available sound processors
         *   getActive&ch=N    — active plugin ID for channel N (0 or 1)
         *   getParams&ch=N    — parameter specs for active plugin on channel N
         *   getPresets&ch=N   — preset names for channel N
         *   getPresetData&id=X — all preset data for plugin X
         *   getAll            — bulk: plugins + active + params + presets (both channels)
         *
         * POST /api/v2/plugins?action=...
         *   setActive&ch=N&id=X            — activate plugin X on channel N
         *   setParam&ch=N&id=X&key=K&val=V — set parameter value
         *   savePreset&ch=N&name=X&number=Y — save current state to preset slot
         *   loadPreset&ch=N&number=Y        — load preset slot
         *   setPresetData&id=X              — body: JSON preset data to store
         */
        class PluginAPI final {
        public:
            PluginAPI() = delete;
            static esp_err_t plugins_get_handler(httpd_req_t *req);
            static esp_err_t plugins_post_handler(httpd_req_t *req);
        };
    }
}
