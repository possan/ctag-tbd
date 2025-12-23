#include "DrumRackSynth.hpp"
#include "DrumRackRompler.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"

using namespace CTAG::SP;

void DrumRackRompler::Init(const DrumRackInitData *initdata) {
    rompler.Init(44100.f);

    initdata->rack->registerParamAndCC(initdata, "bank", 6, [&](const int val){ s1_bank = val;});
    initdata->rack->registerParamAndCC(initdata, "slice", 7, [&](const int val){ s1_slice = val;});
    initdata->rack->registerParamAndCC(initdata, "start", 8, [&](const int val){ s1_start = val;});
    initdata->rack->registerParamAndCC(initdata, "end", 9, [&](const int val){ s1_end = val;});
    
    initdata->rack->registerParamAndCC(initdata, "fc", 10, [&](const int val){ s1_fc = val;});
    initdata->rack->registerParamAndCC(initdata, "fq", 11, [&](const int val){ s1_fq = val;});
    initdata->rack->registerParamAndCC(initdata, "ft", 12, [&](const int val){ s1_ft = val;});
    initdata->rack->registerParamAndCC(initdata, "brr", 13, [&](const int val){ s1_brr = val;});
    
    initdata->rack->registerParamAndCC(initdata, "atk", 14, [&](const int val){ s1_atk = val;});
    initdata->rack->registerParamAndCC(initdata, "dcy", 15, [&](const int val){ s1_dcy = val;});
    initdata->rack->registerParamAndCC(initdata, "speed", 16, [&](const int val){ s1_speed = val;});
    initdata->rack->registerParamAndCC(initdata, "pitch", 17, [&](const int val){ s1_pitch = val;});
    
    initdata->rack->registerParamAndCC(initdata, "lp", 18, [&](const int val){ s1_lp = val;});
    initdata->rack->registerParamAndCC(initdata, "lp_pp", 19, [&](const int val){ s1_lp_pp = val;});
    initdata->rack->registerParamAndCC(initdata, "lp_pos", 20, [&](const int val){ s1_lp_pos = val;});
    initdata->rack->registerParamAndCC(initdata, "eg2fm", 21, [&](const int val){ s1_eg2fm = val;});

    initdata->rack->registerParamAndCC(initdata, "tsmode", 22, [&](const int val){ s1_tsmode = val;});
    initdata->rack->registerParamAndCC(initdata, "tsamount", 23, [&](const int val){ s1_tsamount = val;});
    initdata->rack->registerParamAndCC(initdata, "tssteps", 24, [&](const int val){ s1_tssteps = val;});

    s1_lp = 0;
    s1_lp_pp = 0;

    this->enabled = false;
}

void DrumRackRompler::handleMidiNoteOn(uint8_t note, uint8_t vel) {
    midi_trig = true;
    midi_note = note;
    midi_freq = 440.f * powf(2.f, (note - 69) / 12.f);
    // printf("rompler note on %d, %d (%f hz)\n", note, vel, midi_freq);
}

void DrumRackRompler::handleMidiNoteOff(uint8_t note, uint8_t vel) {
    // TODO: Implement
    // midi_trig = false;
    // printf("Rompler Note off %d %d\n", note, vel);
}

// void DrumRackRompler::handleMidiCC(uint8_t control, uint8_t value) {
//     // TODO: Implement
//     printf("Rompler CC %d %d\n", control, value);
// }

