#pragma once

#include <string>
#include <vector>
#include "rapidjson/document.h"

namespace CTAG {
    namespace MACROPRESETS {
        class TrackDefinition {
            public:
                int index;
                std::string name;
                int midiChannel;
                int drumNote;
                int baseCC;
                std::string activeMachineId;
                std::vector<std::string> macroMachineIds;

            public:
                TrackDefinition();
                ~TrackDefinition();
                bool DeserializeJSON(const rapidjson::Value &jsonelement);
                bool SerializeJSONInto(const rapidjson::Value &jsonelement, rapidjson::Document::AllocatorType &allocator);
        };
    }
}