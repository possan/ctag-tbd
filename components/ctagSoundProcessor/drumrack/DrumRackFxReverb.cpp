#include "DrumRackSynth.hpp"
#include "DrumRackFxReverb.hpp"

using namespace CTAG::SP;

// const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackFxReverb::Init(const DrumRackInitData *initdata) {
     
}

// void DrumRackFxReverb::handleMidiCC(uint8_t control, uint8_t value) {
//     // TODO: Implement
//     printf("FxReverb CC %d %d\n", control, value);
// }

void DrumRackFxReverb::Process(const DrumRackProcessData &data) {
	 

    // mixRenderOutputMono(td3_out, fTD3Lev, fTD3Pan, fTD3FX1Send, fTD3FX2Send);

}
