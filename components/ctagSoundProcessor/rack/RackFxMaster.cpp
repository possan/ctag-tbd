#include "RackSynth.hpp"
#include "RackFxMaster.hpp"

using namespace CTAG::SP;

// const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void RackFxMaster::Init(const PickSeqRackInitData *initdata) {
     
}

// void RackFxMaster::handleMidiCC(uint8_t control, uint8_t value) {
//     // TODO: Implement
//     printf("FxMaster CC %d %d\n", control, value);
// }

void RackFxMaster::Process(const PicoSeqRackProcessData &data) {
	 

    // mixRenderOutputMono(td3_out, fTD3Lev, fTD3Pan, fTD3FX1Send, fTD3FX2Send);

}
