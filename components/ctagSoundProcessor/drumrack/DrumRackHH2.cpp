#include "DrumRackSynth.hpp"
#include "DrumRackHH2.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackHH2::Init(const DrumRackInitData *initdata) {
    hh2.Init();

    initdata->rack->registerParam(initdata->prefix, "trigger", [&](const int val){ hh2_trigger = val;});
	initdata->rack->registerTrig(initdata->prefix, "trigger", [&](const int val){ trig_hh2_trigger = val;});
	initdata->rack->registerParam(initdata->prefix, "accent", [&](const int val){ hh2_accent = val;});
	initdata->rack->registerCv(initdata->prefix, "accent", [&](const int val){ cv_hh2_accent = val;});
	initdata->rack->registerParam(initdata->prefix, "f0", [&](const int val){ hh2_f0 = val;});
	initdata->rack->registerCv(initdata->prefix, "f0", [&](const int val){ cv_hh2_f0 = val;});
	initdata->rack->registerParam(initdata->prefix, "tone", [&](const int val){ hh2_tone = val;});
	initdata->rack->registerCv(initdata->prefix, "tone", [&](const int val){ cv_hh2_tone = val;});
	initdata->rack->registerParam(initdata->prefix, "decay", [&](const int val){ hh2_decay = val;});
	initdata->rack->registerCv(initdata->prefix, "decay", [&](const int val){ cv_hh2_decay = val;});
	initdata->rack->registerParam(initdata->prefix, "noise", [&](const int val){ hh2_noise = val;});
	initdata->rack->registerCv(initdata->prefix, "noise", [&](const int val){ cv_hh2_noise = val;});

    this->enabled = false;
}

void DrumRackHH2::Process(const DrumRackProcessData &data) {
    std::fill_n(hh2_out, BUF_SZ, 0.f);

    MK_BOOL_PAR(bHH2Trig, hh2_trigger)
    if (bHH2Trig != hh2_trig_prev){
        if (bHH2Trig) {
            printf("DrumRackHH2: Trigger.\n");
        }
        hh2_trig_prev = bHH2Trig;
    }

    if (!this->enabled) {
        return;
    }

    MK_FLT_PAR_ABS(fHH2Accent, hh2_accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(fHH2F0, hh2_f0, 4095.f, .00001f, .1f)
    MK_FLT_PAR_ABS(fHH2Tone, hh2_tone, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fHH2Decay, hh2_decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fHH2Noise, hh2_noise, 4095.f, 1.f)
    hh2.Render(
        false,
        bHH2Trig,
        fHH2Accent,
        fHH2F0,
        fHH2Tone,
        fHH2Decay,
        fHH2Noise,
        temp1_,
        temp2_,
        hh2_out,
        BUF_SZ);
}
