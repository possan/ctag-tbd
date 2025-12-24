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

#define BUF_SZ 32

namespace CTAG {
    namespace SP {
        class ctagSoundProcessorPicoSeqRack;

        struct PicoSeqRackProcessData {
            float *cv;
            uint8_t *trig;
            uint32_t firstNonWtSlice;
            HELPERS::ctagSampleRom *sampleRom;
            uint32_t msPerBeat;
        };

        struct PickSeqRackInitData {
            const char *prefix;
            int midi_channel;
            int cc_base;
            ctagSoundProcessorPicoSeqRack *rack;
        };
    }
}