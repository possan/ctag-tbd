#pragma once

#include <vector>
#include <string>

namespace CTAG {
    namespace MACROPRESETS {
        class MacroSoundPresetGroup {
            public:
                std::string id;
                std::string displayName;
                std::vector<std::string> fileIds;
                MacroSoundPresetGroup();
                ~MacroSoundPresetGroup();
        };
    }
}