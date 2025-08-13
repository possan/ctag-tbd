#include "DrumRackSynth.hpp"
#include "DrumRackFMB.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

// const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackFMB::Init(const DrumRackInitData *initdata) {

    fmb.Init();

    initdata->rack->registerParam(initdata->prefix, "fmb_trigger", [&](const int val){ fmb_trigger = val;});
	initdata->rack->registerTrig(initdata->prefix, "fmb_trigger", [&](const int val){ trig_fmb_trigger = val;});
	initdata->rack->registerParam(initdata->prefix, "fmb_mute", [&](const int val){ fmb_mute = val;});
	initdata->rack->registerTrig(initdata->prefix, "fmb_mute", [&](const int val){ trig_fmb_mute = val;});
	initdata->rack->registerParam(initdata->prefix, "fmb_lev", [&](const int val){ fmb_lev = val;});
	initdata->rack->registerCv(initdata->prefix, "fmb_lev", [&](const int val){ cv_fmb_lev = val;});
	initdata->rack->registerParam(initdata->prefix, "fmb_pan", [&](const int val){ fmb_pan = val;});
	initdata->rack->registerCv(initdata->prefix, "fmb_pan", [&](const int val){ cv_fmb_pan = val;});
	initdata->rack->registerParam(initdata->prefix, "fmb_fx1", [&](const int val){ fmb_fx1 = val;});
	initdata->rack->registerCv(initdata->prefix, "fmb_fx1", [&](const int val){ cv_fmb_fx1 = val;});
	initdata->rack->registerParam(initdata->prefix, "fmb_fx2", [&](const int val){ fmb_fx2 = val;});
	initdata->rack->registerCv(initdata->prefix, "fmb_fx2", [&](const int val){ cv_fmb_fx2 = val;});
	initdata->rack->registerParam(initdata->prefix, "fmb_use_ratio_mode", [&](const int val){ fmb_use_ratio_mode = val;});
	initdata->rack->registerTrig(initdata->prefix, "fmb_use_ratio_mode", [&](const int val){ trig_fmb_use_ratio_mode = val;});
	initdata->rack->registerParam(initdata->prefix, "fmb_mod_env_sync", [&](const int val){ fmb_mod_env_sync = val;});
	initdata->rack->registerTrig(initdata->prefix, "fmb_mod_env_sync", [&](const int val){ trig_fmb_mod_env_sync = val;});
	initdata->rack->registerParam(initdata->prefix, "fmb_f_b", [&](const int val){ fmb_f_b = val;});
	initdata->rack->registerCv(initdata->prefix, "fmb_f_b", [&](const int val){ cv_fmb_f_b = val;});
	initdata->rack->registerParam(initdata->prefix, "fmb_d_b", [&](const int val){ fmb_d_b = val;});
	initdata->rack->registerCv(initdata->prefix, "fmb_d_b", [&](const int val){ cv_fmb_d_b = val;});
	initdata->rack->registerParam(initdata->prefix, "fmb_f_m", [&](const int val){ fmb_f_m = val;});
	initdata->rack->registerCv(initdata->prefix, "fmb_f_m", [&](const int val){ cv_fmb_f_m = val;});
	initdata->rack->registerParam(initdata->prefix, "fmb_I", [&](const int val){ fmb_I = val;});
	initdata->rack->registerCv(initdata->prefix, "fmb_I", [&](const int val){ cv_fmb_I = val;});
	initdata->rack->registerParam(initdata->prefix, "fmb_d_m", [&](const int val){ fmb_d_m = val;});
	initdata->rack->registerCv(initdata->prefix, "fmb_d_m", [&](const int val){ cv_fmb_d_m = val;});
	initdata->rack->registerParam(initdata->prefix, "fmb_b_m", [&](const int val){ fmb_b_m = val;});
	initdata->rack->registerCv(initdata->prefix, "fmb_b_m", [&](const int val){ cv_fmb_b_m = val;});
	initdata->rack->registerParam(initdata->prefix, "fmb_A_f", [&](const int val){ fmb_A_f = val;});
	initdata->rack->registerCv(initdata->prefix, "fmb_A_f", [&](const int val){ cv_fmb_A_f = val;});
	initdata->rack->registerParam(initdata->prefix, "fmb_d_f", [&](const int val){ fmb_d_f = val;});
	initdata->rack->registerCv(initdata->prefix, "fmb_d_f", [&](const int val){ cv_fmb_d_f = val;});

}

