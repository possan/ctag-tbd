#pragma once

#include <vector>
#include <set>
#include <string>

namespace CTAG {
    namespace MACROPRESETS {
        class MacroSoundPresetGroup {
            public:
                std::string id;
                std::string displayName;
                std::set<uint8_t> validTracks;
                std::vector<std::string> fileIds;
                MacroSoundPresetGroup();
                ~MacroSoundPresetGroup();
        };
    }
}