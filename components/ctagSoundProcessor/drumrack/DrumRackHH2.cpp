#include "DrumRackSynth.hpp"
#include "DrumRackHH2.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackHH2::Init(const DrumRackInitData *initdata) {
    hh2.Init();

    initdata->rack->registerParam(initdata->prefix, "trigger", [&](const int val){ trigger = val;});
	initdata->rack->registerTrig(initdata->prefix, "trigger", [&](const int val){ trig_trigger = val;});
	initdata->rack->registerParam(initdata->prefix, "accent", [&](const int val){ accent = val;});
	initdata->rack->registerCv(initdata->prefix, "accent", [&](const int val){ cv_accent = val;});
	initdata->rack->registerParam(initdata->prefix, "f0", [&](const int val){ f0 = val;});
	initdata->rack->registerCv(initdata->prefix, "f0", [&](const int val){ cv_f0 = val;});
	initdata->rack->registerParam(initdata->prefix, "tone", [&](const int val){ tone = val;});
	initdata->rack->registerCv(initdata->prefix, "tone", [&](const int val){ cv_tone = val;});
	initdata->rack->registerParam(initdata->prefix, "decay", [&](const int val){ decay = val;});
	initdata->rack->registerCv(initdata->prefix, "decay", [&](const int val){ cv_decay = val;});
	initdata->rack->registerParam(initdata->prefix, "noise", [&](const int val){ noise = val;});
	initdata->rack->registerCv(initdata->prefix, "noise", [&](const int val){ cv_noise = val;});

    this->enabled = false;
}

void DrumRackHH2::Process(const DrumRackProcessData &data) {
    std::fill_n(out, BUF_SZ, 0.f);

    MK_BOOL_PAR(_trig, trigger)
    if (_trig != trig_prev){
        if (_trig) {
            // printf("DrumRackHH2: Trigger.\n");
        }
        trig_prev = _trig;
    }

    if (!this->enabled) {
        return;
    }

    MK_FLT_PAR_ABS(_accent, accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(_f0, f0, 4095.f, .00001f, .1f)
    MK_FLT_PAR_ABS(_tone, tone, 4095.f, 1.f)
    MK_FLT_PAR_ABS(_decay, decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS(_noise, noise, 4095.f, 1.f)
    hh2.Render(
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
        printf("DrumRackHH2: NaN detected!\n");
        hh2.Init();
    }
}
