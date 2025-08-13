#include "DrumRackSynth.hpp"
#include "DrumRackFxReverb.hpp"

using namespace CTAG::SP;

// const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackFxReverb::Init(const DrumRackInitData *initdata) {
     
}

// void DrumRackFxReverb::SetParamValue(const string &id, const string &key, const int val) {
//     // Implementation of setting parameter value
//     // This is where you would handle the parameter setting logic
//     // For example, you might store the value in a map or update an internal state
//     // ESP_LOGI("DrumRackFxReverb", "Setting parameter %s with key %s to value %d", id.c_str(), key.c_str(), val);
//     // You can add your specific logic here
// }

void DrumRackFxReverb::Process(const DrumRackProcessData &data) {
	 

    // mixRenderOutputMono(td3_out, fTD3Lev, fTD3Pan, fTD3FX1Send, fTD3FX2Send);

}
