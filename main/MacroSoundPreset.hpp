#pragma once

#include <vector>
#include <set>
#include <string>
#include <stdint.h>
#include <iostream>
#include <memory>
#include "rapidjson/document.h"

namespace CTAG {
    namespace MACROPRESETS {
        class MacroSoundPreset {
            public:
                std::string id;
                std::string displayName;
                std::string groupName;
                std::string macroDeviceId;
                std::set<uint8_t> validTracks;
                std::vector<int32_t> parameterValues;
                bool DeserializeJSON(const rapidjson::Value &jsonelement);
                bool SerializeJSONInto(rapidjson::Document &doc);
                MacroSoundPreset();
                ~MacroSoundPreset();
        };
    }
}