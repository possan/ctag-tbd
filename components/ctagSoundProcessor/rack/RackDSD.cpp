#include "RackSynth.hpp"
#include "RackDSD.hpp"
#include "../ctagSoundProcessorPicoSeqRack.hpp"

using namespace CTAG::SP;

void RackDSD::Init(const PickSeqRackInitData *initdata) {
    dsd.Init();

	initdata->rack->registerMacroParamAndCC(initdata, "f0", 8, [&](const int val){ f0 = val;});
    initdata->rack->registerMacroParamAndCC(initdata, "decay", 9, [&](const int val){ decay = val;});
    initdata->rack->registerMacroParamAndCC(initdata, "fm_amt", 10, [&](const int val){ fm_amt = val;});
    initdata->rack->registerMacroParamAndCC(initdata, "spy", 11, [&](const int val){ spy = val;});
	initdata->rack->registerMacroParamAndCC(initdata, "accent", 12, [&](const int val){ accent = val;});
    
    this->enabled = false;
}

void RackDSD::trigger() {
    midi_trig = true;
}

void RackDSD::Process(const PicoSeqRackProcessData &data) {
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
        printf("RackDSD: NaN detected!\n");
        dsd.Init();
    }
}
