#pragma once

#include <vector>
#include <string>
#include "ctagDataModelBase.hpp"

namespace CTAG {
    namespace MACROPRESETS {
        class MacroSoundPreset;
        class MacroSoundPresetGroup;

        class MacroSoundPresetDataModel final : public CTAG::SP::ctagDataModelBase{
            private:
                std::vector<MacroSoundPreset*> presets;
                std::vector<MacroSoundPresetGroup*> groups;
            public:
                MacroSoundPresetDataModel();
                ~MacroSoundPresetDataModel();
                void ReloadSoundPresets();
                int GetNumberOfSoundPresetGroups();
                void GetMacroSoundPresetGroupId(int index, std::string *idOutput);
                int GetNumberOfSoundPresets();
                void GetPresetIndexJson(std::string *output);
                void SerializeListJSON(std::string *output);
                void SerializeItemJSON(const std::string &id, std::string *output);
                MacroSoundPreset *GetMacroSoundPreset(const std::string id);
                bool UpdatePreset(const std::string &jsonString);
                void DeleteItem(const std::string &id);
        };
    }
}