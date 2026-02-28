#pragma once

#include <string>
#include <vector>
#include "rapidjson/document.h"

namespace CTAG {
    namespace MACROPRESETS {
        enum SynthParameterType {
            SynthParameterType_None = 0,
            SynthParameterType_CC = 1,
            SynthParameterType_NRPM = 2,
        };

        class SynthParameter {
            public:
                std::string id;
                std::string name;
                enum SynthParameterType type;
                uint16_t defaultValue;
                uint8_t cc;

            public:
                SynthParameter();
                ~SynthParameter();
        };

        enum SynthType {
            SynthType_None = 0,
            SynthType_Synth = 1,
            SynthType_Drum = 2,
        };

        class SynthDefinition {
            public:
                std::string id;
                std::string name;
                enum SynthType type;
                std::vector<SynthParameter*> parameters;

            public:
                SynthDefinition();
                ~SynthDefinition();
                bool DeserializeJSON(const rapidjson::Value &jsonelement);
                // bool SerializeJSONInto(const rapidjson::Value &jsonelement, rapidjson::Document::AllocatorType &allocator);
        };
    }
}