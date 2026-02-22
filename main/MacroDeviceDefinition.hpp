#pragma once

#include <string>
#include <vector>
#include "rapidjson/document.h"

namespace CTAG {
    namespace MACROPRESETS {
        class MacroDeviceParameter {
            public:
                uint8_t index;
                std::string name;
                int32_t defaultValue;
                int32_t minValue;
                int32_t maxValue;
                int32_t resolution;
                std::string uiType;
            public:
                MacroDeviceParameter();
                ~MacroDeviceParameter();
                bool DeserializeJSON(const rapidjson::Value &jsonelement);
                bool SerializeJSONInto(rapidjson::Document &doc);
        };

        class MacroDeviceParameterGroup {
            public:
                std::string name;
                std::vector<MacroDeviceParameter*> parameters;
            public:
                MacroDeviceParameterGroup();
                ~MacroDeviceParameterGroup();
                bool DeserializeJSON(const rapidjson::Value &jsonelement);
                bool SerializeJSONInto(rapidjson::Document &doc);
        };

        class MacroDeviceOutputMappingSource {
            public:
                uint8_t parameterIndex;
                int32_t amount;

            public:
                MacroDeviceOutputMappingSource();
                ~MacroDeviceOutputMappingSource();
                bool DeserializeJSON(const rapidjson::Value &jsonelement);
                bool SerializeJSONInto(rapidjson::Document &doc);
        };

        class MacroDeviceOutputMapping {
            public:
                std::string synthParameterId;
                int startValue;
                std::vector<MacroDeviceOutputMappingSource*> sources;

            public:
                MacroDeviceOutputMapping();
                ~MacroDeviceOutputMapping();
                bool DeserializeJSON(const rapidjson::Value &jsonelement);
                bool SerializeJSONInto(rapidjson::Document &doc);
        };

        class MacroDeviceDefinition {
            public:
                std::string id;
                std::string name;
                std::string synthId;
                std::vector<MacroDeviceParameterGroup*> parameterGroups;
                std::vector<MacroDeviceOutputMapping*> outputMappings;
            public:
                MacroDeviceDefinition();
                ~MacroDeviceDefinition();
                bool DeserializeJSON(const rapidjson::Value &jsonelement);
                bool SerializeJSONInto(rapidjson::Document &doc);
        };
    }
}