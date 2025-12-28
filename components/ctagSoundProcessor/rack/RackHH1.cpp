#include "RackSynth.hpp"
#include "RackHH1.hpp"
#include "../ctagSoundProcessorPicoSeqRack.hpp"

using namespace CTAG::SP;

void RackHH1::Init(const PickSeqRackInitData *initdata) {
    hh1.Init();

    initdata->rack->registerParamAndCC(initdata, "f0", 6, [&](const int val){ hh1_f0 = val;});
    initdata->rack->registerParamAndCC(initdata, "tone", 7, [&](const int val){ hh1_tone = val;});
    initdata->rack->registerParamAndCC(initdata, "decay", 8, [&](const int val){ hh1_decay = val;});
	initdata->rack->registerParamAndCC(initdata, "noise", 9, [&](const int val){ hh1_noise = val;});
    initdata->rack->registerParamAndCC(initdata, "accent", 10, [&](const int val){ hh1_accent = val;});

    this->enabled = false;
}

void RackHH1::trigger() {
    midi_trig = true;
}

void RackHH1::Process(const PicoSeqRackProcessData &data) {
    std::fill_n(out, BUF_SZ, 0.f);

    if (!this->enabled) {
        return;
    }

    bool _trig = midi_trig;
    if (_trig != trig_prev) {
        trig_prev = _trig;
    }
    midi_trig = false;

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
        printf("RackHH1: NaN detected!\n");
        hh1.Init();
    }
}
