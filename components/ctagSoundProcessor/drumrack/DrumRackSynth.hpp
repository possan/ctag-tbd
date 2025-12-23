#pragma once

#include <stdint.h>
#include <string>
#include <memory>
#include <map>
#include <functional>
#include <atomic>

#include "../ctagSoundProcessor.hpp"
#include "helpers/ctagSampleRom.hpp"

using namespace std;

namespace CTAG {
    namespace SP {
        class ctagSoundProcessorDrumRack;

        struct DrumRackProcessData {
            float *cv;
            uint8_t *trig;
            uint32_t firstNonWtSlice;
            HELPERS::ctagSampleRom *sampleRom;
            uint32_t msPerBeat;
        };

        struct DrumRackInitData {
            const char *prefix;
            int midi_channel;
            int cc_base;
            ctagSoundProcessorDrumRack *rack;
        };
    }
}