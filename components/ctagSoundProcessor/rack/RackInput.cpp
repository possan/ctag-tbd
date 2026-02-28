#include "RackSynth.hpp"
#include "RackInput.hpp"
#include "../ctagSoundProcessorPicoSeqRack.hpp"

using namespace CTAG::SP;

void RackInput::Init(const PickSeqRackInitData *initdata) {
    this->enabled = false;
}

void RackInput::Process(const PicoSeqRackProcessData &data) {
    if (!this->enabled) {
        return;
    }

    // std::fill_n(out, BUF_SZ, 0.f);
    memcpy(out, data.inputbuffer, sizeof(float) * 32 * 2);
}
