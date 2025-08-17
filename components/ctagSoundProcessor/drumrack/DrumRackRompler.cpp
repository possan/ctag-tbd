#include "DrumRackSynth.hpp"
#include "DrumRackRompler.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

void DrumRackRompler::Init(const DrumRackInitData *initdata) {
    rompler.Init(44100.f);

    initdata->rack->registerParam(initdata->prefix, "gate", [&](const int val){ s1_gate = val;});
	initdata->rack->registerTrig(initdata->prefix, "gate", [&](const int val){ trig_s1_gate = val;});
	initdata->rack->registerParam(initdata->prefix, "speed", [&](const int val){ s1_speed = val;});
	initdata->rack->registerCv(initdata->prefix, "speed", [&](const int val){ cv_s1_speed = val;});
	initdata->rack->registerParam(initdata->prefix, "pitch", [&](const int val){ s1_pitch = val;});
	initdata->rack->registerCv(initdata->prefix, "pitch", [&](const int val){ cv_s1_pitch = val;});
	initdata->rack->registerParam(initdata->prefix, "bank", [&](const int val){ s1_bank = val;});
	initdata->rack->registerCv(initdata->prefix, "bank", [&](const int val){ cv_s1_bank = val;});
	initdata->rack->registerParam(initdata->prefix, "slice", [&](const int val){ s1_slice = val;});
	initdata->rack->registerCv(initdata->prefix, "slice", [&](const int val){ cv_s1_slice = val;});
	initdata->rack->registerParam(initdata->prefix, "start", [&](const int val){ s1_start = val;});
	initdata->rack->registerCv(initdata->prefix, "start", [&](const int val){ cv_s1_start = val;});
	initdata->rack->registerParam(initdata->prefix, "end", [&](const int val){ s1_end = val;});
	initdata->rack->registerCv(initdata->prefix, "end", [&](const int val){ cv_s1_end = val;});
	initdata->rack->registerParam(initdata->prefix, "lp", [&](const int val){ s1_lp = val;});
	initdata->rack->registerTrig(initdata->prefix, "lp", [&](const int val){ trig_s1_lp = val;});
	initdata->rack->registerParam(initdata->prefix, "lp_pp", [&](const int val){ s1_lp_pp = val;});
	initdata->rack->registerTrig(initdata->prefix, "lp_pp", [&](const int val){ trig_s1_lp_pp = val;});
	initdata->rack->registerParam(initdata->prefix, "lp_pos", [&](const int val){ s1_lp_pos = val;});
	initdata->rack->registerCv(initdata->prefix, "lp_pos", [&](const int val){ cv_s1_lp_pos = val;});
	initdata->rack->registerParam(initdata->prefix, "atk", [&](const int val){ s1_atk = val;});
	initdata->rack->registerCv(initdata->prefix, "atk", [&](const int val){ cv_s1_atk = val;});
	initdata->rack->registerParam(initdata->prefix, "dcy", [&](const int val){ s1_dcy = val;});
	initdata->rack->registerCv(initdata->prefix, "dcy", [&](const int val){ cv_s1_dcy = val;});
	initdata->rack->registerParam(initdata->prefix, "eg2fm", [&](const int val){ s1_eg2fm = val;});
	initdata->rack->registerCv(initdata->prefix, "eg2fm", [&](const int val){ cv_s1_eg2fm = val;});
	initdata->rack->registerParam(initdata->prefix, "brr", [&](const int val){ s1_brr = val;});
	initdata->rack->registerCv(initdata->prefix, "brr", [&](const int val){ cv_s1_brr = val;});
	initdata->rack->registerParam(initdata->prefix, "ft", [&](const int val){ s1_ft = val;});
	initdata->rack->registerCv(initdata->prefix, "ft", [&](const int val){ cv_s1_ft = val;});
	initdata->rack->registerParam(initdata->prefix, "fc", [&](const int val){ s1_fc = val;});
	initdata->rack->registerCv(initdata->prefix, "fc", [&](const int val){ cv_s1_fc = val;});
	initdata->rack->registerParam(initdata->prefix, "fq", [&](const int val){ s1_fq = val;});
	initdata->rack->registerCv(initdata->prefix, "fq", [&](const int val){ cv_s1_fq = val;});

    this->enabled = false;
}

void DrumRackRompler::Process(const DrumRackProcessData &data) {
    std::fill_n(s1_out, BUF_SZ, 0.f);

    if (!this->enabled) {
        return;
    }

    uint32_t firstNonWtSlice = data.firstNonWtSlice; // sampleRom.GetFirstNonWaveTableSlice();

    MK_BOOL_PAR(bGateS1, s1_gate)
    rompler.params.gate = bGateS1;

    float fS1Speed = s1_speed / 4095.f * 2.f;
    if (cv_s1_speed != -1) fS1Speed += data.cv[cv_s1_speed] * 2.f;
    CONSTRAIN(fS1Speed, -2.f, 2.f)
    rompler.params.playbackSpeed = fS1Speed;
    float fS1Pitch = s1_pitch;
    if (cv_s1_pitch != -1){
        fS1Pitch += data.cv[cv_s1_pitch] * 12.f * 5.f;
    }
    rompler.params.pitch = fS1Pitch;
    MK_INT_PAR_ABS(iS1Bank, s1_bank, 32.f)
    CONSTRAIN(iS1Bank, 0, 31)
    MK_INT_PAR_ABS(iS1Slice, s1_slice, 32.f)
    CONSTRAIN(iS1Slice, 0, 31)
    iS1Slice = iS1Bank * 32 + iS1Slice + firstNonWtSlice;
    rompler.params.slice = iS1Slice;
    MK_FLT_PAR_ABS(fS1Start, s1_start, 4095.f, 1.f)
    rompler.params.startOffsetRelative = fS1Start;
    MK_FLT_PAR_ABS(fS1Length, s1_end, 4095.f, 1.f)
    rompler.params.lengthRelative = fS1Length;
    MK_FLT_PAR_ABS(fS1LoopPos, s1_lp_pos, 4095.f, 1.f)
    rompler.params.loopMarker = fS1LoopPos;
    MK_BOOL_PAR(bS1Loop, s1_lp)
    rompler.params.loop = bS1Loop;
    MK_BOOL_PAR(bS1LoopPipo, s1_lp_pp)
    rompler.params.loopPiPo = bS1LoopPipo;
    MK_FLT_PAR_ABS(fS1Attack, s1_atk, 4095.f, 2.f)
    rompler.params.a = fS1Attack;
    MK_FLT_PAR_ABS(fS1Decay, s1_dcy, 4095.f, 50.f)
    rompler.params.d = fS1Decay;
    MK_FLT_PAR_ABS_SFT(fS1EGFM, s1_eg2fm, 4095.f, 12.f)
    rompler.params.egFM = fS1EGFM;
    MK_INT_PAR_ABS(iS1Brr, s1_brr, 16)
    CONSTRAIN(iS1Brr, 0, 14)
    rompler.params.bitReduction = iS1Brr;
    // filter params
    MK_FLT_PAR_ABS(fS1Cut, s1_fc, 4095.f, 1.f)
    rompler.params.cutoff = fS1Cut;
    MK_FLT_PAR_ABS(fS1Reso, s1_fq, 4095.f, 10.f)
    rompler.params.resonance = fS1Reso;
    MK_INT_PAR_ABS(iS1FType, s1_ft, 4.f)
    CONSTRAIN(iS1FType, 0, 3);
    rompler.params.filterType = static_cast<CTAG::SYNTHESIS::RomplerVoiceMinimal::FilterType>(iS1FType);
    rompler.Process(s1_out, BUF_SZ);
};