// void DrumRackFMB::SetParamValue(const string &id, const string &key, const int val) {
//     // Implementation of setting parameter value
//     // This is where you would handle the parameter setting logic
//     // For example, you might store the value in a map or update an internal state
//     // ESP_LOGI("DrumRackFMB", "Setting parameter %s with key %s to value %d", id.c_str(), key.c_str(), val);
//     // You can add your specific logic here
// }

void DrumRackFMB::Process(const DrumRackProcessData &data) {
    MK_BOOL_PAR(bFMBMute, fmb_mute)
    MK_BOOL_PAR(bFMBTrig, fmb_trigger)
    if (bFMBTrig != fmb_trig_prev && bFMBTrig){
	    fmb_trig_prev = true;
	    fmb.Trigger();
    }
    else if (!bFMBTrig){
	    fmb_trig_prev = false;
    }

    MK_FLT_PAR_ABS_PAN(fFMBPan, fmb_pan, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fFMBLev, fmb_lev, 4095.f, 2.f); fFMBLev *= fFMBLev;
    MK_FLT_PAR_ABS(fFMBFX1Send, fmb_fx1, 4095.f, maxFXSendLevelDly); fFMBFX1Send *= fFMBFX1Send;
    MK_FLT_PAR_ABS(fFMBFX2Send, fmb_fx2, 4095.f, maxFXSendLevelRev); fFMBFX2Send *= fFMBFX2Send;

    if (bFMBMute || fFMBLev < minVolume) {
        return;
    }

    MK_BOOL_PAR(bFMBUseRatioMode, fmb_use_ratio_mode)
    MK_BOOL_PAR(bFMBModEnvSync, fmb_mod_env_sync)
    float fFMBF0 = fmb_f_b/4095.f * (200.f-20.f)+20.f;
    if(cv_fmb_f_b != -1){
        float fMod = data.cv[cv_fmb_f_b] * 5.f;
        fMod = CTAG::SP::HELPERS::fastpow2(fMod);
        fFMBF0 *= fMod;
    }
    MK_FLT_PAR_ABS_MIN_MAX(fFMBDecayBase, fmb_d_b, 4095.f, 0.001f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(fFMBFMod, fmb_f_m, 4095.f, 40.f, 2000.f)
    MK_FLT_PAR_ABS(fFMBRatioModIndex, fmb_f_m, 4095.f, 63.f)
    int iModIndex = static_cast<int>(fFMBRatioModIndex);
    CONSTRAIN(iModIndex, 0, 63)
    MK_FLT_PAR_ABS_MIN_MAX(fFMBI, fmb_I, 4095.f, 0.f, 10.f)
    MK_FLT_PAR_ABS_MIN_MAX(fFMBDecayMod, fmb_d_m, 4095.f, 0.001f, .5f)
    MK_INT_PAR(iModFeedback, fmb_b_m, 16.f)
    MK_FLT_PAR_ABS_MIN_MAX(fFMBAmpFreq, fmb_A_f, 4095.f, 0.f, 1000.f)
    MK_FLT_PAR_ABS_MIN_MAX(fFMBDecayFreq, fmb_d_f, 4095.f, 0.001f, .1f)

    fmb.params.use_ratio_mode = bFMBUseRatioMode;
    fmb.params.mod_env_sync = bFMBModEnvSync;
    fmb.params.f_b = fFMBF0;
    fmb.params.d_b = fFMBDecayBase;
    fmb.params.f_m = fFMBFMod;
    fmb.params.mod_ratio_index = iModIndex;
    fmb.params.I = fFMBI;
    fmb.params.d_m = fFMBDecayMod;
    fmb.params.b_m = static_cast<float>(iModFeedback);
    fmb.params.A_f = fFMBAmpFreq;
    fmb.params.d_f = fFMBDecayFreq;

    fmb.Process(fmb_out, 32);
    // mixRenderOutputMono(fmb_out, fFMBLev, fFMBPan, fFMBFX1Send, fFMBFX2Send);
    // mixRenderOutputMono(td3_out, fTD3Lev, fTD3Pan, fTD3FX1Send, fTD3FX2Send);

}
