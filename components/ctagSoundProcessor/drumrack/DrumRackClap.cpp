#include "DrumRackSynth.hpp"
#include "DrumRackClap.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

// const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackClap::Init(const DrumRackInitData *initdata) {
    cl.Init();

    initdata->rack->registerParam(initdata->prefix, "trigger", [&](const int val){ trigger = val;});
	initdata->rack->registerTrig(initdata->prefix, "trigger", [&](const int val){ trig_trigger = val;});
	initdata->rack->registerParam(initdata->prefix, "f0", [&](const int val){ f0 = val;});
	initdata->rack->registerCv(initdata->prefix, "f0", [&](const int val){ cv_f0 = val;});
	initdata->rack->registerParam(initdata->prefix, "tone", [&](const int val){ tone = val;});
	initdata->rack->registerCv(initdata->prefix, "tone", [&](const int val){ cv_tone = val;});
	initdata->rack->registerParam(initdata->prefix, "decay", [&](const int val){ decay = val;});
	initdata->rack->registerCv(initdata->prefix, "decay", [&](const int val){ cv_decay = val;});
	initdata->rack->registerParam(initdata->prefix, "scale", [&](const int val){ scale = val;});
	initdata->rack->registerCv(initdata->prefix, "scale", [&](const int val){ cv_scale = val;});
	initdata->rack->registerParam(initdata->prefix, "transient", [&](const int val){ transient = val;});
	initdata->rack->registerCv(initdata->prefix, "transient", [&](const int val){ cv_transient = val;});

    this->enabled = false;
}

void DrumRackClap::Process(const DrumRackProcessData &data) {
    std::fill_n(out, BUF_SZ, 0.f);

    if (!this->enabled) {
        return;
    }

    MK_FLT_PAR_ABS_MIN_MAX(_pitch1_, f0, 4095.f, 350.f, 4000.f)
    MK_FLT_PAR_ABS_MIN_MAX(_pitch2_, f0, 4095.f, 300.f, 3000.f)
    MK_FLT_PAR_ABS_MIN_MAX(_reso1_, tone, 4095.f, 1.f, 2.5f)
    MK_FLT_PAR_ABS_MIN_MAX(_reso2_, tone, 4095.f, 0.75f, 6.5f)
    MK_FLT_PAR_ABS_MIN_MAX(_decay1_, decay, 4095.f, 0.05f, 0.3f)
    MK_FLT_PAR_ABS_MIN_MAX(_decay2_, decay, 4095.f, 0.05f, 2.f)
    MK_FLT_PAR_ABS_MIN_MAX(_scale_attack_, scale, 4095.f, 0.f, 0.1f)
    MK_FLT_PAR_ABS_MIN_MAX(_scale_trans, scale, 4095.f, 1.f, 3.f)
    MK_INT_PAR_ABS(_trans_, transient, 16)

    cl.params.pitch1 = _pitch1_ / 44100.f;
    cl.params.pitch2 = _pitch2_ / 44100.f;
    cl.params.reso1 = _reso1_;
    cl.params.reso2 = _reso2_;
    cl.params.decay1 = _decay1_;
    cl.params.decay2 = _decay2_;
    cl.params.attack = _scale_attack_;
    cl.params.scale = _scale_trans;
    cl.params.transient = _trans_ % 16;

    MK_BOOL_PAR(_trig, trigger)
    if (_trig != trig_prev){
        if (_trig) {
            printf("CL\n");
            cl.Trigger();
        }
        trig_prev = _trig;
    }

    cl.Process(out, BUF_SZ);

    if (out[0] != out[0]) {
        printf("DrumRackCL: NaN detected!\n");
        cl.Init();
    }
}
