#include "RackSynth.hpp"
#include "RackASD.hpp"
#include "../ctagSoundProcessorPicoSeqRack.hpp"

using namespace CTAG::SP;

void RackASD::Init(const PickSeqRackInitData *initdata) {
    asd.Init();

    initdata->rack->registerMacroParamAndCC(initdata, "f0", 8, [&](const int val){ f0 = val;});
    initdata->rack->registerMacroParamAndCC(initdata, "tone", 9, [&](const int val){ tone = val;});
    initdata->rack->registerMacroParamAndCC(initdata, "decay", 10, [&](const int val){ decay = val;});
    initdata->rack->registerMacroParamAndCC(initdata, "a_spy", 11, [&](const int val){ a_spy = val;});
    initdata->rack->registerMacroParamAndCC(initdata, "accent", 12, [&](const int val){ accent = val;});

    this->enabled = false;
}

void RackASD::trigger() {
    this->midi_trig = true;
}

void RackASD::Process(const PicoSeqRackProcessData &data) {
    // MK_BOOL_PAR_NOCV(_trig, trigger)
    bool _trig = false;
    if (this->midi_trig) {
        _trig = true;
        midi_trig = false;
    }
    if (_trig != trig_prev){
        // if (_trig) {
            // printf("ASD\n");
        // }
        trig_prev = _trig;
    }

    std::fill_n(out, BUF_SZ, 0.f);

    if (!this->enabled) {
        return;
    }

    MK_FLT_PAR_ABS_NOCV(_accent, accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(_f0, f0, 4095.f, 0.001f, 0.01f)
    MK_FLT_PAR_ABS_NOCV(_tone, tone, 4095.f, 1.f)
    MK_FLT_PAR_ABS_NOCV(_decay, decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS_NOCV(_a_spy, a_spy, 4095.f, 1.f)
    asd.Render(
        false,
        _trig,
        _accent,
        _f0,
        _tone,
        _decay,
        _a_spy,
        out,
        BUF_SZ);

    if (out[0] != out[0]) {
        printf("RackASD: NaN detected!\n");
        asd.Init();
    }
}
