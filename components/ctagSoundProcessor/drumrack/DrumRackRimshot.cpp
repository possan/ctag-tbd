#include "DrumRackSynth.hpp"
#include "DrumRackRimshot.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

// const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackRimshot::Init(const DrumRackInitData *initdata) {
    rs.Init();

    initdata->rack->registerParam(initdata->prefix, "trigger", [&](const int val){ rs_trigger = val;});
	initdata->rack->registerTrig(initdata->prefix, "trigger", [&](const int val){ trig_rs_trigger = val;});
	initdata->rack->registerParam(initdata->prefix, "accent", [&](const int val){ rs_accent = val;});
	initdata->rack->registerCv(initdata->prefix, "accent", [&](const int val){ cv_rs_accent = val;});
	initdata->rack->registerParam(initdata->prefix, "f0", [&](const int val){ rs_f0 = val;});
	initdata->rack->registerCv(initdata->prefix, "f0", [&](const int val){ cv_rs_f0 = val;});
	initdata->rack->registerParam(initdata->prefix, "tone", [&](const int val){ rs_tone = val;});
	initdata->rack->registerCv(initdata->prefix, "tone", [&](const int val){ cv_rs_tone = val;});
	initdata->rack->registerParam(initdata->prefix, "decay", [&](const int val){ rs_decay = val;});
	initdata->rack->registerCv(initdata->prefix, "decay", [&](const int val){ cv_rs_decay = val;});
	initdata->rack->registerParam(initdata->prefix, "noise", [&](const int val){ rs_noise = val;});
	initdata->rack->registerCv(initdata->prefix, "noise", [&](const int val){ cv_rs_noise = val;});

    this->enabled = false;
}

void DrumRackRimshot::Process(const DrumRackProcessData &data) {
    std::fill_n(rs_out, BUF_SZ, 0.f);

    if (!this->enabled) {
        return;
    }

    MK_FLT_PAR_ABS_MIN_MAX(rs_f0_, rs_f0, 4095.f, 70.f, 350.f)
    MK_FLT_PAR_ABS_MIN_MAX(rs_decay_, rs_decay, 4095.f, .1f, .75f)
    MK_FLT_PAR_ABS_MIN_MAX(rs_noise_, rs_noise, 4095.f, 0.f, .2f)
    MK_FLT_PAR_ABS_MIN_MAX(rs_accent_, rs_accent, 4095.f, 0.1f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(rs_base_, rs_tone, 4095.f, .35f, .65f)
    MK_FLT_PAR_ABS_MIN_MAX(rs_reso_hp_, rs_tone, 4095.f, 5.f, 1.f)

    rs.params.f0 = rs_f0_ / 44100.f;
    rs.params.decay = rs_decay_;
    rs.params.accent = rs_accent_;
    rs.params.reso_hp = rs_reso_hp_;
    rs.params.base = rs_base_;
    rs.params.noise_level = rs_noise_;

    MK_BOOL_PAR(bRSTrig, rs_trigger)
    if (bRSTrig != rs_trig_prev && bRSTrig) {
        rs.Trigger();
    }
    rs_trig_prev = bRSTrig;

    rs.Process(rs_out, BUF_SZ);

    if (rs_out[0] != rs_out[0]) {
        printf("DrumRackRimshot: NaN detected!\n");
        rs.Init();
    }
}
