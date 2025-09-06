#include "DrumRackSynth.hpp"
#include "DrumRackFxDelay.hpp"

using namespace CTAG::SP;

// const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackFxDelay::Init(const DrumRackInitData *initdata) {
     
}

// void DrumRackFxDelay::handleMidiCC(uint8_t control, uint8_t value) {
//     // TODO: Implement
//     printf("FxDelay CC %d %d\n", control, value);
// }

void DrumRackFxDelay::Process(const DrumRackProcessData &data) {
	 

    // mixRenderOutputMono(td3_out, fTD3Lev, fTD3Pan, fTD3FX1Send, fTD3FX2Send);

}
