#pragma once

#include <string>
#include <vector>
#include "rapidjson/document.h"

namespace CTAG {
    namespace MACROPRESETS {

        // Response curve types for parameter mapping
        enum class MacroCurveType : uint8_t {
            Linear = 0,   // Default: straight 1:1 mapping
            Log    = 1,   // Logarithmic: slow start, fast end (freq/cutoff)
            Exp    = 2,   // Exponential: fast start, slow end (envelope times)
            SCurve = 3    // S-curve: gentle at extremes, steep in middle
        };

        class MacroDeviceOutputMappingSource {
            public:
                uint8_t parameterIndex;
                int32_t multiplier;
                int32_t divider;
                MacroCurveType curve;

            public:
                MacroDeviceOutputMappingSource();
                ~MacroDeviceOutputMappingSource();
                bool DeserializeJSON(const rapidjson::Value &jsonelement);
                // bool SerializeJSONInto(rapidjson::Document &doc);
        };

        class MacroDeviceOutputMapping {
            public:
                int ctrl;
                int startValue;
                std::vector<MacroDeviceOutputMappingSource> sources;

            public:
                MacroDeviceOutputMapping();
                ~MacroDeviceOutputMapping();
                bool DeserializeJSON(const rapidjson::Value &jsonelement);
                // bool SerializeJSONInto(rapidjson::Document &doc);
        };

        class MacroDeviceDefinition {
            public:
                std::string id;
                std::string name;
                std::string synthId;
                // std::vector<MacroDeviceParameterGroup> parameterGroups;
                std::vector<MacroDeviceOutputMapping> outputMappings;
            public:
                MacroDeviceDefinition();
                ~MacroDeviceDefinition();
                MacroDeviceDefinition *copy();
                bool DeserializeJSON(const rapidjson::Value &jsonelement);
                // bool SerializeJSONInto(rapidjson::Document &doc);
        };
    }
}