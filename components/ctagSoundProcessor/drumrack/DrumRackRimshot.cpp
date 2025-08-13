#include "DrumRackSynth.hpp"
#include "DrumRackRimshot.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

// const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackRimshot::Init(const DrumRackInitData *initdata) {
    rs.Init();

    initdata->rack->registerParam(initdata->prefix, "rs_trigger", [&](const int val){ rs_trigger = val;});
	initdata->rack->registerTrig(initdata->prefix, "rs_trigger", [&](const int val){ trig_rs_trigger = val;});
	initdata->rack->registerParam(initdata->prefix, "rs_mute", [&](const int val){ rs_mute = val;});
	initdata->rack->registerTrig(initdata->prefix, "rs_mute", [&](const int val){ trig_rs_mute = val;});
	initdata->rack->registerParam(initdata->prefix, "rs_lev", [&](const int val){ rs_lev = val;});
	initdata->rack->registerCv(initdata->prefix, "rs_lev", [&](const int val){ cv_rs_lev = val;});
	initdata->rack->registerParam(initdata->prefix, "rs_pan", [&](const int val){ rs_pan = val;});
	initdata->rack->registerCv(initdata->prefix, "rs_pan", [&](const int val){ cv_rs_pan = val;});
	initdata->rack->registerParam(initdata->prefix, "rs_fx1", [&](const int val){ rs_fx1 = val;});
	initdata->rack->registerCv(initdata->prefix, "rs_fx1", [&](const int val){ cv_rs_fx1 = val;});
	initdata->rack->registerParam(initdata->prefix, "rs_fx2", [&](const int val){ rs_fx2 = val;});
	initdata->rack->registerCv(initdata->prefix, "rs_fx2", [&](const int val){ cv_rs_fx2 = val;});
	initdata->rack->registerParam(initdata->prefix, "rs_accent", [&](const int val){ rs_accent = val;});
	initdata->rack->registerCv(initdata->prefix, "rs_accent", [&](const int val){ cv_rs_accent = val;});
	initdata->rack->registerParam(initdata->prefix, "rs_f0", [&](const int val){ rs_f0 = val;});
	initdata->rack->registerCv(initdata->prefix, "rs_f0", [&](const int val){ cv_rs_f0 = val;});
	initdata->rack->registerParam(initdata->prefix, "rs_tone", [&](const int val){ rs_tone = val;});
	initdata->rack->registerCv(initdata->prefix, "rs_tone", [&](const int val){ cv_rs_tone = val;});
	initdata->rack->registerParam(initdata->prefix, "rs_decay", [&](const int val){ rs_decay = val;});
	initdata->rack->registerCv(initdata->prefix, "rs_decay", [&](const int val){ cv_rs_decay = val;});
	initdata->rack->registerParam(initdata->prefix, "rs_noise", [&](const int val){ rs_noise = val;});
	initdata->rack->registerCv(initdata->prefix, "rs_noise", [&](const int val){ cv_rs_noise = val;});

}

// void DrumRackRimshot::SetParamValue(const string &id, const string &key, const int val) {
//     // Implementation of setting parameter value
//     // This is where you would handle the parameter setting logic
//     // For example, you might store the value in a map or update an internal state
//     // ESP_LOGI("DrumRackRimshot", "Setting parameter %s with key %s to value %d", id.c_str(), key.c_str(), val);
//     // You can add your specific logic here
// }

void DrumRackRimshot::Process(const DrumRackProcessData &data) {
	  MK_BOOL_PAR(bRSMute, rs_mute)

    MK_FLT_PAR_ABS_PAN(fRSPan, rs_pan, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fRSLev, rs_lev, 4095.f, 2.f); fRSLev *= fRSLev;
    MK_FLT_PAR_ABS(fRSFX1Send, rs_fx1, 4095.f, maxFXSendLevelDly); fRSFX1Send *= fRSFX1Send;
    MK_FLT_PAR_ABS(fRSFX2Send, rs_fx2, 4095.f, maxFXSendLevelRev); fRSFX2Send *= fRSFX2Send;

    if (bRSMute || fRSLev < minVolume) {
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

    rs.Process(rs_out, 32);
    // mixRenderOutputMono(rs_out, fRSLev, fRSPan, fRSFX1Send, fRSFX2Send);

    // mixRenderOutputMono(td3_out, fTD3Lev, fTD3Pan, fTD3FX1Send, fTD3FX2Send);

}