void DrumRackRompler::Process(const DrumRackProcessData &data) {
    std::fill_n(s1_out, BUF_SZ, 0.f);

    if (!this->enabled) {
        return;
    }

    MK_INT_PAR_ABS_NOCV(bTSMode, s1_tsmode, 2.0f)
    rompler.params.timeStretchEnable = bTSMode > 0;

    // timestretch target length
    MK_INT_PAR_ABS_NOCV(iTSSteps, s1_tssteps, 127.f)
    CONSTRAIN(iTSSteps, 1, 127)

    uint32_t firstNonWtSlice = data.firstNonWtSlice; // sampleRom.GetFirstNonWaveTableSlice();
    MK_INT_PAR_ABS_NOCV(iS1Bank, s1_bank, 32.f)
    CONSTRAIN(iS1Bank, 0, 31)
    MK_INT_PAR_ABS_NOCV(iS1Slice, s1_slice, 127.f) // midi cc
    CONSTRAIN(iS1Slice, 0, 31)
    iS1Slice = iS1Bank * 32 + iS1Slice + firstNonWtSlice;
    rompler.params.slice = iS1Slice;
   
    MK_FLT_PAR_ABS_NOCV(fS1Speed, s1_speed, 4095.f, 2.f)
    // fS1Speed -= 2.0f;
    CONSTRAIN(fS1Speed, 0.f, 2.f)
    if (!rompler.params.timeStretchEnable) {
        rompler.params.playbackSpeed = fS1Speed;
        // base on loop length / track length
        // rompler.params.playbackSpeed = 1.0;
    }

    MK_INT_PAR_ABS_NOCV(iS1Pitch, s1_pitch, 127.f) // midi cc
    if (rompler.params.timeStretchEnable) {
        rompler.params.pitch = iS1Pitch / 10.0;
    } else { 
        rompler.params.pitch = midi_note;
    }

     MK_FLT_PAR_ABS_NOCV(fS1Start, s1_start, 4095.f, 1.f)
    rompler.params.startOffsetRelative = fS1Start;
    MK_FLT_PAR_ABS_NOCV(fS1Length, s1_end, 4095.f, 1.f)
    rompler.params.lengthRelative = fS1Length;
    MK_FLT_PAR_ABS_NOCV(fS1LoopPos, s1_lp_pos, 4095.f, 1.f)
    rompler.params.loopMarker = fS1LoopPos;
    MK_BOOL_PAR_NOCV(bS1Loop, s1_lp)
    rompler.params.loop = bS1Loop;
    MK_BOOL_PAR_NOCV(bS1LoopPipo, s1_lp_pp)
    rompler.params.loopPiPo = bS1LoopPipo;
    MK_FLT_PAR_ABS_NOCV(fS1Attack, s1_atk, 4095.f, 2.f)
    rompler.params.a = fS1Attack;
    MK_FLT_PAR_ABS_NOCV(fS1Decay, s1_dcy, 4095.f, 50.f)
    rompler.params.d = fS1Decay;
    MK_FLT_PAR_ABS_SFT_NOCV(fS1EGFM, s1_eg2fm, 4095.f, 12.f)
    rompler.params.egFM = fS1EGFM;
    MK_INT_PAR_ABS_NOCV(iS1Brr, s1_brr, 16)
    CONSTRAIN(iS1Brr, 0, 14)
    rompler.params.bitReduction = iS1Brr;
    // filter params
    MK_FLT_PAR_ABS_NOCV(fS1Cut, s1_fc, 4095.f, 1.f)
    rompler.params.cutoff = fS1Cut;
    MK_FLT_PAR_ABS_NOCV(fS1Reso, s1_fq, 4095.f, 10.f)
    rompler.params.resonance = fS1Reso;
    MK_INT_PAR_ABS_NOCV(iS1FType, s1_ft, 4.f)
    CONSTRAIN(iS1FType, 0, 3);
    // timestretch stuff


    MK_FLT_PAR_ABS_NOCV(fTSAmount, s1_tsamount, 4095.f, 1.f)
    float fTS1Amount = 0.005f + fTSAmount * 0.995f;
    rompler.params.timeStretchWindowSize = fTS1Amount;

    // MK_BOOL_PAR_NOCV(bGateS1, s1_gate)
    rompler.params.gate = midi_trig;
    if (midi_trig && !trig_prev) {


        uint32_t sliceLength = 0;
        uint32_t stepsLengthMs = 0;
        uint32_t sliceLengthMs = 0;

        rompler.params.playbackSpeed = 1.0;
        if (data.sampleRom->HasSlice(rompler.params.slice)) {
            sliceLength = data.sampleRom->GetSliceSize(rompler.params.slice);
            stepsLengthMs = iTSSteps * data.msPerBeat / 4;
            sliceLengthMs = (sliceLength * 1000) / 44100;
            rompler.params.playbackSpeed = (float)sliceLengthMs / (float)stepsLengthMs;
        }

        printf("S1 sl=%ld ps=%1.3f pitch=%1.3f, ts=%d>%1.1f, slicelen=%ld,msperbeat=%ld,steps=%d,slicelenms=%ld\n",
            rompler.params.slice,
            rompler.params.playbackSpeed,
            rompler.params.pitch,
            rompler.params.timeStretchEnable,
            rompler.params.timeStretchWindowSize,
            sliceLength,
            data.msPerBeat,
            iTSSteps,
            sliceLengthMs
        );

        // printf("S1 slice=%ld ps=%1.1f pitch=%1.1f %1.1f %1.1f\n",
        //     rompler.params.slice,
        //     rompler.params.playbackSpeed,
        //     rompler.params.pitch,
        //     rompler.params.startOffsetRelative,
        //     rompler.params.lengthRelative);

        // printf("S2 %1.1f %1.1f %1.1f %1.1f %d\n",
        //     (float)rompler.params.a,
        //     (float)rompler.params.d,
        //     (float)rompler.params.cutoff,
        //     (float)rompler.params.resonance,
        //     (int)rompler.params.filterType);

        // printf("S3 %d %d %1.1f %1.1f %ld %d\n",
        //     rompler.params.loop,
        //     rompler.params.loopPiPo,
        //     (float)rompler.params.loopMarker,
        //     (float)rompler.params.egFM,
        //     rompler.params.bitReduction,
        //     rompler.params.gate);
    }
    trig_prev = midi_trig;
    midi_trig = false;

    rompler.params.filterType = static_cast<CTAG::SYNTHESIS::RomplerVoiceMinimal::Params::FilterType>(iS1FType);
    rompler.Process(s1_out, BUF_SZ);
};
