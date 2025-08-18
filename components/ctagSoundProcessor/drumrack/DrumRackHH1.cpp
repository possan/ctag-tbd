#include "DrumRackSynth.hpp"
#include "DrumRackHH1.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackHH1::Init(const DrumRackInitData *initdata) {
    hh1.Init();

    initdata->rack->registerParam(initdata->prefix, "trigger", [&](const int val){ hh1_trigger = val;});
	initdata->rack->registerTrig(initdata->prefix, "trigger", [&](const int val){ trig_hh1_trigger = val;});
	initdata->rack->registerParam(initdata->prefix, "accent", [&](const int val){ hh1_accent = val;});
	initdata->rack->registerCv(initdata->prefix, "accent", [&](const int val){ cv_hh1_accent = val;});
	initdata->rack->registerParam(initdata->prefix, "f0", [&](const int val){ hh1_f0 = val;});
	initdata->rack->registerCv(initdata->prefix, "f0", [&](const int val){ cv_hh1_f0 = val;});
	initdata->rack->registerParam(initdata->prefix, "tone", [&](const int val){ hh1_tone = val;});
	initdata->rack->registerCv(initdata->prefix, "tone", [&](const int val){ cv_hh1_tone = val;});
	initdata->rack->registerParam(initdata->prefix, "decay", [&](const int val){ hh1_decay = val;});
	initdata->rack->registerCv(initdata->prefix, "decay", [&](const int val){ cv_hh1_decay = val;});
	initdata->rack->registerParam(initdata->prefix, "noise", [&](const int val){ hh1_noise = val;});
	initdata->rack->registerCv(initdata->prefix, "noise", [&](const int val){ cv_hh1_noise = val;});

    this->enabled = false;
}

void DrumRackHH1::Process(const DrumRackProcessData &data) {
    std::fill_n(out, BUF_SZ, 0.f);

    if (!this->enabled) {
        return;
    }

    MK_BOOL_PAR(_trig, hh1_trigger)
    if (_trig != trig_prev) {
        if (_trig) {
            printf("HH1\n");
        }
        trig_prev = _trig;
    }

    MK_FLT_PAR_ABS(_accent, hh1_accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(_f0, hh1_f0, 4095.f, 0.0005f, 0.1f)
    MK_FLT_PAR_ABS(_tone, hh1_tone, 4095.f, 1.f)
    MK_FLT_PAR_ABS(_decay, hh1_decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS(_noise, hh1_noise, 4095.f, 1.f)
    hh1.Render(
        false,
        _trig,
        _accent,
        _f0,
        _tone,
        _decay,
        _noise,
        temp1,
        temp2,
        out,
        BUF_SZ);

    if (out[0] != out[0]) {
        printf("DrumRackHH1: NaN detected!\n");
        hh1.Init();
    }
}
