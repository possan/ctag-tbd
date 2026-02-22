#pragma once

#include <stdint.h>
#include <vector>
#include <string>
#include <ctagSoundProcessor.hpp>
#include "SynthDefinitionDataModel.hpp"
#include "MacroSoundPresetDataModel.hpp"
#include "MacroDeviceDefinition.hpp"
#include "MacroDeviceDefinitionDataModel.hpp"

namespace CTAG {
    namespace MACROPRESETS {
        class MacroTranslator {
            private:
                int midiChannelToTrack[16];
                int trackToMidiChannel[16];
                int trackBaseCC[16];
                int trackParameterValues[16][16];
                int trackDirty[16];
                int trackOutputMappingCC[16][16]; // [track][parameterIndex] => CC
                std::string trackMachineId[16];
                MacroDeviceDefinition *definition[16];

                void _parseIncomingMidiMessages(const uint8_t *buf, const size_t len);

            public:
                MacroTranslator();
                ~MacroTranslator();

                std::shared_ptr<SynthDefinitionDataModel> synthDefinitionModel;

                std::shared_ptr<MacroSoundPresetDataModel> macroSoundDefinitionModel;

                std::shared_ptr<MacroDeviceDefinitionDataModel> macroDeviceDefinitionModel;

                CTAG::SP::ctagSoundProcessor *soundProcessor;

                void SetTrackMachine(const int trackIndex, const std::string &synthID);

                void SetTrackMacroDefinition(const int trackIndex, MacroDeviceDefinition *def);

                void SetTrackParameter(const int trackIndex, int parameterIndex, int32_t value);

                void SetTrackParametersFromJSON(const int trackIndex, const std::   string &parametersJSON);

                void TranslateInput(CTAG::SP::ProcessData *pd);
        };
    }
}