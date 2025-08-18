#include "DrumRackSynth.hpp"
#include "DrumRackDSD.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

// const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackDSD::Init(const DrumRackInitData *initdata) {
    dsd.Init();

    initdata->rack->registerParam(initdata->prefix, "trigger", [&](const int val){ trigger = val;});
	initdata->rack->registerTrig(initdata->prefix, "trigger", [&](const int val){ trig_trigger = val;});
	initdata->rack->registerParam(initdata->prefix, "accent", [&](const int val){ accent = val;});
	initdata->rack->registerCv(initdata->prefix, "accent", [&](const int val){ cv_accent = val;});
	initdata->rack->registerParam(initdata->prefix, "f0", [&](const int val){ f0 = val;});
	initdata->rack->registerCv(initdata->prefix, "f0", [&](const int val){ cv_f0 = val;});
	initdata->rack->registerParam(initdata->prefix, "fm_amt", [&](const int val){ fm_amt = val;});
	initdata->rack->registerCv(initdata->prefix, "fm_amt", [&](const int val){ cv_fm_amt = val;});
	initdata->rack->registerParam(initdata->prefix, "decay", [&](const int val){ decay = val;});
	initdata->rack->registerCv(initdata->prefix, "decay", [&](const int val){ cv_decay = val;});
	initdata->rack->registerParam(initdata->prefix, "spy", [&](const int val){ spy = val;});
	initdata->rack->registerCv(initdata->prefix, "spy", [&](const int val){ cv_spy = val;});

    this->enabled = false;
}

void DrumRackDSD::Process(const DrumRackProcessData &data) {
    std::fill_n(out, BUF_SZ, 0.f);

    MK_BOOL_PAR(_trig, trigger)
    if (_trig != trig_prev){
        if (_trig) {
            printf("DSD\n");
        }
        trig_prev = _trig;
    }

    if (!this->enabled) {
        return;
    }

    MK_FLT_PAR_ABS(_accent, accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(_f0, f0, 4095.f, 0.0008f, 0.01f)
    MK_FLT_PAR_ABS(_fmAmt, fm_amt, 4095.f, 1.5f)
    MK_FLT_PAR_ABS(_decay, decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS(_spy, spy, 4095.f, 1.f)
    dsd.Render(
        false,
        _trig,
        _accent,
        _f0,
        _fmAmt,
        _decay,
        _spy,
        out,
        BUF_SZ);

    if (out[0] != out[0]) {
        printf("DrumRackDSD: NaN detected!\n");
        dsd.Init();
    }
}
