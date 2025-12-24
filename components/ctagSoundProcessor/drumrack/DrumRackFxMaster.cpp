#include "DrumRackSynth.hpp"
#include "DrumRackFxMaster.hpp"

using namespace CTAG::SP;

// const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackFxMaster::Init(const PickSeqRackInitData *initdata) {
     
}

// void DrumRackFxMaster::handleMidiCC(uint8_t control, uint8_t value) {
//     // TODO: Implement
//     printf("FxMaster CC %d %d\n", control, value);
// }

void DrumRackFxMaster::Process(const PicoSeqRackProcessData &data) {
	 

    // mixRenderOutputMono(td3_out, fTD3Lev, fTD3Pan, fTD3FX1Send, fTD3FX2Send);

}
