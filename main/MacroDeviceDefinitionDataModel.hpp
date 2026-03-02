#pragma once

#include <string>
#include <vector>
#include "ctagDataModelBase.hpp"

namespace CTAG {
    namespace MACROPRESETS {
        class MacroDeviceDefinition;

        class MacroDeviceDefinitionDataModel final : public CTAG::SP::ctagDataModelBase{
            private:
                std::vector<MacroDeviceDefinition*> definitions;
            public:
                MacroDeviceDefinitionDataModel();
                ~MacroDeviceDefinitionDataModel();
                void ReloadMachineDefinitions();
                int GetNumberOfDefinitions();
                // void GetMacroDeviceDefinitionId(int index, char *buffer, int bufferSize);
                MacroDeviceDefinition *LoadMacroDeviceDefinition(const std::string id);
                void SerializeListJSON(std::string *output);
                void SerializeItemJSON(const std::string &id, std::string *output);
                bool UpdateDefinition(const std::string &jsonString);
                void DeleteItem(const std::string &id);
        };
    }
}