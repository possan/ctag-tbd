#include "DrumRackSynth.hpp"
#include "DrumRackRompler.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

void DrumRackRompler::Init(const DrumRackInitData *initdata) {
    // uint8_t *privatedata = initdata->allocator(1000);

    for (auto& r : rompler){
        r.Init(44100.f);
    }

    initdata->rack->registerParam(initdata->prefix, "s1_gate", [&](const int val){ s1_gate = val;});
	initdata->rack->registerTrig(initdata->prefix, "s1_gate", [&](const int val){ trig_s1_gate = val;});
	initdata->rack->registerParam(initdata->prefix, "s1_mute", [&](const int val){ s1_mute = val;});
	initdata->rack->registerTrig(initdata->prefix, "s1_mute", [&](const int val){ trig_s1_mute = val;});
	initdata->rack->registerParam(initdata->prefix, "s1_lev", [&](const int val){ s1_lev = val;});
	initdata->rack->registerCv(initdata->prefix, "s1_lev", [&](const int val){ cv_s1_lev = val;});
	initdata->rack->registerParam(initdata->prefix, "s1_pan", [&](const int val){ s1_pan = val;});
	initdata->rack->registerCv(initdata->prefix, "s1_pan", [&](const int val){ cv_s1_pan = val;});
	initdata->rack->registerParam(initdata->prefix, "s1_fx1", [&](const int val){ s1_fx1 = val;});
	initdata->rack->registerCv(initdata->prefix, "s1_fx1", [&](const int val){ cv_s1_fx1 = val;});
	initdata->rack->registerParam(initdata->prefix, "s1_fx2", [&](const int val){ s1_fx2 = val;});
	initdata->rack->registerCv(initdata->prefix, "s1_fx2", [&](const int val){ cv_s1_fx2 = val;});
	initdata->rack->registerParam(initdata->prefix, "s1_speed", [&](const int val){ s1_speed = val;});
	initdata->rack->registerCv(initdata->prefix, "s1_speed", [&](const int val){ cv_s1_speed = val;});
	initdata->rack->registerParam(initdata->prefix, "s1_pitch", [&](const int val){ s1_pitch = val;});
	initdata->rack->registerCv(initdata->prefix, "s1_pitch", [&](const int val){ cv_s1_pitch = val;});
	initdata->rack->registerParam(initdata->prefix, "s1_bank", [&](const int val){ s1_bank = val;});
	initdata->rack->registerCv(initdata->prefix, "s1_bank", [&](const int val){ cv_s1_bank = val;});
	initdata->rack->registerParam(initdata->prefix, "s1_slice", [&](const int val){ s1_slice = val;});
	initdata->rack->registerCv(initdata->prefix, "s1_slice", [&](const int val){ cv_s1_slice = val;});
	initdata->rack->registerParam(initdata->prefix, "s1_start", [&](const int val){ s1_start = val;});
	initdata->rack->registerCv(initdata->prefix, "s1_start", [&](const int val){ cv_s1_start = val;});
	initdata->rack->registerParam(initdata->prefix, "s1_end", [&](const int val){ s1_end = val;});
	initdata->rack->registerCv(initdata->prefix, "s1_end", [&](const int val){ cv_s1_end = val;});
	initdata->rack->registerParam(initdata->prefix, "s1_lp", [&](const int val){ s1_lp = val;});
	initdata->rack->registerTrig(initdata->prefix, "s1_lp", [&](const int val){ trig_s1_lp = val;});
	initdata->rack->registerParam(initdata->prefix, "s1_lp_pp", [&](const int val){ s1_lp_pp = val;});
	initdata->rack->registerTrig(initdata->prefix, "s1_lp_pp", [&](const int val){ trig_s1_lp_pp = val;});
	initdata->rack->registerParam(initdata->prefix, "s1_lp_pos", [&](const int val){ s1_lp_pos = val;});
	initdata->rack->registerCv(initdata->prefix, "s1_lp_pos", [&](const int val){ cv_s1_lp_pos = val;});
	initdata->rack->registerParam(initdata->prefix, "s1_atk", [&](const int val){ s1_atk = val;});
	initdata->rack->registerCv(initdata->prefix, "s1_atk", [&](const int val){ cv_s1_atk = val;});
	initdata->rack->registerParam(initdata->prefix, "s1_dcy", [&](const int val){ s1_dcy = val;});
	initdata->rack->registerCv(initdata->prefix, "s1_dcy", [&](const int val){ cv_s1_dcy = val;});
	initdata->rack->registerParam(initdata->prefix, "s1_eg2fm", [&](const int val){ s1_eg2fm = val;});
	initdata->rack->registerCv(initdata->prefix, "s1_eg2fm", [&](const int val){ cv_s1_eg2fm = val;});
	initdata->rack->registerParam(initdata->prefix, "s1_brr", [&](const int val){ s1_brr = val;});
	initdata->rack->registerCv(initdata->prefix, "s1_brr", [&](const int val){ cv_s1_brr = val;});
	initdata->rack->registerParam(initdata->prefix, "s1_ft", [&](const int val){ s1_ft = val;});
	initdata->rack->registerCv(initdata->prefix, "s1_ft", [&](const int val){ cv_s1_ft = val;});
	initdata->rack->registerParam(initdata->prefix, "s1_fc", [&](const int val){ s1_fc = val;});
	initdata->rack->registerCv(initdata->prefix, "s1_fc", [&](const int val){ cv_s1_fc = val;});
	initdata->rack->registerParam(initdata->prefix, "s1_fq", [&](const int val){ s1_fq = val;});
	initdata->rack->registerCv(initdata->prefix, "s1_fq", [&](const int val){ cv_s1_fq = val;});

}

