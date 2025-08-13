#include "DrumRackSynth.hpp"
#include "DrumRackClap.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

// const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackClap::Init(const DrumRackInitData *initdata) {
    cl.Init();

    initdata->rack->registerParam(initdata->prefix, "cl_trigger", [&](const int val){ cl_trigger = val;});
	initdata->rack->registerTrig(initdata->prefix, "cl_trigger", [&](const int val){ trig_cl_trigger = val;});
	initdata->rack->registerParam(initdata->prefix, "cl_mute", [&](const int val){ cl_mute = val;});
	initdata->rack->registerTrig(initdata->prefix, "cl_mute", [&](const int val){ trig_cl_mute = val;});
	initdata->rack->registerParam(initdata->prefix, "cl_lev", [&](const int val){ cl_lev = val;});
	initdata->rack->registerCv(initdata->prefix, "cl_lev", [&](const int val){ cv_cl_lev = val;});
	initdata->rack->registerParam(initdata->prefix, "cl_pan", [&](const int val){ cl_pan = val;});
	initdata->rack->registerCv(initdata->prefix, "cl_pan", [&](const int val){ cv_cl_pan = val;});
	initdata->rack->registerParam(initdata->prefix, "cl_fx1", [&](const int val){ cl_fx1 = val;});
	initdata->rack->registerCv(initdata->prefix, "cl_fx1", [&](const int val){ cv_cl_fx1 = val;});
	initdata->rack->registerParam(initdata->prefix, "cl_fx2", [&](const int val){ cl_fx2 = val;});
	initdata->rack->registerCv(initdata->prefix, "cl_fx2", [&](const int val){ cv_cl_fx2 = val;});
	initdata->rack->registerParam(initdata->prefix, "cl_f0", [&](const int val){ cl_f0 = val;});
	initdata->rack->registerCv(initdata->prefix, "cl_f0", [&](const int val){ cv_cl_f0 = val;});
	initdata->rack->registerParam(initdata->prefix, "cl_tone", [&](const int val){ cl_tone = val;});
	initdata->rack->registerCv(initdata->prefix, "cl_tone", [&](const int val){ cv_cl_tone = val;});
	initdata->rack->registerParam(initdata->prefix, "cl_decay", [&](const int val){ cl_decay = val;});
	initdata->rack->registerCv(initdata->prefix, "cl_decay", [&](const int val){ cv_cl_decay = val;});
	initdata->rack->registerParam(initdata->prefix, "cl_scale", [&](const int val){ cl_scale = val;});
	initdata->rack->registerCv(initdata->prefix, "cl_scale", [&](const int val){ cv_cl_scale = val;});
	initdata->rack->registerParam(initdata->prefix, "cl_transient", [&](const int val){ cl_transient = val;});
	initdata->rack->registerCv(initdata->prefix, "cl_transient", [&](const int val){ cv_cl_transient = val;});

}

// void DrumRackClap::SetParamValue(const string &id, const string &key, const int val) {
//     // Implementation of setting parameter value
//     // This is where you would handle the parameter setting logic
//     // For example, you might store the value in a map or update an internal state
//     // ESP_LOGI("DrumRackClap", "Setting parameter %s with key %s to value %d", id.c_str(), key.c_str(), val);
//     // You can add your specific logic here
// }

void DrumRackClap::Process(const DrumRackProcessData &data) {
	  MK_BOOL_PAR(bCLMute, cl_mute)

    MK_FLT_PAR_ABS_PAN(fCLPan, cl_pan, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fCLLev, cl_lev, 4095.f, 2.f); fCLLev *= fCLLev;
    MK_FLT_PAR_ABS(fCLFX1Send, cl_fx1, 4095.f, maxFXSendLevelDly); fCLFX1Send *= fCLFX1Send;
    MK_FLT_PAR_ABS(fCLFX2Send, cl_fx2, 4095.f, maxFXSendLevelRev); fCLFX2Send *= fCLFX2Send;

    if (bCLMute || fCLLev < minVolume) {
        // return;
    }

    MK_FLT_PAR_ABS_MIN_MAX(cl_pitch1_, cl_f0, 4095.f, 350.f, 4000.f)
    MK_FLT_PAR_ABS_MIN_MAX(cl_pitch2_, cl_f0, 4095.f, 300.f, 3000.f)
    MK_FLT_PAR_ABS_MIN_MAX(cl_reso1_, cl_tone, 4095.f, 1.f, 2.5f)
    MK_FLT_PAR_ABS_MIN_MAX(cl_reso2_, cl_tone, 4095.f, 0.75f, 6.5f)
    MK_FLT_PAR_ABS_MIN_MAX(cl_decay1_, cl_decay, 4095.f, 0.05f, 0.3f)
    MK_FLT_PAR_ABS_MIN_MAX(cl_decay2_, cl_decay, 4095.f, 0.05f, 2.f)
    MK_FLT_PAR_ABS_MIN_MAX(cl_scale_attack_, cl_scale, 4095.f, 0.f, 0.1f)
    MK_FLT_PAR_ABS_MIN_MAX(cl_scale_trans, cl_scale, 4095.f, 1.f, 3.f)
    MK_INT_PAR_ABS(cl_trans_, cl_transient, 16)

    cl.params.pitch1 = cl_pitch1_ / 44100.f;
    cl.params.pitch2 = cl_pitch2_ / 44100.f;
    cl.params.reso1 = cl_reso1_;
    cl.params.reso2 = cl_reso2_;
    cl.params.decay1 = cl_decay1_;
    cl.params.decay2 = cl_decay2_;
    cl.params.attack = cl_scale_attack_;
    cl.params.scale = cl_scale_trans;
    cl.params.transient = cl_trans_ % 16;

    MK_BOOL_PAR(bCLTrig, cl_trigger)
    if (bCLTrig != cl_trig_prev && bCLTrig){
        cl_trig_prev = true;
        cl.Trigger();
    }
    else if (!bCLTrig){
        cl_trig_prev = false;
    }

    cl.Process(cl_out, 32);
    // mixRenderOutputMono(cl_out, fCLLev, fCLPan, fCLFX1Send, fCLFX2Send);

    // mixRenderOutputMono(td3_out, fTD3Lev, fTD3Pan, fTD3FX1Send, fTD3FX2Send);

}
