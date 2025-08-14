#include "DrumRackSynth.hpp"
#include "DrumRackInput.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

// const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackInput::Init(const DrumRackInitData *initdata) {
    this->enabled = false;
}

void DrumRackInput::Process(const DrumRackProcessData &data) {
    if (!this->enabled) {
        return;
    }

    // memcpy(in_out, data.buf, sizeof(float) * 32 * 2);
}
