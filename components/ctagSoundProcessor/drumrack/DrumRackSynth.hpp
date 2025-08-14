#pragma once

#include <stdint.h>
#include <string>
#include <memory>
#include <map>
#include <functional>
#include <atomic>

#include "../ctagSoundProcessor.hpp"

using namespace std;

namespace CTAG {
    namespace SP {
        class ctagSoundProcessorDrumRack;

        struct DrumRackProcessData {
            float *cv;
            uint8_t *trig;
            uint32_t firstNonWtSlice;
        };

        struct DrumRackInitData {
            const char *prefix;
            ctagSoundProcessorDrumRack *rack;
        };
    }
}