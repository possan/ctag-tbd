#include "RackSynth.hpp"
#include "RackHH2.hpp"
#include "../ctagSoundProcessorPicoSeqRack.hpp"

using namespace CTAG::SP;

void RackHH2::Init(const PickSeqRackInitData *initdata) {
    hh2.Init();

    initdata->rack->registerParamAndCC(initdata, "f0", 6, [&](const int val){ f0 = val;});
    initdata->rack->registerParamAndCC(initdata, "tone", 7, [&](const int val){ tone = val;});
    initdata->rack->registerParamAndCC(initdata, "decay", 8, [&](const int val){ decay = val;});
    initdata->rack->registerParamAndCC(initdata, "noise", 9, [&](const int val){ noise = val;});
    initdata->rack->registerParamAndCC(initdata, "accent", 10, [&](const int val){ accent = val;});
    
    this->enabled = false;
}

void RackHH2::trigger() {
    midi_trig = true;
}

void RackHH2::Process(const PicoSeqRackProcessData &data) {
    std::fill_n(out, BUF_SZ, 0.f);

    // MK_BOOL_PAR_NOCV(_trig, trigger)
    bool _trig = midi_trig;
    if (_trig != trig_prev) {
        if (_trig) {
            printf("HH2\n");
        }
        trig_prev = _trig;
    }
    midi_trig = false;

    if (!this->enabled) {
        return;
    }

    MK_FLT_PAR_ABS_NOCV(_accent, accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(_f0, f0, 4095.f, .00001f, .1f)
    MK_FLT_PAR_ABS_NOCV(_tone, tone, 4095.f, 1.f)
    MK_FLT_PAR_ABS_NOCV(_decay, decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS_NOCV(_noise, noise, 4095.f, 1.f)
    hh2.Render(
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
        printf("RackHH2: NaN detected!\n");
        hh2.Init();
    }
}
