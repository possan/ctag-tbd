#include "DrumRackSynth.hpp"
#include "DrumRackFMB.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

// const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackFMB::Init(const DrumRackInitData *initdata) {
    fmb.Init();

    initdata->rack->registerParamAndCC(initdata, "use_ratio_mode", 6, [&](const int val){ use_ratio_mode = val;});
	initdata->rack->registerParamAndCC(initdata, "mod_env_sync", 7, [&](const int val){ mod_env_sync = val;});
    initdata->rack->registerParamAndCC(initdata, "f_b", 8, [&](const int val){ f_b = val;});
    initdata->rack->registerParamAndCC(initdata, "d_b", 9, [&](const int val){ d_b = val;});
    initdata->rack->registerParamAndCC(initdata, "f_m", 10, [&](const int val){ f_m = val;});
    initdata->rack->registerParamAndCC(initdata, "I", 11, [&](const int val){ I = val;});
    initdata->rack->registerParamAndCC(initdata, "d_m", 12, [&](const int val){ d_m = val;});
    initdata->rack->registerParamAndCC(initdata, "b_m", 13, [&](const int val){ b_m = val;});
    initdata->rack->registerParamAndCC(initdata, "A_f", 14, [&](const int val){ A_f = val;});
    initdata->rack->registerParamAndCC(initdata, "d_f", 15, [&](const int val){ d_f = val;});

    this->enabled = false;
}

void DrumRackFMB::handleMidiNoteOn() {
    midi_trig = true;
    // printf("FMB note on\n");
}

// void DrumRackFMB::handleMidiCC(uint8_t control, uint8_t value) {
//     // TODO: Implement
//     printf("FMB CC %d %d\n", control, value);
// }

void DrumRackFMB::Process(const DrumRackProcessData &data) {
    std::fill_n(out, BUF_SZ, 0.f);

    // MK_BOOL_PAR_NOCV(_trig, trigger)
    bool _trig = false;
    if (midi_trig) {
        _trig = true;
        midi_trig = false;
    }
    if (_trig != trig_prev) {
        if (_trig) {
            // printf("FMB\n");
            fmb.Trigger();
        }
        trig_prev = _trig;
    }

    if (!this->enabled) {
        return;
    }

    MK_BOOL_PAR_NOCV(_use_ratio_mode, use_ratio_mode)
    MK_BOOL_PAR_NOCV(_mod_env_sync, mod_env_sync)
    float _f0 = f_b/4095.f * (200.f-20.f)+20.f;
    if(cv_f_b != -1){
        float fMod = data.cv[cv_f_b] * 5.f;
        fMod = CTAG::SP::HELPERS::fastpow2(fMod);
        _f0 *= fMod;
    }
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(_d_b, d_b, 4095.f, 0.001f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(_f_m, f_m, 4095.f, 40.f, 2000.f)
    MK_FLT_PAR_ABS_NOCV(_modindex, f_m, 4095.f, 63.f)
    int iModIndex = static_cast<int>(_modindex);
    CONSTRAIN(iModIndex, 0, 63)
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(_I, I, 4095.f, 0.f, 10.f)
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(_d_m, d_m, 4095.f, 0.001f, .5f)
    MK_INT_PAR_NOCV(iModFeedback, b_m, 16.f)
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(_A_f, A_f, 4095.f, 0.f, 1000.f)
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(_d_f, d_f, 4095.f, 0.001f, .1f)

    fmb.params.use_ratio_mode = _use_ratio_mode;
    fmb.params.mod_env_sync = _mod_env_sync;
    fmb.params.f_b = _f0;
    fmb.params.d_b = _d_b;
    fmb.params.f_m = _f_m;
    fmb.params.mod_ratio_index = iModIndex;
    fmb.params.I = _I;
    fmb.params.d_m = _d_m;
    fmb.params.b_m = static_cast<float>(iModFeedback);
    fmb.params.A_f = _A_f;
    fmb.params.d_f = _d_f;

    fmb.Process(out, BUF_SZ);
    if (out[0] != out[0]) {
        printf("DrumRackCL: NaN detected!\n");
        fmb.Init();
    }
}
