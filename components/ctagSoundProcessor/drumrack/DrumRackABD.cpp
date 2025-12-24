#include "DrumRackSynth.hpp"
#include "DrumRackABD.hpp"
#include "../ctagSoundProcessorPicoSeqRack.hpp"

using namespace CTAG::SP;

#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackABD::Init(const PickSeqRackInitData *initdata) {
    abd.Init();

    initdata->rack->registerParamAndCC(initdata, "f0", 6, [&](const int val){ f0 = val;});
    initdata->rack->registerParamAndCC(initdata, "tone", 7, [&](const int val){ tone = val;});
	initdata->rack->registerParamAndCC(initdata, "decay", 8, [&](const int val){ decay = val;});
    initdata->rack->registerParamAndCC(initdata, "a_fm", 9, [&](const int val){ a_fm = val;});
    initdata->rack->registerParamAndCC(initdata, "s_fm", 10, [&](const int val){ s_fm = val;});
    initdata->rack->registerParamAndCC(initdata, "accent", 11, [&](const int val){ accent = val;});
    
    this->enabled = false;
}

void DrumRackABD::handleMidiNoteOn() {
    midi_trig = true;
    // printf("ABD note on\n");
}

// void DrumRackABD::handleMidiCC(uint8_t control, uint8_t value) {
//     // TODO: Implement
//     printf("ABD CC %d %d\n", control, value);
// }

void DrumRackABD::Process(const PicoSeqRackProcessData &data) {
    // MK_BOOL_PAR_NOCV(_trig, trigger)
    bool _trig = false;
    if (midi_trig) {
        _trig = true;
        midi_trig = false;
    }
    if (_trig != trig_prev){
        // if (_trig) {
            // printf("ABD\n");
        // }
        trig_prev = _trig;
    }

    std::fill_n(out, BUF_SZ, 0.f);

    if (!this->enabled) {
        return;
    }

    MK_FLT_PAR_ABS_NOCV(_accent, accent, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(_f0, f0, 4095.f, 0.0001f, 0.01f)
    MK_FLT_PAR_ABS_NOCV(_tone, tone, 4095.f, 1.f)
    MK_FLT_PAR_ABS_NOCV(_decay, decay, 4095.f, 1.f)
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(_a_fm, a_fm, 4095.f, 0.f, 100.f)
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(_s_fm, s_fm, 4095.f, 0.f, 100.f)
    abd.Render(
        false,
        _trig,
        _accent,
        _f0,
        _tone,
        _decay,
        _a_fm,
        _s_fm,
        out,
        BUF_SZ);

    if (out[0] != out[0]) {
        printf("DrumRackABD: NaN detected!\n");
        abd.Init();
    }
}
