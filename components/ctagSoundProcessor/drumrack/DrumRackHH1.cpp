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
    std::fill_n(hh1_out, BUF_SZ, 0.f);

    if (!this->enabled) {
        return;
    }

    MK_BOOL_PAR(bHH1Trig, hh1_trigger)
    if (bHH1Trig != hh1_trig_prev){
        if (bHH1Trig) {
            printf("DrumRackHH1: Trigger.\n");
        }
        hh1_trig_prev = bHH1Trig;
    }

    MK_FLT_PAR_ABS(fHH1Accent, hh1_accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX(fHH1F0, hh1_f0, 4095.f, 0.0005f, 0.1f)
    MK_FLT_PAR_ABS(fHH1Tone, hh1_tone, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fHH1Decay, hh1_decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fHH1Noise, hh1_noise, 4095.f, 1.f)
    hh1.Render(
        false,
        bHH1Trig,
        fHH1Accent,
        fHH1F0,
        fHH1Tone,
        fHH1Decay,
        fHH1Noise,
        temp1_,
        temp2_,
        hh1_out,
        BUF_SZ);
}
