#include "DrumRackSynth.hpp"
#include "DrumRackABD.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

// const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackABD::Init(const DrumRackInitData *initdata) {
    abd.Init();

    initdata->rack->registerParam(initdata->prefix, "ab_trigger", [&](const int val){ ab_trigger = val;});
	initdata->rack->registerTrig(initdata->prefix, "ab_trigger", [&](const int val){ trig_ab_trigger = val;});
	initdata->rack->registerParam(initdata->prefix, "ab_mute", [&](const int val){ ab_mute = val;});
	initdata->rack->registerTrig(initdata->prefix, "ab_mute", [&](const int val){ trig_ab_mute = val;});
	initdata->rack->registerParam(initdata->prefix, "ab_lev", [&](const int val){ ab_lev = val;});
	initdata->rack->registerCv(initdata->prefix, "ab_lev", [&](const int val){ cv_ab_lev = val;});
	initdata->rack->registerParam(initdata->prefix, "ab_pan", [&](const int val){ ab_pan = val;});
	initdata->rack->registerCv(initdata->prefix, "ab_pan", [&](const int val){ cv_ab_pan = val;});
	initdata->rack->registerParam(initdata->prefix, "ab_fx1", [&](const int val){ ab_fx1 = val;});
	initdata->rack->registerCv(initdata->prefix, "ab_fx1", [&](const int val){ cv_ab_fx1 = val;});
	initdata->rack->registerParam(initdata->prefix, "ab_fx2", [&](const int val){ ab_fx2 = val;});
	initdata->rack->registerCv(initdata->prefix, "ab_fx2", [&](const int val){ cv_ab_fx2 = val;});
	initdata->rack->registerParam(initdata->prefix, "ab_accent", [&](const int val){ ab_accent = val;});
	initdata->rack->registerCv(initdata->prefix, "ab_accent", [&](const int val){ cv_ab_accent = val;});
	initdata->rack->registerParam(initdata->prefix, "ab_f0", [&](const int val){ ab_f0 = val;});
	initdata->rack->registerCv(initdata->prefix, "ab_f0", [&](const int val){ cv_ab_f0 = val;});
	initdata->rack->registerParam(initdata->prefix, "ab_tone", [&](const int val){ ab_tone = val;});
	initdata->rack->registerCv(initdata->prefix, "ab_tone", [&](const int val){ cv_ab_tone = val;});
	initdata->rack->registerParam(initdata->prefix, "ab_decay", [&](const int val){ ab_decay = val;});
	initdata->rack->registerCv(initdata->prefix, "ab_decay", [&](const int val){ cv_ab_decay = val;});
	initdata->rack->registerParam(initdata->prefix, "ab_a_fm", [&](const int val){ ab_a_fm = val;});
	initdata->rack->registerCv(initdata->prefix, "ab_a_fm", [&](const int val){ cv_ab_a_fm = val;});
	initdata->rack->registerParam(initdata->prefix, "ab_s_fm", [&](const int val){ ab_s_fm = val;});
	initdata->rack->registerCv(initdata->prefix, "ab_s_fm", [&](const int val){ cv_ab_s_fm = val;});
}

// void DrumRackABD::SetParamValue(const string &id, const string &key, const int val) {
//     // Implementation of setting parameter value
//     // This is where you would handle the parameter setting logic
//     // For example, you might store the value in a map or update an internal state
//     // ESP_LOGI("DrumRackABD", "Setting parameter %s with key %s to value %d", id.c_str(), key.c_str(), val);
//     // You can add your specific logic here
// }

void DrumRackABD::Process(const DrumRackProcessData &data) {
	  // Analog Bass Drum
    MK_BOOL_PAR(bABMute, ab_mute)
    MK_BOOL_PAR(bABTrig, ab_trigger)
    if (bABTrig != abd_trig_prev){
        abd_trig_prev = bABTrig;
    }
    else{
        bABTrig = false;
    }

    MK_FLT_PAR_ABS_PAN(fABPan, ab_pan, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fABLev, ab_lev, 4095.f, 2.f); fABLev *= fABLev;
    MK_FLT_PAR_ABS(fABFX1Send, ab_fx1, 4095.f, maxFXSendLevelDly); fABFX1Send *= fABFX1Send;
    MK_FLT_PAR_ABS(fABFX2Send, ab_fx2, 4095.f, maxFXSendLevelRev); fABFX2Send *= fABFX2Send;

    if (bABMute || fABLev < minVolume) {
        return;
    }

    MK_FLT_PAR_ABS(fABAccent, ab_accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(fABF0, ab_f0, 4095.f, 0.0001f, 0.01f)
    MK_FLT_PAR_ABS(fABTone, ab_tone, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fABDecay, ab_decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(fABAfm, ab_a_fm, 4095.f, 0.f, 100.f)
    MK_FLT_PAR_ABS_MIN_MAX(fABSfm, ab_s_fm, 4095.f, 0.f, 100.f)
    abd.Render(
        false,
        bABTrig,
        fABAccent,
        fABF0,
        fABTone,
        fABDecay,
        fABAfm,
        fABSfm,
        abd_out,
        32);

    // mixRenderOutputMono(abd_out, fABLev, fABPan, fABFX1Send, fABFX2Send);

    // mixRenderOutputMono(td3_out, fTD3Lev, fTD3Pan, fTD3FX1Send, fTD3FX2Send);

}
