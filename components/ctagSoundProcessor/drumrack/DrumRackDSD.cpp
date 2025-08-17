#include "DrumRackSynth.hpp"
#include "DrumRackDSD.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

// const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackDSD::Init(const DrumRackInitData *initdata) {
    dsd.Init();

    initdata->rack->registerParam(initdata->prefix, "trigger", [&](const int val){ ds_trigger = val;});
	initdata->rack->registerTrig(initdata->prefix, "trigger", [&](const int val){ trig_ds_trigger = val;});
	initdata->rack->registerParam(initdata->prefix, "accent", [&](const int val){ ds_accent = val;});
	initdata->rack->registerCv(initdata->prefix, "accent", [&](const int val){ cv_ds_accent = val;});
	initdata->rack->registerParam(initdata->prefix, "f0", [&](const int val){ ds_f0 = val;});
	initdata->rack->registerCv(initdata->prefix, "f0", [&](const int val){ cv_ds_f0 = val;});
	initdata->rack->registerParam(initdata->prefix, "fm_amt", [&](const int val){ ds_fm_amt = val;});
	initdata->rack->registerCv(initdata->prefix, "fm_amt", [&](const int val){ cv_ds_fm_amt = val;});
	initdata->rack->registerParam(initdata->prefix, "decay", [&](const int val){ ds_decay = val;});
	initdata->rack->registerCv(initdata->prefix, "decay", [&](const int val){ cv_ds_decay = val;});
	initdata->rack->registerParam(initdata->prefix, "spy", [&](const int val){ ds_spy = val;});
	initdata->rack->registerCv(initdata->prefix, "spy", [&](const int val){ cv_ds_spy = val;});

    this->enabled = false;
}

void DrumRackDSD::Process(const DrumRackProcessData &data) {
    std::fill_n(dsd_out, BUF_SZ, 0.f);

    MK_BOOL_PAR(bDSTrig, ds_trigger)
    if (bDSTrig != dsd_trig_prev){
        dsd_trig_prev = bDSTrig;
    }
    else{
        bDSTrig = false;
    }

    if (!this->enabled) {
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
        BUF_SZ);

    if (dsd_out[0] != dsd_out[0]) {
        printf("DrumRackDSD: NaN detected!\n");
        dsd.Init();
    }
}
