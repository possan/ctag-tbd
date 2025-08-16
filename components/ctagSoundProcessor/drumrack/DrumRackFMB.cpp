#include "DrumRackSynth.hpp"
#include "DrumRackFMB.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

// const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackFMB::Init(const DrumRackInitData *initdata) {
    fmb.Init();

    initdata->rack->registerParam(initdata->prefix, "trigger", [&](const int val){ fmb_trigger = val;});
	initdata->rack->registerTrig(initdata->prefix, "trigger", [&](const int val){ trig_fmb_trigger = val;});
	initdata->rack->registerParam(initdata->prefix, "use_ratio_mode", [&](const int val){ fmb_use_ratio_mode = val;});
	initdata->rack->registerTrig(initdata->prefix, "use_ratio_mode", [&](const int val){ trig_fmb_use_ratio_mode = val;});
	initdata->rack->registerParam(initdata->prefix, "mod_env_sync", [&](const int val){ fmb_mod_env_sync = val;});
	initdata->rack->registerTrig(initdata->prefix, "mod_env_sync", [&](const int val){ trig_fmb_mod_env_sync = val;});
	initdata->rack->registerParam(initdata->prefix, "f_b", [&](const int val){ fmb_f_b = val;});
	initdata->rack->registerCv(initdata->prefix, "f_b", [&](const int val){ cv_fmb_f_b = val;});
	initdata->rack->registerParam(initdata->prefix, "d_b", [&](const int val){ fmb_d_b = val;});
	initdata->rack->registerCv(initdata->prefix, "d_b", [&](const int val){ cv_fmb_d_b = val;});
	initdata->rack->registerParam(initdata->prefix, "f_m", [&](const int val){ fmb_f_m = val;});
	initdata->rack->registerCv(initdata->prefix, "f_m", [&](const int val){ cv_fmb_f_m = val;});
	initdata->rack->registerParam(initdata->prefix, "I", [&](const int val){ fmb_I = val;});
	initdata->rack->registerCv(initdata->prefix, "I", [&](const int val){ cv_fmb_I = val;});
	initdata->rack->registerParam(initdata->prefix, "d_m", [&](const int val){ fmb_d_m = val;});
	initdata->rack->registerCv(initdata->prefix, "d_m", [&](const int val){ cv_fmb_d_m = val;});
	initdata->rack->registerParam(initdata->prefix, "b_m", [&](const int val){ fmb_b_m = val;});
	initdata->rack->registerCv(initdata->prefix, "b_m", [&](const int val){ cv_fmb_b_m = val;});
	initdata->rack->registerParam(initdata->prefix, "A_f", [&](const int val){ fmb_A_f = val;});
	initdata->rack->registerCv(initdata->prefix, "A_f", [&](const int val){ cv_fmb_A_f = val;});
	initdata->rack->registerParam(initdata->prefix, "d_f", [&](const int val){ fmb_d_f = val;});
	initdata->rack->registerCv(initdata->prefix, "d_f", [&](const int val){ cv_fmb_d_f = val;});

    this->enabled = false;
}

void DrumRackFMB::Process(const DrumRackProcessData &data) {
    MK_BOOL_PAR(bFMBTrig, fmb_trigger)
    if (bFMBTrig != fmb_trig_prev) {
        if (bFMBTrig) {
            printf("DrumRackFMB: Trigger.\n");
            fmb.Trigger();
        }
	    fmb_trig_prev = bFMBTrig;
    }

    if (!this->enabled) {
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

    fmb.Process(fmb_out, BUF_SZ);
}
