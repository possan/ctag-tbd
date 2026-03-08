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
            uint32_t firstNonWtSlice;
            HELPERS::ctagSampleRom *sampleRom;
            uint32_t msPerBeat;
            uint32_t tempo; // BPM * 100
            uint8_t quantum;
            float *inputbuffer;
        };

        struct PickSeqRackInitData {
            int track_index;
            const char *prefix;
            int midi_channel;
            int cc_base;
            ctagSoundProcessorPicoSeqRack *rack;
        };
    }
}