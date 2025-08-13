#include "DrumRackSynth.hpp"
#include "DrumRackHH2.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

// const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackHH2::Init(const DrumRackInitData *initdata) {
    hh2.Init();

    initdata->rack->registerParam(initdata->prefix, "hh2_trigger", [&](const int val){ hh2_trigger = val;});
	initdata->rack->registerTrig(initdata->prefix, "hh2_trigger", [&](const int val){ trig_hh2_trigger = val;});
	initdata->rack->registerParam(initdata->prefix, "hh2_mute", [&](const int val){ hh2_mute = val;});
	initdata->rack->registerTrig(initdata->prefix, "hh2_mute", [&](const int val){ trig_hh2_mute = val;});
	initdata->rack->registerParam(initdata->prefix, "hh2_lev", [&](const int val){ hh2_lev = val;});
	initdata->rack->registerCv(initdata->prefix, "hh2_lev", [&](const int val){ cv_hh2_lev = val;});
	initdata->rack->registerParam(initdata->prefix, "hh2_pan", [&](const int val){ hh2_pan = val;});
	initdata->rack->registerCv(initdata->prefix, "hh2_pan", [&](const int val){ cv_hh2_pan = val;});
	initdata->rack->registerParam(initdata->prefix, "hh2_fx1", [&](const int val){ hh2_fx1 = val;});
	initdata->rack->registerCv(initdata->prefix, "hh2_fx1", [&](const int val){ cv_hh2_fx1 = val;});
	initdata->rack->registerParam(initdata->prefix, "hh2_fx2", [&](const int val){ hh2_fx2 = val;});
	initdata->rack->registerCv(initdata->prefix, "hh2_fx2", [&](const int val){ cv_hh2_fx2 = val;});
	initdata->rack->registerParam(initdata->prefix, "hh2_accent", [&](const int val){ hh2_accent = val;});
	initdata->rack->registerCv(initdata->prefix, "hh2_accent", [&](const int val){ cv_hh2_accent = val;});
	initdata->rack->registerParam(initdata->prefix, "hh2_f0", [&](const int val){ hh2_f0 = val;});
	initdata->rack->registerCv(initdata->prefix, "hh2_f0", [&](const int val){ cv_hh2_f0 = val;});
	initdata->rack->registerParam(initdata->prefix, "hh2_tone", [&](const int val){ hh2_tone = val;});
	initdata->rack->registerCv(initdata->prefix, "hh2_tone", [&](const int val){ cv_hh2_tone = val;});
	initdata->rack->registerParam(initdata->prefix, "hh2_decay", [&](const int val){ hh2_decay = val;});
	initdata->rack->registerCv(initdata->prefix, "hh2_decay", [&](const int val){ cv_hh2_decay = val;});
	initdata->rack->registerParam(initdata->prefix, "hh2_noise", [&](const int val){ hh2_noise = val;});
	initdata->rack->registerCv(initdata->prefix, "hh2_noise", [&](const int val){ cv_hh2_noise = val;});

}

// void DrumRackHH2::SetParamValue(const string &id, const string &key, const int val) {
//     // Implementation of setting parameter value
//     // This is where you would handle the parameter setting logic
//     // For example, you might store the value in a map or update an internal state
//     // ESP_LOGI("DrumRackHH2", "Setting parameter %s with key %s to value %d", id.c_str(), key.c_str(), val);
//     // You can add your specific logic here
// }

void DrumRackHH2::Process(const DrumRackProcessData &data) {
	 
MK_BOOL_PAR(bHH2Mute, hh2_mute)
    MK_BOOL_PAR(bHH2Trig, hh2_trigger)
    if (bHH2Trig != hh2_trig_prev){
        hh2_trig_prev = bHH2Trig;
    }
    else{
        bHH2Trig = false;
    }

    MK_FLT_PAR_ABS_PAN(fHH2Pan, hh2_pan, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fHH2Lev, hh2_lev, 4095.f, 2.f); fHH2Lev *= fHH2Lev;
    MK_FLT_PAR_ABS(fHH2FX1Send, hh2_fx1, 4095.f, maxFXSendLevelDly); fHH2FX1Send *= fHH2FX1Send;
    MK_FLT_PAR_ABS(fHH2FX2Send, hh2_fx2, 4095.f, maxFXSendLevelRev); fHH2FX2Send *= fHH2FX2Send;

    if (bHH2Mute || fHH2Lev < minVolume) {
        return;
    }

    MK_FLT_PAR_ABS(fHH2Accent, hh2_accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(fHH2F0, hh2_f0, 4095.f, .00001f, .1f)
    MK_FLT_PAR_ABS(fHH2Tone, hh2_tone, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fHH2Decay, hh2_decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fHH2Noise, hh2_noise, 4095.f, 1.f)
    hh2.Render(
        false,
        bHH2Trig,
        fHH2Accent,
        fHH2F0,
        fHH2Tone,
        fHH2Decay,
        fHH2Noise,
        temp1_,
        temp2_,
        hh2_out,
        32);

    // mixRenderOutputMono(hh2_out, fHH2Lev, fHH2Pan, fHH2FX1Send, fHH2FX2Send);
    // mixRenderOutputMono(td3_out, fTD3Lev, fTD3Pan, fTD3FX1Send, fTD3FX2Send);

}
