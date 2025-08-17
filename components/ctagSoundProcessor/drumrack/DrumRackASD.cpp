#include "DrumRackSynth.hpp"
#include "DrumRackASD.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackASD::Init(const DrumRackInitData *initdata) {
    asd.Init();

    initdata->rack->registerParam(initdata->prefix, "trigger", [&](const int val){ as_trigger = val;});
	initdata->rack->registerTrig(initdata->prefix, "trigger", [&](const int val){ trig_as_trigger = val;});
	initdata->rack->registerParam(initdata->prefix, "accent", [&](const int val){ as_accent = val;});
	initdata->rack->registerCv(initdata->prefix, "accent", [&](const int val){ cv_as_accent = val;});
	initdata->rack->registerParam(initdata->prefix, "f0", [&](const int val){ as_f0 = val;});
	initdata->rack->registerCv(initdata->prefix, "f0", [&](const int val){ cv_as_f0 = val;});
	initdata->rack->registerParam(initdata->prefix, "tone", [&](const int val){ as_tone = val;});
	initdata->rack->registerCv(initdata->prefix, "tone", [&](const int val){ cv_as_tone = val;});
	initdata->rack->registerParam(initdata->prefix, "decay", [&](const int val){ as_decay = val;});
	initdata->rack->registerCv(initdata->prefix, "decay", [&](const int val){ cv_as_decay = val;});
	initdata->rack->registerParam(initdata->prefix, "a_spy", [&](const int val){ as_a_spy = val;});
	initdata->rack->registerCv(initdata->prefix, "a_spy", [&](const int val){ cv_as_a_spy = val;});

    this->enabled = false;
}

void DrumRackASD::Process(const DrumRackProcessData &data) {
    MK_BOOL_PAR(_trig, as_trigger)
    if (_trig != trig_prev){
        trig_prev = _trig;
    }
    else{
        _trig = false;
    }

    std::fill_n(out, BUF_SZ, 0.f);

    if (!this->enabled) {
        return;
    }

    MK_FLT_PAR_ABS(_accent, as_accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(_f0, as_f0, 4095.f, 0.001f, 0.01f)
    MK_FLT_PAR_ABS(_tone, as_tone, 4095.f, 1.f)
    MK_FLT_PAR_ABS(_decay, as_decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS(_a_spy, as_a_spy, 4095.f, 1.f)
    asd.Render(
        false,
        _trig,
        _accent,
        _f0,
        _tone,
        _decay,
        _a_spy,
        out,
        BUF_SZ);
    
    if (out[0] != out[0]) {
        printf("DrumRackASD: NaN detected!\n");
        asd.Init();
    }
}
