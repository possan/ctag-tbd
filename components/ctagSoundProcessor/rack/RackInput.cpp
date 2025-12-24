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
    // memcpy(in_out, data.buf, sizeof(float) * 32 * 2);
}
