#include "DrumRackSynth.hpp"
#include "DrumRackDSD.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

// const float minVolume {0.000001f};
#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackDSD::Init(const DrumRackInitData *initdata) {
    dsd.Init();

	initdata->rack->registerParamAndCC(initdata, "accent", 6, [&](const int val){ accent = val;});
	initdata->rack->registerParamAndCC(initdata, "f0", 7, [&](const int val){ f0 = val;});
    initdata->rack->registerParamAndCC(initdata, "fm_amt", 8, [&](const int val){ fm_amt = val;});
    initdata->rack->registerParamAndCC(initdata, "decay", 9, [&](const int val){ decay = val;});
    initdata->rack->registerParamAndCC(initdata, "spy", 10, [&](const int val){ spy = val;});

    this->enabled = false;
}

void DrumRackDSD::handleMidiNoteOn() {
    midi_trig = true;
    // printf("DSD note on\n");
}

// void DrumRackDSD::handleMidiCC(uint8_t control, uint8_t value) {
//     // TODO: Implement
//     printf("DSD CC %d %d\n", control, value);
// }

void DrumRackDSD::Process(const DrumRackProcessData &data) {
    std::fill_n(out, BUF_SZ, 0.f);

    // MK_BOOL_PAR_NOCV(_trig, trigger)
    bool _trig = false;
    if (midi_trig) {
        _trig = true;
        midi_trig = false;
    }
    if (_trig != trig_prev){
        // if (_trig) {
        //     printf("DSD\n");
        // }
        trig_prev = _trig;
    }

    if (!this->enabled) {
        return;
    }

    MK_FLT_PAR_ABS_NOCV(_accent, accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(_f0, f0, 4095.f, 0.0008f, 0.01f)
    MK_FLT_PAR_ABS_NOCV(_fmAmt, fm_amt, 4095.f, 1.5f)
    MK_FLT_PAR_ABS_NOCV(_decay, decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS_NOCV(_spy, spy, 4095.f, 1.f)
    dsd.Render(
        false,
        _trig,
        _accent,
        _f0,
        _fmAmt,
        _decay,
        _spy,
        out,
        BUF_SZ);

    if (out[0] != out[0]) {
        printf("DrumRackDSD: NaN detected!\n");
        dsd.Init();
    }
}
