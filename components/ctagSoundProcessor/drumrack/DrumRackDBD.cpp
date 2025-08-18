#include "DrumRackSynth.hpp"
#include "DrumRackDBD.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

// const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackDBD::Init(const DrumRackInitData *initdata) {
    dbd.Init();

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
	initdata->rack->registerParam(initdata->prefix, "dirty", [&](const int val){ dirty = val;});
	initdata->rack->registerCv(initdata->prefix, "dirty", [&](const int val){ cv_dirty = val;});
	initdata->rack->registerParam(initdata->prefix, "fm_env", [&](const int val){ fm_env = val;});
	initdata->rack->registerCv(initdata->prefix, "fm_env", [&](const int val){ cv_fm_env = val;});
	initdata->rack->registerParam(initdata->prefix, "fm_dcy", [&](const int val){ fm_dcy = val;});
	initdata->rack->registerCv(initdata->prefix, "fm_dcy", [&](const int val){ cv_fm_dcy = val;});

    this->enabled = false;
}

void DrumRackDBD::Process(const DrumRackProcessData &data) {
    std::fill_n(out, BUF_SZ, 0.f);

    MK_BOOL_PAR(_trig, trigger)
    if (_trig != trig_prev){
        if (_trig) {
            printf("DBD\n");
        }
        trig_prev = _trig;
    }

    if (!this->enabled) {
        return;
    }

    MK_FLT_PAR_ABS(_accent, accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(_f0, f0, 4095.f, 0.0005f, 0.01f)
    MK_FLT_PAR_ABS(_tone, tone, 4095.f, 1.f)
    MK_FLT_PAR_ABS(_decay, decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS(_dirty, dirty, 4095.f, 5.f)
    MK_FLT_PAR_ABS(_fmEnv, fm_env, 4095.f, 5.f)
    MK_FLT_PAR_ABS(_fmDcy, fm_dcy, 4095.f, 4.f)
    dbd.Render(
        false,
        _trig,
        _accent,
        _f0,
        _tone,
        _decay,
        _dirty,
        _fmEnv,
        _fmDcy,
        out,
        BUF_SZ);

    if (out[0] != out[0]) {
        printf("DrumRackDBD: NaN detected!\n");
        dbd.Init();
    }
}
