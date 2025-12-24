#include "RackSynth.hpp"
#include "RackInput.hpp"
#include "../ctagSoundProcessorPicoSeqRack.hpp"

using namespace CTAG::SP;

// const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void RackInput::Init(const PickSeqRackInitData *initdata) {
    this->enabled = false;
}

// void RackInput::handleMidiCC(uint8_t control, uint8_t value) {
//     // TODO: Implement
//     printf("Input CC %d %d\n", control, value);
// }

void RackInput::Process(const PicoSeqRackProcessData &data) {
    if (!this->enabled) {
        return;
    }

    // memcpy(in_out, data.buf, sizeof(float) * 32 * 2);
}
