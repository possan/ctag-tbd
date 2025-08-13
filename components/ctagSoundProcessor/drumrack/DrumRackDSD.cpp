#include "DrumRackSynth.hpp"
#include "DrumRackDSD.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

// const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackDSD::Init(const DrumRackInitData *initdata) {
    dsd.Init();

    initdata->rack->registerParam(initdata->prefix, "ds_trigger", [&](const int val){ ds_trigger = val;});
	initdata->rack->registerTrig(initdata->prefix, "ds_trigger", [&](const int val){ trig_ds_trigger = val;});
	initdata->rack->registerParam(initdata->prefix, "ds_mute", [&](const int val){ ds_mute = val;});
	initdata->rack->registerTrig(initdata->prefix, "ds_mute", [&](const int val){ trig_ds_mute = val;});
	initdata->rack->registerParam(initdata->prefix, "ds_lev", [&](const int val){ ds_lev = val;});
	initdata->rack->registerCv(initdata->prefix, "ds_lev", [&](const int val){ cv_ds_lev = val;});
	initdata->rack->registerParam(initdata->prefix, "ds_pan", [&](const int val){ ds_pan = val;});
	initdata->rack->registerCv(initdata->prefix, "ds_pan", [&](const int val){ cv_ds_pan = val;});
	initdata->rack->registerParam(initdata->prefix, "ds_fx1", [&](const int val){ ds_fx1 = val;});
	initdata->rack->registerCv(initdata->prefix, "ds_fx1", [&](const int val){ cv_ds_fx1 = val;});
	initdata->rack->registerParam(initdata->prefix, "ds_fx2", [&](const int val){ ds_fx2 = val;});
	initdata->rack->registerCv(initdata->prefix, "ds_fx2", [&](const int val){ cv_ds_fx2 = val;});
	initdata->rack->registerParam(initdata->prefix, "ds_accent", [&](const int val){ ds_accent = val;});
	initdata->rack->registerCv(initdata->prefix, "ds_accent", [&](const int val){ cv_ds_accent = val;});
	initdata->rack->registerParam(initdata->prefix, "ds_f0", [&](const int val){ ds_f0 = val;});
	initdata->rack->registerCv(initdata->prefix, "ds_f0", [&](const int val){ cv_ds_f0 = val;});
	initdata->rack->registerParam(initdata->prefix, "ds_fm_amt", [&](const int val){ ds_fm_amt = val;});
	initdata->rack->registerCv(initdata->prefix, "ds_fm_amt", [&](const int val){ cv_ds_fm_amt = val;});
	initdata->rack->registerParam(initdata->prefix, "ds_decay", [&](const int val){ ds_decay = val;});
	initdata->rack->registerCv(initdata->prefix, "ds_decay", [&](const int val){ cv_ds_decay = val;});
	initdata->rack->registerParam(initdata->prefix, "ds_spy", [&](const int val){ ds_spy = val;});
	initdata->rack->registerCv(initdata->prefix, "ds_spy", [&](const int val){ cv_ds_spy = val;});

}

// void DrumRackDSD::SetParamValue(const string &id, const string &key, const int val) {
//     // Implementation of setting parameter value
//     // This is where you would handle the parameter setting logic
//     // For example, you might store the value in a map or update an internal state
//     // ESP_LOGI("DrumRackDSD", "Setting parameter %s with key %s to value %d", id.c_str(), key.c_str(), val);
//     // You can add your specific logic here
// }

void DrumRackDSD::Process(const DrumRackProcessData &data) {
	 
  MK_BOOL_PAR(bDSMute, ds_mute)
    MK_BOOL_PAR(bDSTrig, ds_trigger)
    if (bDSTrig != dsd_trig_prev){
        dsd_trig_prev = bDSTrig;
    }
    else{
        bDSTrig = false;
    }

    MK_FLT_PAR_ABS_PAN(fDSPan, ds_pan, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fDSLev, ds_lev, 4095.f, 2.f); fDSLev *= fDSLev;
    MK_FLT_PAR_ABS(fDSFX1Send, ds_fx1, 4095.f, maxFXSendLevelDly); fDSFX1Send *= fDSFX1Send;
    MK_FLT_PAR_ABS(fDSFX2Send, ds_fx2, 4095.f, maxFXSendLevelRev); fDSFX2Send *= fDSFX2Send;

    if (bDSMute || fDSLev < minVolume) {
        return;
    }

    MK_FLT_PAR_ABS(fDSAccent, ds_accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(fDSF0, ds_f0, 4095.f, 0.0008f, 0.01f)
    MK_FLT_PAR_ABS(fDSFmAmt, ds_fm_amt, 4095.f, 1.5f)
    MK_FLT_PAR_ABS(fDSDecay, ds_decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fDSSpy, ds_spy, 4095.f, 1.f)
    dsd.Render(
        false,
        bDSTrig,
        fDSAccent,
        fDSF0,
        fDSFmAmt,
        fDSDecay,
        fDSSpy,
        dsd_out,
        32);

    // mixRenderOutputMono(dsd_out, fDSLev, fDSPan, fDSFX1Send, fDSFX2Send);
    // mixRenderOutputMono(td3_out, fTD3Lev, fTD3Pan, fTD3FX1Send, fTD3FX2Send);

}