// void DrumRackRompler::SetParamValue(const string &id, const string &key, const int val) {
//     // Implementation of setting parameter value
//     // This is where you would handle the parameter setting logic
//     // For example, you might store the value in a map or update an internal state
//     // ESP_LOGI("DrumRackRompler", "Setting parameter %s with key %s to value %d", id.c_str(), key.c_str(), val);
//     // You can add your specific logic here
// }

void DrumRackRompler::Process(const DrumRackProcessData &data) {
    //  uint32_t firstNonWtSlice = sampleRom.GetFirstNonWaveTableSlice();
    // float fS1Lev = 0.f, fS1Pan = 0.f;
    // MK_BOOL_PAR(bMuteS1, s1_mute)
    // fS1Lev = s1_lev / 4095.f * 1.5f;
    // if (cv_s1_lev != -1) fS1Lev += fabsf(data.cv[cv_s1_lev]);
    // fS1Lev *= fS1Lev;
    // fS1Pan = (s1_pan / 4095.f + 1.f) / 2.f * 1.f;
    // if (cv_s1_pan != -1) fS1Pan = fabsf(data.cv[cv_s1_pan]) * 1.f;
    // MK_FLT_PAR_ABS(fS1FX1Send, s1_fx1, 4095.f, maxFXSendLevelDly); fS1FX1Send *= fS1FX1Send;
    // MK_FLT_PAR_ABS(fS1FX2Send, s1_fx2, 4095.f, maxFXSendLevelRev); fS1FX2Send *= fS1FX2Send;

    // if (bMuteS1 || fS1Lev < minVolume) {
    //     return;
    // }

    // MK_BOOL_PAR(bGateS1, s1_gate)
    // rompler[0].params.gate = bGateS1;
    // fS1Lev = s1_lev / 4095.f * 1.5f;
    // if (cv_s1_lev != -1) fS1Lev += fabsf(data.cv[cv_s1_lev]);
    // fS1Lev *= fS1Lev;
    // float fS1Speed = s1_speed / 4095.f * 2.f;
    // if (cv_s1_speed != -1) fS1Speed += data.cv[cv_s1_speed] * 2.f;
    // CONSTRAIN(fS1Speed, -2.f, 2.f)
    // rompler[0].params.playbackSpeed = fS1Speed;
    // float fS1Pitch = s1_pitch;
    // if (cv_s1_pitch != -1){
    //     fS1Pitch += data.cv[cv_s1_pitch] * 12.f * 5.f;
    // }
    // rompler[0].params.pitch = fS1Pitch;
    // MK_INT_PAR_ABS(iS1Bank, s1_bank, 32.f)
    // CONSTRAIN(iS1Bank, 0, 31)
    // MK_INT_PAR_ABS(iS1Slice, s1_slice, 32.f)
    // CONSTRAIN(iS1Slice, 0, 31)
    // iS1Slice = iS1Bank * 32 + iS1Slice + firstNonWtSlice;
    // rompler[0].params.slice = iS1Slice;
    // MK_FLT_PAR_ABS(fS1Start, s1_start, 4095.f, 1.f)
    // rompler[0].params.startOffsetRelative = fS1Start;
    // MK_FLT_PAR_ABS(fS1Length, s1_end, 4095.f, 1.f)
    // rompler[0].params.lengthRelative = fS1Length;
    // MK_FLT_PAR_ABS(fS1LoopPos, s1_lp_pos, 4095.f, 1.f)
    // rompler[0].params.loopMarker = fS1LoopPos;
    // MK_BOOL_PAR(bS1Loop, s1_lp)
    // rompler[0].params.loop = bS1Loop;
    // MK_BOOL_PAR(bS1LoopPipo, s1_lp_pp)
    // rompler[0].params.loopPiPo = bS1LoopPipo;
    // MK_FLT_PAR_ABS(fS1Attack, s1_atk, 4095.f, 2.f)
    // rompler[0].params.a = fS1Attack;
    // MK_FLT_PAR_ABS(fS1Decay, s1_dcy, 4095.f, 50.f)
    // rompler[0].params.d = fS1Decay;
    // MK_FLT_PAR_ABS_SFT(fS1EGFM, s1_eg2fm, 4095.f, 12.f)
    // rompler[0].params.egFM = fS1EGFM;
    // MK_INT_PAR_ABS(iS1Brr, s1_brr, 16)
    // CONSTRAIN(iS1Brr, 0, 14)
    // rompler[0].params.bitReduction = iS1Brr;
    // // filter params
    // MK_FLT_PAR_ABS(fS1Cut, s1_fc, 4095.f, 1.f)
    // rompler[0].params.cutoff = fS1Cut;
    // MK_FLT_PAR_ABS(fS1Reso, s1_fq, 4095.f, 10.f)
    // rompler[0].params.resonance = fS1Reso;
    // MK_INT_PAR_ABS(iS1FType, s1_ft, 4.f)
    // CONSTRAIN(iS1FType, 0, 3);
    // rompler[0].params.filterType = static_cast<CTAG::SYNTHESIS::RomplerVoiceMinimal::FilterType>(iS1FType);
    // rompler[0].Process(s1_out, 32);
    // mixRenderOutputMono(s1_out, fS1Lev, fS1Pan, fS1FX1Send, fS1FX2Send);

};
