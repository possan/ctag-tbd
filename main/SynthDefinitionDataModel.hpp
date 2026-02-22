#pragma once

#include <string>
#include <vector>
#include "ctagDataModelBase.hpp"

namespace CTAG {
    namespace MACROPRESETS {
        class TrackDefinition;
        class SynthDefinition;

        class SynthDefinitionDataModel final : public CTAG::SP::ctagDataModelBase {
            private:
                std::vector<TrackDefinition*> tracks;
                std::vector<SynthDefinition*> synths;

            public:
                SynthDefinitionDataModel();
                ~SynthDefinitionDataModel();
                void ReloadSynthDefinitions();
                int GetNumberOfSynthDefinitions();
                void GetSynthDeviceDefinitionId(int index, std::string *idOutput);
                void GetSynthDefinitionsJSON(const std::string *output);
                SynthDefinition *GetSynthDefinition(const std::string id);
                TrackDefinition *GetTrackDefinition(int index);
                bool DeserializeJSON(const rapidjson::Value &jsonelement);
                bool DeserializeJSON(const std::string *jsonString);
                void SerializeJSON(std::string *output);
                void SerializeTrackJSON(int index, std::string *output);
                void SerializeSynthJSON(const std::string id, std::string *output);
                void SerializeListJSON(std::string *output);
                void SerializeStateJSON(std::string *output);
        };
    }
}