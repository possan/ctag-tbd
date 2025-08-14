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
	initdata->rack->registerParam(initdata->prefix, "accent", [&](const int val){ ab_accent = val;});
	initdata->rack->registerCv(initdata->prefix, "accent", [&](const int val){ cv_ab_accent = val;});
	initdata->rack->registerParam(initdata->prefix, "f0", [&](const int val){ ab_f0 = val;});
	initdata->rack->registerCv(initdata->prefix, "f0", [&](const int val){ cv_ab_f0 = val;});
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
    MK_BOOL_PAR(bABTrig, ab_trigger)
    if (bABTrig != abd_trig_prev){
        abd_trig_prev = bABTrig;
    }
    else{
        bABTrig = false;
    }

    if (!this->enabled) {
        return;
    }

    MK_FLT_PAR_ABS(fABAccent, ab_accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(fABF0, ab_f0, 4095.f, 0.0001f, 0.01f)
    MK_FLT_PAR_ABS(fABTone, ab_tone, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fABDecay, ab_decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(fABAfm, ab_a_fm, 4095.f, 0.f, 100.f)
    MK_FLT_PAR_ABS_MIN_MAX(fABSfm, ab_s_fm, 4095.f, 0.f, 100.f)
    abd.Render(
        false,
        bABTrig,
        fABAccent,
        fABF0,
        fABTone,
        fABDecay,
        fABAfm,
        fABSfm,
        abd_out,
        32);
}
