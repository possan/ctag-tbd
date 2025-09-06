#include "DrumRackSynth.hpp"
#include "DrumRackHH1.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackHH1::Init(const DrumRackInitData *initdata) {
    hh1.Init();

    initdata->rack->registerParamAndCC(initdata, "accent", 6, [&](const int val){ hh1_accent = val;});
    initdata->rack->registerParamAndCC(initdata, "f0", 7, [&](const int val){ hh1_f0 = val;});
    initdata->rack->registerParamAndCC(initdata, "tone", 8, [&](const int val){ hh1_tone = val;});
    initdata->rack->registerParamAndCC(initdata, "decay", 9, [&](const int val){ hh1_decay = val;});
	initdata->rack->registerParamAndCC(initdata, "noise", 10, [&](const int val){ hh1_noise = val;});

    this->enabled = false;
}

void DrumRackHH1::handleMidiNoteOn() {
    midi_trig = true;
    // printf("HH1 note on\n");
}

// void DrumRackHH1::handleMidiCC(uint8_t control, uint8_t value) {
//     // TODO: Implement
//     printf("HH1 CC %d %d\n", control, value);
// }

void DrumRackHH1::Process(const DrumRackProcessData &data) {
    std::fill_n(out, BUF_SZ, 0.f);

    if (!this->enabled) {
        return;
    }

    // MK_BOOL_PAR_NOCV(_trig, hh1_trigger)
    bool _trig = false;
    if (_trig != trig_prev) {
        if (_trig) {
            // printf("HH1\n");
        }
        trig_prev = _trig;
    }

    MK_FLT_PAR_ABS_NOCV(_accent, hh1_accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(_f0, hh1_f0, 4095.f, 0.0005f, 0.1f)
    MK_FLT_PAR_ABS_NOCV(_tone, hh1_tone, 4095.f, 1.f)
    MK_FLT_PAR_ABS_NOCV(_decay, hh1_decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS_NOCV(_noise, hh1_noise, 4095.f, 1.f)
    hh1.Render(
        false,
        _trig,
        _accent,
        _f0,
        _tone,
        _decay,
        _noise,
        temp1,
        temp2,
        out,
        BUF_SZ);

    if (out[0] != out[0]) {
        printf("DrumRackHH1: NaN detected!\n");
        hh1.Init();
    }
}
