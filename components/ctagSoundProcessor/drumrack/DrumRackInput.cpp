#include "DrumRackSynth.hpp"
#include "DrumRackInput.hpp"
#include "../ctagSoundProcessorPicoSeqRack.hpp"

using namespace CTAG::SP;

// const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackInput::Init(const PickSeqRackInitData *initdata) {
    this->enabled = false;
}

// void DrumRackInput::handleMidiCC(uint8_t control, uint8_t value) {
//     // TODO: Implement
//     printf("Input CC %d %d\n", control, value);
// }

void DrumRackInput::Process(const PicoSeqRackProcessData &data) {
    if (!this->enabled) {
        return;
    }

    // memcpy(in_out, data.buf, sizeof(float) * 32 * 2);
}
