#include "DrumRackSynth.hpp"
#include "DrumRackASD.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

// const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackASD::Init(const DrumRackInitData *initdata) {
    asd.Init();

    initdata->rack->registerParam(initdata->prefix, "as_trigger", [&](const int val){ as_trigger = val;});
	initdata->rack->registerTrig(initdata->prefix, "as_trigger", [&](const int val){ trig_as_trigger = val;});
	initdata->rack->registerParam(initdata->prefix, "as_mute", [&](const int val){ as_mute = val;});
	initdata->rack->registerTrig(initdata->prefix, "as_mute", [&](const int val){ trig_as_mute = val;});
	initdata->rack->registerParam(initdata->prefix, "as_lev", [&](const int val){ as_lev = val;});
	initdata->rack->registerCv(initdata->prefix, "as_lev", [&](const int val){ cv_as_lev = val;});
	initdata->rack->registerParam(initdata->prefix, "as_pan", [&](const int val){ as_pan = val;});
	initdata->rack->registerCv(initdata->prefix, "as_pan", [&](const int val){ cv_as_pan = val;});
	initdata->rack->registerParam(initdata->prefix, "as_fx1", [&](const int val){ as_fx1 = val;});
	initdata->rack->registerCv(initdata->prefix, "as_fx1", [&](const int val){ cv_as_fx1 = val;});
	initdata->rack->registerParam(initdata->prefix, "as_fx2", [&](const int val){ as_fx2 = val;});
	initdata->rack->registerCv(initdata->prefix, "as_fx2", [&](const int val){ cv_as_fx2 = val;});
	initdata->rack->registerParam(initdata->prefix, "as_accent", [&](const int val){ as_accent = val;});
	initdata->rack->registerCv(initdata->prefix, "as_accent", [&](const int val){ cv_as_accent = val;});
	initdata->rack->registerParam(initdata->prefix, "as_f0", [&](const int val){ as_f0 = val;});
	initdata->rack->registerCv(initdata->prefix, "as_f0", [&](const int val){ cv_as_f0 = val;});
	initdata->rack->registerParam(initdata->prefix, "as_tone", [&](const int val){ as_tone = val;});
	initdata->rack->registerCv(initdata->prefix, "as_tone", [&](const int val){ cv_as_tone = val;});
	initdata->rack->registerParam(initdata->prefix, "as_decay", [&](const int val){ as_decay = val;});
	initdata->rack->registerCv(initdata->prefix, "as_decay", [&](const int val){ cv_as_decay = val;});
	initdata->rack->registerParam(initdata->prefix, "as_a_spy", [&](const int val){ as_a_spy = val;});
	initdata->rack->registerCv(initdata->prefix, "as_a_spy", [&](const int val){ cv_as_a_spy = val;});

}

// void DrumRackASD::SetParamValue(const string &id, const string &key, const int val) {
//     // Implementation of setting parameter value
//     // This is where you would handle the parameter setting logic
//     // For example, you might store the value in a map or update an internal state
//     // ESP_LOGI("DrumRackASD", "Setting parameter %s with key %s to value %d", id.c_str(), key.c_str(), val);
//     // You can add your specific logic here
// }

void DrumRackASD::Process(const DrumRackProcessData &data) {
    MK_BOOL_PAR(bASMute, as_mute)
    MK_BOOL_PAR(bASTrig, as_trigger)
    if (bASTrig != asd_trig_prev){
        asd_trig_prev = bASTrig;
    }
    else{
        bASTrig = false;
    }

    MK_FLT_PAR_ABS_PAN(fASPan, as_pan, 4095.f, 1.f);
    MK_FLT_PAR_ABS(fASLev, as_lev, 4095.f, 2.f); fASLev *= fASLev;
    MK_FLT_PAR_ABS(fASFX1Send, as_fx1, 4095.f, maxFXSendLevelDly); fASFX1Send *= fASFX1Send;
    MK_FLT_PAR_ABS(fASFX2Send, as_fx2, 4095.f, maxFXSendLevelRev); fASFX2Send *= fASFX2Send;

    if (bASMute || fASLev < minVolume) {
        // return;
    }

    MK_FLT_PAR_ABS(fASAccent, as_accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(fASF0, as_f0, 4095.f, 0.001f, 0.01f)
    MK_FLT_PAR_ABS(fASTone, as_tone, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fASDecay, as_decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fASAspy, as_a_spy, 4095.f, 1.f)
    asd.Render(
        false,
        bASTrig,
        fASAccent,
        fASF0,
        fASTone,
        fASDecay,
        fASAspy,
        asd_out,
        32);

    // mixRenderOutputMono(asd_out, fASLev, fASPan, fASFX1Send, fASFX2Send);
    // mixRenderOutputMono(td3_out, fTD3Lev, fTD3Pan, fTD3FX1Send, fTD3FX2Send);

}
