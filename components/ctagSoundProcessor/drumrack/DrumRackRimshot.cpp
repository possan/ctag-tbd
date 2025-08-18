#include "DrumRackSynth.hpp"
#include "DrumRackRimshot.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

// const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackRimshot::Init(const DrumRackInitData *initdata) {
    rs.Init();

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

void DrumRackRimshot::Process(const DrumRackProcessData &data) {
    std::fill_n(rs_out, BUF_SZ, 0.f);

    if (!this->enabled) {
        return;
    }

    MK_FLT_PAR_ABS_MIN_MAX(_f0_, f0, 4095.f, 70.f, 350.f)
    MK_FLT_PAR_ABS_MIN_MAX(_decay, decay, 4095.f, .1f, .75f)
    MK_FLT_PAR_ABS_MIN_MAX(_noise, noise, 4095.f, 0.f, .2f)
    MK_FLT_PAR_ABS_MIN_MAX(_accent, accent, 4095.f, 0.1f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(_base, tone, 4095.f, .35f, .65f)
    MK_FLT_PAR_ABS_MIN_MAX(_reso_hp, tone, 4095.f, 5.f, 1.f)

    rs.params.f0 = _f0_ / 44100.f;
    rs.params.decay = _decay;
    rs.params.accent = _accent;
    rs.params.reso_hp = _reso_hp;
    rs.params.base = _base;
    rs.params.noise_level = _noise;

    MK_BOOL_PAR(_trig, trigger)
    if (_trig != trig_prev) {
        if (_trig) {
            printf("RS\n");
            rs.Trigger();
        }
        trig_prev = _trig;
    }

    rs.Process(rs_out, BUF_SZ);

    if (rs_out[0] != rs_out[0]) {
        printf("DrumRackRimshot: NaN detected!\n");
        rs.Init();
    }
}
