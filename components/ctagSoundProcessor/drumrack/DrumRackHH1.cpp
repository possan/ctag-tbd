#include "DrumRackSynth.hpp"
#include "DrumRackHH1.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

// const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackHH1::Init(const DrumRackInitData *initdata) {
    hh1.Init();

    initdata->rack->registerParam(initdata->prefix, "hh1_trigger", [&](const int val){ hh1_trigger = val;});
	initdata->rack->registerTrig(initdata->prefix, "hh1_trigger", [&](const int val){ trig_hh1_trigger = val;});
	initdata->rack->registerParam(initdata->prefix, "hh1_mute", [&](const int val){ hh1_mute = val;});
	initdata->rack->registerTrig(initdata->prefix, "hh1_mute", [&](const int val){ trig_hh1_mute = val;});
	initdata->rack->registerParam(initdata->prefix, "hh1_lev", [&](const int val){ hh1_lev = val;});
	initdata->rack->registerCv(initdata->prefix, "hh1_lev", [&](const int val){ cv_hh1_lev = val;});
	initdata->rack->registerParam(initdata->prefix, "hh1_pan", [&](const int val){ hh1_pan = val;});
	initdata->rack->registerCv(initdata->prefix, "hh1_pan", [&](const int val){ cv_hh1_pan = val;});
	initdata->rack->registerParam(initdata->prefix, "hh1_fx1", [&](const int val){ hh1_fx1 = val;});
	initdata->rack->registerCv(initdata->prefix, "hh1_fx1", [&](const int val){ cv_hh1_fx1 = val;});
	initdata->rack->registerParam(initdata->prefix, "hh1_fx2", [&](const int val){ hh1_fx2 = val;});
	initdata->rack->registerCv(initdata->prefix, "hh1_fx2", [&](const int val){ cv_hh1_fx2 = val;});
	initdata->rack->registerParam(initdata->prefix, "hh1_accent", [&](const int val){ hh1_accent = val;});
	initdata->rack->registerCv(initdata->prefix, "hh1_accent", [&](const int val){ cv_hh1_accent = val;});
	initdata->rack->registerParam(initdata->prefix, "hh1_f0", [&](const int val){ hh1_f0 = val;});
	initdata->rack->registerCv(initdata->prefix, "hh1_f0", [&](const int val){ cv_hh1_f0 = val;});
	initdata->rack->registerParam(initdata->prefix, "hh1_tone", [&](const int val){ hh1_tone = val;});
	initdata->rack->registerCv(initdata->prefix, "hh1_tone", [&](const int val){ cv_hh1_tone = val;});
	initdata->rack->registerParam(initdata->prefix, "hh1_decay", [&](const int val){ hh1_decay = val;});
	initdata->rack->registerCv(initdata->prefix, "hh1_decay", [&](const int val){ cv_hh1_decay = val;});
	initdata->rack->registerParam(initdata->prefix, "hh1_noise", [&](const int val){ hh1_noise = val;});
	initdata->rack->registerCv(initdata->prefix, "hh1_noise", [&](const int val){ cv_hh1_noise = val;});

}

// void DrumRackHH1::SetParamValue(const string &id, const string &key, const int val) {
//     // Implementation of setting parameter value
//     // This is where you would handle the parameter setting logic
//     // For example, you might store the value in a map or update an internal state
//     // ESP_LOGI("DrumRackHH1", "Setting parameter %s with key %s to value %d", id.c_str(), key.c_str(), val);
//     // You can add your specific logic here
// }

void DrumRackHH1::Process(const DrumRackProcessData &data) {
	 
MK_BOOL_PAR(bHH1Mute, hh1_mute)
    MK_BOOL_PAR(bHH1Trig, hh1_trigger)
    if (bHH1Trig != hh1_trig_prev){
        hh1_trig_prev = bHH1Trig;
    }
    else{
        bHH1Trig = false;
    }

    MK_FLT_PAR_ABS_PAN(fHH1Pan, hh1_pan, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fHH1Lev, hh1_lev, 4095.f, 2.f); fHH1Lev *= fHH1Lev;
    MK_FLT_PAR_ABS(fHH1FX1Send, hh1_fx1, 4095.f, maxFXSendLevelDly); fHH1FX1Send *= fHH1FX1Send;
    MK_FLT_PAR_ABS(fHH1FX2Send, hh1_fx2, 4095.f, maxFXSendLevelRev); fHH1FX2Send *= fHH1FX2Send;

    if (bHH1Mute || fHH1Lev < minVolume) {
        // return;
    }

    MK_FLT_PAR_ABS(fHH1Accent, hh1_accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(fHH1F0, hh1_f0, 4095.f, 0.0005f, 0.1f)
    MK_FLT_PAR_ABS(fHH1Tone, hh1_tone, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fHH1Decay, hh1_decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fHH1Noise, hh1_noise, 4095.f, 1.f)
    hh1.Render(
        false,
        bHH1Trig,
        fHH1Accent,
        fHH1F0,
        fHH1Tone,
        fHH1Decay,
        fHH1Noise,
        temp1_,
        temp2_,
        hh1_out,
        32);

    // mixRenderOutputMono(hh1_out, fHH1Lev, fHH1Pan, fHH1FX1Send, fHH1FX2Send);
    // mixRenderOutputMono(td3_out, fTD3Lev, fTD3Pan, fTD3FX1Send, fTD3FX2Send);

}
