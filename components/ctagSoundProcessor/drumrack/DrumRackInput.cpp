#include "DrumRackSynth.hpp"
#include "DrumRackInput.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

// const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackInput::Init(const DrumRackInitData *initdata) {
     
}

// void DrumRackInput::SetParamValue(const string &id, const string &key, const int val) {
//     // Implementation of setting parameter value
//     // This is where you would handle the parameter setting logic
//     // For example, you might store the value in a map or update an internal state
//     // ESP_LOGI("DrumRackInput", "Setting parameter %s with key %s to value %d", id.c_str(), key.c_str(), val);
//     // You can add your specific logic here
// }

void DrumRackInput::Process(const DrumRackProcessData &data) {
	 MK_BOOL_PAR(bMuteIN, in_mute)

    MK_FLT_PAR_ABS_PAN(fINPan, in_pan, 4095.f, 1.f);
    MK_FLT_PAR_ABS(fINLev, in_lev, 4095.f, 2.f); fINLev *= fINLev;
    MK_FLT_PAR_ABS(fINFX1Send, in_fx1, 4095.f, maxFXSendLevelDly); fINFX1Send *= fINFX1Send;
    MK_FLT_PAR_ABS(fINFX2Send, in_fx2, 4095.f, maxFXSendLevelRev); fINFX2Send *= fINFX2Send;

    if (bMuteIN || fINLev < minVolume) {
        return;
    }

    float in_out[32 * 2];
    memcpy(in_out, data.buf, sizeof(float) * 32 * 2);

    // mixRenderOutputStereo(in_out, fINLev, fINPan, fINFX1Send, fINFX2Send);    // mixRenderOutputMono(td3_out, fTD3Lev, fTD3Pan, fTD3FX1Send, fTD3FX2Send);

}
