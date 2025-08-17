#include "DrumRackSynth.hpp"
#include "DrumRackABD.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackABD::Init(const DrumRackInitData *initdata) {
    abd.Init();

    initdata->rack->registerParam(initdata->prefix, "trigger", [&](const int val){ ab_trigger = val;});
	initdata->rack->registerTrig(initdata->prefix, "trigger", [&](const int val){ trig_ab_trigger = val;});
    
	initdata->rack->registerParam(initdata->prefix, "f0", [&](const int val){ ab_f0 = val;});
	initdata->rack->registerCv(initdata->prefix, "f0", [&](const int val){ cv_ab_f0 = val;});

    initdata->rack->registerParam(initdata->prefix, "accent", [&](const int val){ ab_accent = val;});
	initdata->rack->registerCv(initdata->prefix, "accent", [&](const int val){ cv_ab_accent = val;});
	initdata->rack->registerParam(initdata->prefix, "tone", [&](const int val){ ab_tone = val;});
	initdata->rack->registerCv(initdata->prefix, "tone", [&](const int val){ cv_ab_tone = val;});
	initdata->rack->registerParam(initdata->prefix, "decay", [&](const int val){ ab_decay = val;});
	initdata->rack->registerCv(initdata->prefix, "decay", [&](const int val){ cv_ab_decay = val;});
	initdata->rack->registerParam(initdata->prefix, "a_fm", [&](const int val){ ab_a_fm = val;});
	initdata->rack->registerCv(initdata->prefix, "a_fm", [&](const int val){ cv_ab_a_fm = val;});
	initdata->rack->registerParam(initdata->prefix, "s_fm", [&](const int val){ ab_s_fm = val;});
	initdata->rack->registerCv(initdata->prefix, "s_fm", [&](const int val){ cv_ab_s_fm = val;});

    this->enabled = false;
}

void DrumRackABD::Process(const DrumRackProcessData &data) {
    MK_BOOL_PAR(_trig, ab_trigger)
    if (_trig != trig_prev){
        if (_trig) {
            // printf("DrumRackABD: Trigger.\n");
        }
        trig_prev = _trig;
    }

    std::fill_n(out, BUF_SZ, 0.f);

    if (!this->enabled) {
        return;
    }

    MK_FLT_PAR_ABS(_accent, ab_accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(_f0, ab_f0, 4095.f, 0.0001f, 0.01f)
    MK_FLT_PAR_ABS(_tone, ab_tone, 4095.f, 1.f)
    MK_FLT_PAR_ABS(_decay, ab_decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(_a_fm, ab_a_fm, 4095.f, 0.f, 100.f)
    MK_FLT_PAR_ABS_MIN_MAX(_s_fm, ab_s_fm, 4095.f, 0.f, 100.f)
    abd.Render(
        false,
        _trig,
        _accent,
        _f0,
        _tone,
        _decay,
        _a_fm,
        _s_fm,
        out,
        BUF_SZ);

    if (out[0] != out[0]) {
        printf("DrumRackABD: NaN detected!\n");
        abd.Init();
    }
}
