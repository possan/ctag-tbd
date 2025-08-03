#include "ctagSoundProcessorDrumRackSampler.hpp"

using namespace CTAG::SP;

// TODOs: fx return before compressor, stereo panning with delay -> when panned right, levels are lower, metallic sound of reverb.

void ctagSoundProcessorDrumRackSampler::Process(const ProcessData& data){
    const float maxFXSendLevelDly {4.f};
    const float maxFXSendLevelRev {2.f};
  
    // romplers
    uint32_t firstNonWtSlice = sampleRom.GetFirstNonWaveTableSlice();
    float fS1Lev = 0.f, fS1Pan = 0.f;
    MK_BOOL_PAR(bMuteS1, s1_mute)
    MK_FLT_PAR_ABS(fS1FX1Send, s1_fx1, 4095.f, maxFXSendLevelDly)
    fS1FX1Send *= fS1FX1Send;
    MK_FLT_PAR_ABS(fS1FX2Send, s1_fx2, 4095.f, maxFXSendLevelRev)
    fS1FX2Send *= fS1FX2Send;
    if (!bMuteS1){
        MK_BOOL_PAR(bGateS1, s1_gate)
        rompler[0].params.gate = bGateS1;
        fS1Lev = s1_lev / 4095.f * 1.5f;
        if (cv_s1_lev != -1) fS1Lev += fabsf(data.cv[cv_s1_lev]);
        fS1Lev *= fS1Lev;
        fS1Pan = (s1_pan / 4095.f + 1.f) / 2.f * 1.f;
        if (cv_s1_pan != -1) fS1Pan = fabsf(data.cv[cv_s1_pan]) * 1.f;
        float fS1Speed = s1_speed / 4095.f * 2.f;
        if (cv_s1_speed != -1) fS1Speed += data.cv[cv_s1_speed] * 2.f;
        CONSTRAIN(fS1Speed, -2.f, 2.f)
        rompler[0].params.playbackSpeed = fS1Speed;
        float fS1Pitch = s1_pitch;
        if (cv_s1_pitch != -1){
            fS1Pitch += data.cv[cv_s1_pitch] * 12.f * 5.f;
        }
        rompler[0].params.pitch = fS1Pitch;
        MK_INT_PAR_ABS(iS1Bank, s1_bank, 32.f)
        CONSTRAIN(iS1Bank, 0, 31)
        MK_INT_PAR_ABS(iS1Slice, s1_slice, 32.f)
        CONSTRAIN(iS1Slice, 0, 31)
        iS1Slice = iS1Bank * 32 + iS1Slice + firstNonWtSlice;
        rompler[0].params.slice = iS1Slice;
        MK_FLT_PAR_ABS(fS1Start, s1_start, 4095.f, 1.f)
        rompler[0].params.startOffsetRelative = fS1Start;
        MK_FLT_PAR_ABS(fS1Length, s1_end, 4095.f, 1.f)
        rompler[0].params.lengthRelative = fS1Length;
        MK_FLT_PAR_ABS(fS1LoopPos, s1_lp_pos, 4095.f, 1.f)
        rompler[0].params.loopMarker = fS1LoopPos;
        MK_BOOL_PAR(bS1Loop, s1_lp)
        rompler[0].params.loop = bS1Loop;
        MK_BOOL_PAR(bS1LoopPipo, s1_lp_pp)
        rompler[0].params.loopPiPo = bS1LoopPipo;
        MK_FLT_PAR_ABS(fS1Attack, s1_atk, 4095.f, 2.f)
        rompler[0].params.a = fS1Attack;
        MK_FLT_PAR_ABS(fS1Decay, s1_dcy, 4095.f, 50.f)
        rompler[0].params.d = fS1Decay;
        MK_FLT_PAR_ABS_SFT(fS1EGFM, s1_eg2fm, 4095.f, 12.f)
        rompler[0].params.egFM = fS1EGFM;
        MK_INT_PAR_ABS(iS1Brr, s1_brr, 16)
        CONSTRAIN(iS1Brr, 0, 14)
        rompler[0].params.bitReduction = iS1Brr;
        // filter params
        MK_FLT_PAR_ABS(fS1Cut, s1_fc, 4095.f, 1.f)
        rompler[0].params.cutoff = fS1Cut;
        MK_FLT_PAR_ABS(fS1Reso, s1_fq, 4095.f, 10.f)
        rompler[0].params.resonance = fS1Reso;
        MK_INT_PAR_ABS(iS1FType, s1_ft, 4.f)
        CONSTRAIN(iS1FType, 0, 3);
        rompler[0].params.filterType = static_cast<CTAG::SYNTHESIS::RomplerVoiceMinimal::FilterType>(iS1FType);
        rompler[0].Process(s1_out, 32);
        // data_ptrs[9] = s1_out;
    }
    else{
        // data_ptrs[9] = silence;
    }
 
    // // delay
    // MK_FLT_PAR_ABS(fFeedback, fx1_feedback, 4095.f, 1.5f)
    // MK_FLT_PAR_ABS(fBase, fx1_base, 4095.f, 1.f)
    // MK_FLT_PAR_ABS(fWidth, fx1_width, 4095.f, 1.f)
    // MK_FLT_PAR_ABS(fDelayStereoWidth, fx1_st_width, 4095.f, 1.f)
    // MK_FLT_PAR_ABS(fDelayReverbSend, fx1_fx_send, 4095.f, maxFXSendLevelRev)
    // fDelayReverbSend *= fDelayReverbSend;
    // MK_FLT_PAR_ABS(fDelayAmount, fx1_amount, 4095.f, 2.f)
    // MK_BOOL_PAR(bTapeDigital, fx1_tape_digital)
    // MK_BOOL_PAR(bFreeze, fx1_freeze)
    // bool bSync = fx1_sync;
    // bool bSyncTrig {false};
    // if(trig_fx1_sync != -1) bSyncTrig = data.trig[trig_fx1_sync] == 1 ? false : true;
    // if(!bSync){
    //     fDelayTime = fx1_time_ms;
    //     if(cv_fx1_time_ms != -1) fDelayTime = fabsf(data.cv[cv_fx1_time_ms]) * 2000.f;
    // }

    // fBase = 20.f * stmlib::SemitonesToRatio(fBase * 120.f);
    // fWidth = 20.f * stmlib::SemitonesToRatio(fWidth * 120.f);
    // CONSTRAIN(fBase, 20.f, 20000.f)
    // CONSTRAIN(fWidth, 50.f, 20000.f)
    // float hp_cut = fBase;
    // float lp_cut = fBase + fWidth;
    // CONSTRAIN(lp_cut, 20.f, 20000.f)
    // CONSTRAIN(hp_cut, 20.f, 20000.f)
    // lp_l.set_f<stmlib::FREQUENCY_ACCURATE>(lp_cut / 44100.f);
    // hp_l.set_f<stmlib::FREQUENCY_ACCURATE>(hp_cut / 44100.f);
    // lp_r.copy_f(lp_l);
    // hp_r.copy_f(hp_l);
    

    // // sync mechanism
    // if(bSyncTrig != pre_sync){
    //     pre_sync = bSyncTrig;
    //     if(bSyncTrig && bSync){
    //         int delta = timer - pre_timer;
    //         if(std::abs(delta) > 1){
    //             fDelayTime = static_cast<float>(timer) * 32.f / 44.1f;
    //         }
    //         pre_timer = timer;
    //         timer = 0;
    //     }
    // }
    // timer++;

    // // reverb
    // MK_FLT_PAR_ABS(fRevTime, fx2_time, 4095.f, 1.f)
    // MK_FLT_PAR_ABS(fRevAmount, fx2_amount, 4095.f, 2.f)
    // MK_FLT_PAR_ABS(fReverbLPF, fx2_lp, 4095.f, 1.f)
    // reverb.set_time(fRevTime);
    // reverb.set_lp(fReverbLPF);

    // // sum compressor
    // MK_FLT_PAR_ABS_MIN_MAX(fCompThresdB, c_thres, 4095.f, -80.f, 0.f)
    // sumCompressor.setThresh(fCompThresdB);
    // MK_FLT_PAR_ABS_MIN_MAX(fCompAtk, c_atk, 4095.f, 0.3f, 30.f)
    // sumCompressor.setAttack(fCompAtk);
    // MK_FLT_PAR_ABS_MIN_MAX(fCompRel, c_rel, 4095.f, 40.f, 2000.f)
    // sumCompressor.setRelease(fCompRel);
    // MK_FLT_PAR_ABS_MIN_MAX(fCompRatio, c_ratio, 4095.f, 0.0001f, 1.25f)
    // sumCompressor.setRatio(fCompRatio);
    // MK_BOOL_PAR(bSideChainLPF, c_lpf)
    // MK_FLT_PAR_ABS_PAN(fCompMix, c_mix, 4095.f, 1.f)
    // MK_FLT_PAR_ABS_MIN_MAX(fCompMUPGain, c_gain, 4095.f, 0.f, 60.f) // in dB
    // if (fCompMUPGain != fCompMUPGain_pre){
    //     fCompMUPGain = chunkware_simple::dB2lin(fCompMUPGain);
    //     fCompMUPGain_pre = fCompMUPGain;
    // }
    // MK_FLT_PAR_ABS(fCompDlyLevel, c_dly_level, 4095.f, 2.f)
    // fCompDlyLevel *= fCompDlyLevel;
    // MK_FLT_PAR_ABS(fCompRevLevel, c_rev_level, 4095.f, 2.f)
    // fCompRevLevel *= fCompRevLevel;


    // // overall mix
    // MK_BOOL_PAR(bSumMute, sum_mute)
    // MK_FLT_PAR_ABS(fMixLevel, sum_lev, 4095.f, 3.f)
    // fMixLevel *= fMixLevel;

    // if (bSumMute){
    //     memset(data.buf, 0, 32 * 2 * sizeof(float));
    //     return;
    // }

    // // render final buffer
    // // calc volumes
    // float lev_l[13] = {
	//     fABLev * (1.f - fABPan),
	//     fASLev * (1.f - fASPan),
	//     fDBLev * (1.f - fDBPan),
	//     fDSLev * (1.f - fDSPan),
	//     fHH1Lev * (1.f - fHH1Pan),
	//     fHH2Lev * (1.f - fHH2Pan),
	//     fRSLev * (1.f - fRSPan),
	//     fCLLev * (1.f - fCLPan),
	//     fFMBLev * (1.f - fFMBPan),
	//     fS1Lev * (1.f - fS1Pan),
	//     fS2Lev * (1.f - fS2Pan),
	//     fS3Lev * (1.f - fS3Pan),
	//     fS4Lev * (1.f - fS4Pan)
    // };
    // float lev_r[13] = {
	//     fABLev * fABPan,
	//     fASLev * fASPan,
	//     fDBLev * fDBPan,
	//     fDSLev * fDSPan,
	//     fHH1Lev * fHH1Pan,
	//     fHH2Lev * fHH2Pan,
    //     fRSLev * fRSPan,
    //     fCLLev * fCLPan,
    //     fFMBLev * fFMBPan,
    //     fS1Lev * fS1Pan,
    //     fS2Lev * fS2Pan,
    //     fS3Lev * fS3Pan,
    //     fS4Lev * fS4Pan
    // };
    // float buf_fx1_l[32], buf_fx1_r[32], buf_fx2[32];
    // for (int i = 0; i < 32; i++){
    //     float fVal_l = 0.f;
    //     float fVal_r = 0.f;
    // 	// models
    //     fVal_l += data_ptrs[0][i] * lev_l[0];
    //     fVal_l += data_ptrs[1][i] * lev_l[1];
    //     fVal_l += data_ptrs[2][i] * lev_l[2];
    //     fVal_l += data_ptrs[3][i] * lev_l[3];
    //     fVal_l += data_ptrs[4][i] * lev_l[4];
    //     fVal_l += data_ptrs[5][i] * lev_l[5];
    //     fVal_l += data_ptrs[6][i] * lev_l[6];
    //     fVal_l += data_ptrs[7][i] * lev_l[7];
    // 	fVal_l += data_ptrs[8][i] * lev_l[8];
	// 	// romplers
    // 	fVal_l += data_ptrs[9][i] * lev_l[9];
    //     fVal_l += data_ptrs[10][i] * lev_l[10];
    //     fVal_l += data_ptrs[11][i] * lev_l[11];
    //     fVal_l += data_ptrs[12][i] * lev_l[12];
	// 	// models
    //     fVal_r += data_ptrs[0][i] * lev_r[0];
    //     fVal_r += data_ptrs[1][i] * lev_r[1];
    //     fVal_r += data_ptrs[2][i] * lev_r[2];
    //     fVal_r += data_ptrs[3][i] * lev_r[3];
    //     fVal_r += data_ptrs[4][i] * lev_r[4];
    //     fVal_r += data_ptrs[5][i] * lev_r[5];
    //     fVal_r += data_ptrs[6][i] * lev_r[6];
    //     fVal_r += data_ptrs[7][i] * lev_r[7];
    // 	fVal_r += data_ptrs[8][i] * lev_r[8];
	// 	// romplers
    // 	fVal_r += data_ptrs[9][i] * lev_r[9];
    //     fVal_r += data_ptrs[10][i] * lev_r[10];
    //     fVal_r += data_ptrs[11][i] * lev_r[11];
    //     fVal_r += data_ptrs[12][i] * lev_r[12];


    //     // FX1 models
    //     buf_fx1_l[i] = data_ptrs[0][i] * fABFX1Send * lev_l[0];
    //     buf_fx1_l[i] += data_ptrs[1][i] * fASFX1Send * lev_l[1];
    //     buf_fx1_l[i] += data_ptrs[2][i] * fDBFX1Send * lev_l[2];
    //     buf_fx1_l[i] += data_ptrs[3][i] * fDSFX1Send * lev_l[3];
    //     buf_fx1_l[i] += data_ptrs[4][i] * fHH1FX1Send * lev_l[4];
    //     buf_fx1_l[i] += data_ptrs[5][i] * fHH2FX1Send * lev_l[5];
    //     buf_fx1_l[i] += data_ptrs[6][i] * fRSFX1Send * lev_l[6];
    //     buf_fx1_l[i] += data_ptrs[7][i] * fCLFX1Send * lev_l[7];
    //     buf_fx1_l[i] += data_ptrs[8][i] * fFMBFX1Send * lev_l[8];

    // 	// FX1 romplers
    //     buf_fx1_l[i] += data_ptrs[9][i] * fS1FX1Send * lev_l[9];
    //     buf_fx1_l[i] += data_ptrs[10][i] * fS2FX1Send * lev_l[10];
    //     buf_fx1_l[i] += data_ptrs[11][i] * fS3FX1Send * lev_l[11];
    //     buf_fx1_l[i] += data_ptrs[12][i] * fS4FX1Send * lev_l[12];

    // 	// FX1 models
    //     buf_fx1_r[i] = data_ptrs[0][i] * fABFX1Send * lev_r[0];
    //     buf_fx1_r[i] += data_ptrs[1][i] * fASFX1Send * lev_r[1];
    //     buf_fx1_r[i] += data_ptrs[2][i] * fDBFX1Send * lev_r[2];
    //     buf_fx1_r[i] += data_ptrs[3][i] * fDSFX1Send * lev_r[3];
    //     buf_fx1_r[i] += data_ptrs[4][i] * fHH1FX1Send * lev_r[4];
    //     buf_fx1_r[i] += data_ptrs[5][i] * fHH2FX1Send * lev_r[5];
    //     buf_fx1_r[i] += data_ptrs[6][i] * fRSFX1Send * lev_r[6];
    //     buf_fx1_r[i] += data_ptrs[7][i] * fCLFX1Send * lev_r[7];
    //     buf_fx1_r[i] += data_ptrs[8][i] * fFMBFX1Send * lev_r[8];

    // 	// FX1 romplers
    //     buf_fx1_r[i] += data_ptrs[9][i] * fS1FX1Send * lev_r[9];
    //     buf_fx1_r[i] += data_ptrs[10][i] * fS2FX1Send * lev_r[10];
    //     buf_fx1_r[i] += data_ptrs[11][i] * fS3FX1Send * lev_r[11];
    //     buf_fx1_r[i] += data_ptrs[12][i] * fS4FX1Send * lev_r[12];
        
    //     // FX2 models, reverb is mono in stereo out
    //     buf_fx2[i] = data_ptrs[0][i] * fABFX2Send;
    //     buf_fx2[i] += data_ptrs[1][i] * fASFX2Send;
    //     buf_fx2[i] += data_ptrs[2][i] * fDBFX2Send;
    //     buf_fx2[i] += data_ptrs[3][i] * fDSFX2Send;
    //     buf_fx2[i] += data_ptrs[4][i] * fHH1FX2Send;
    //     buf_fx2[i] += data_ptrs[5][i] * fHH2FX2Send;
    //     buf_fx2[i] += data_ptrs[6][i] * fRSFX2Send;
    //     buf_fx2[i] += data_ptrs[7][i] * fCLFX2Send;
    //     buf_fx2[i] += data_ptrs[8][i] * fFMBFX2Send;
	// 	// FX2 romplers
    //     buf_fx2[i] += data_ptrs[9][i] * fS1FX2Send;
    //     buf_fx2[i] += data_ptrs[10][i] * fS2FX2Send;
    //     buf_fx2[i] += data_ptrs[11][i] * fS3FX2Send;
    //     buf_fx2[i] += data_ptrs[12][i] * fS4FX2Send;


    //     float dry_l = fVal_l;
    //     float dry_r = fVal_r;
    //     if (bSideChainLPF){
    //         ONE_POLE(side_l, fVal_l, 0.0005f);
    //         ONE_POLE(side_r, fVal_r, 0.0005f);
    //     }
    //     else{
    //         side_l = fVal_l;
    //         side_r = fVal_r;
    //     }
    //     side_l = fabsf(side_l);
    //     side_r = fabsf(side_r);
    //     float side = std::max(side_l, side_r);
    //     sumCompressor.process(fVal_l, fVal_r, side);
    //     fVal_l = fVal_l * fCompMUPGain * fCompMix + dry_l * (1.f - fCompMix);
    //     fVal_r = fVal_r * fCompMUPGain * fCompMix + dry_r * (1.f - fCompMix);
    //     data.buf[i * 2] = fVal_l * fMixLevel;
    //     data.buf[i * 2 + 1] = fVal_r * fMixLevel;
    // }

    // // fx buffers
    // float dly_buf_l[32], dly_buf_r[32];
    // float rev_buf_l[32], rev_buf_r[32];

    // // delay
    // CONSTRAIN(fDelayTime, 0.0001, 2000.f)
    // float ofs = fDelayTime * 44.1f;
    // if(fabsf(ofs - delayOffset) < 16) ofs = delayOffset;
    // for(int i=0; i<32; i++){
    //     // Calculate the delay offset in samples
    //     if(delayOffset != ofs){
    //         if(bTapeDigital){
    //             if(ofs != delayOffset){
    //                 duck = 1.f;
    //             }
    //             delayOffset = ofs;
    //         } else {
    //             float temp = delayOffset;
    //             delayOffset = ONE_POLE(temp, ofs, 0.0001f);
    //         }
    //         readPos = static_cast<float>(writeIndex) - delayOffset;
    //         if(readPos < 0.f) readPos += float(delayBufferSizeMax);
    //         if(readPos >= float(delayBufferSizeMax)) readPos -= float(delayBufferSizeMax);
    //     }

    //     float inputSample_l = buf_fx1_l[i];
    //     float inputSample_r = buf_fx1_r[i];
    //     float outputSample_l, outputSample_r;

    //     outputSample_l = HELPERS::InterpolateWaveLinearWrap(delayBuffer_l, readPos, delayBufferSizeMax);
    //     outputSample_r = HELPERS::InterpolateWaveLinearWrap(delayBuffer_r, readPos, delayBufferSizeMax);
    //     readPos += 1.f;
    //     readPos > float(delayBufferSizeMax) ? readPos -= float(delayBufferSizeMax) : readPos;

    //     float temp = duck;
    //     duck = ONE_POLE(temp, 0.f, 0.35f)
    //     outputSample_l = outputSample_l * (1.f - duck);
    //     outputSample_r = outputSample_r * (1.f - duck);
    //     // Write the input sample to the delay buffer
    //     float out_l, out_r;
    //     if(!bFreeze){
    //         out_l = inputSample_l + fFeedback * ((1.f - fDelayStereoWidth) * outputSample_l + fDelayStereoWidth * outputSample_r);
    //         out_l = lp_l.Process<stmlib::FILTER_MODE_LOW_PASS>(out_l);
    //         out_l = hp_l.Process<stmlib::FILTER_MODE_HIGH_PASS>(out_l);
    //         out_r = (1.f - fDelayStereoWidth) * inputSample_r + fFeedback * ((1.f - fDelayStereoWidth) * outputSample_r + fDelayStereoWidth * outputSample_l);
    //         out_r = lp_r.Process<stmlib::FILTER_MODE_LOW_PASS>(out_r);
    //         out_r = hp_r.Process<stmlib::FILTER_MODE_HIGH_PASS>(out_r);
    //     }
    //     else{
    //         out_l = ((1.f - fDelayStereoWidth) * outputSample_l + fDelayStereoWidth * outputSample_r);
    //         out_r = ((1.f - fDelayStereoWidth) * outputSample_r + fDelayStereoWidth * outputSample_l);
    //     }

    //     delayBuffer_l[writeIndex] = stmlib::SoftLimit(out_l);
    //     delayBuffer_r[writeIndex] = stmlib::SoftLimit(out_r);
    //     writeIndex = (writeIndex + 1) % delayBufferSizeMax;

    //     // Mix the dry (input) and wet (delayed) signal
    //     dly_buf_l[i] = outputSample_l;
    //     dly_buf_r[i] = outputSample_r;
    //     rev_buf_l[i] = buf_fx2[i] + dly_buf_l[i] * fDelayReverbSend;
    //     rev_buf_r[i] = buf_fx2[i] + dly_buf_r[i] * fDelayReverbSend;
    // }

    // // reverb
    // reverb.Process(rev_buf_l, rev_buf_r, 32);

    // // add fx to sum
    // fRevAmount *= fRevAmount;
    // fDelayAmount *= fDelayAmount;
    // for (int i = 0; i < 32; i++) {
    //     data.buf[i * 2] += rev_buf_l[i] * fRevAmount + dly_buf_l[i] * fDelayAmount;
    //     data.buf[i * 2 + 1] += rev_buf_r[i] * fRevAmount + dly_buf_r[i] * fDelayAmount;
    // }
}

void ctagSoundProcessorDrumRackSampler::Init(std::size_t blockSize, void* blockPtr){
    // construct internal data model
    knowYourself();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);

    sampleRom.BufferInSPIRAM();
    std::fill_n(silence, 32, 0.f);

    // init romplers
    for (auto& r : rompler){
        r.Init(44100.f);
    }

    // check if blockMem is large enough
    // blockMem is used just like larger blocks of heap memory
    // assert(blockSize >= memLen);
    // if memory larger than blockMem is needed, use heap_caps_malloc() instead with MALLOC_CAPS_SPIRAM
}

// no ctor, use Init() instead, is called from factory after successful creation
// dtor
ctagSoundProcessorDrumRackSampler::~ctagSoundProcessorDrumRackSampler(){
    // no explicit freeing for blockMem needed, done by ctagSPAllocator
    // explicit free is only needed when using heap_caps_malloc() with MALLOC_CAPS_SPIRAM
}

void ctagSoundProcessorDrumRackSampler::knowYourself(){
    // autogenerated code here
    // sectionCpp0
	pMapPar.emplace("s1_gate", [&](const int val){ s1_gate = val;});
	pMapTrig.emplace("s1_gate", [&](const int val){ trig_s1_gate = val;});
	pMapPar.emplace("s1_mute", [&](const int val){ s1_mute = val;});
	pMapTrig.emplace("s1_mute", [&](const int val){ trig_s1_mute = val;});
	// pMapPar.emplace("s1_lev", [&](const int val){ s1_lev = val;});
	// pMapCv.emplace("s1_lev", [&](const int val){ cv_s1_lev = val;});
	// pMapPar.emplace("s1_pan", [&](const int val){ s1_pan = val;});
	// pMapCv.emplace("s1_pan", [&](const int val){ cv_s1_pan = val;});
	// pMapPar.emplace("s1_fx1", [&](const int val){ s1_fx1 = val;});
	// pMapCv.emplace("s1_fx1", [&](const int val){ cv_s1_fx1 = val;});
	// pMapPar.emplace("s1_fx2", [&](const int val){ s1_fx2 = val;});
	// pMapCv.emplace("s1_fx2", [&](const int val){ cv_s1_fx2 = val;});
	pMapPar.emplace("s1_speed", [&](const int val){ s1_speed = val;});
	pMapCv.emplace("s1_speed", [&](const int val){ cv_s1_speed = val;});
	pMapPar.emplace("s1_pitch", [&](const int val){ s1_pitch = val;});
	pMapCv.emplace("s1_pitch", [&](const int val){ cv_s1_pitch = val;});
	pMapPar.emplace("s1_bank", [&](const int val){ s1_bank = val;});
	pMapCv.emplace("s1_bank", [&](const int val){ cv_s1_bank = val;});
	pMapPar.emplace("s1_slice", [&](const int val){ s1_slice = val;});
	pMapCv.emplace("s1_slice", [&](const int val){ cv_s1_slice = val;});
	pMapPar.emplace("s1_start", [&](const int val){ s1_start = val;});
	pMapCv.emplace("s1_start", [&](const int val){ cv_s1_start = val;});
	pMapPar.emplace("s1_end", [&](const int val){ s1_end = val;});
	pMapCv.emplace("s1_end", [&](const int val){ cv_s1_end = val;});
	pMapPar.emplace("s1_lp", [&](const int val){ s1_lp = val;});
	pMapTrig.emplace("s1_lp", [&](const int val){ trig_s1_lp = val;});
	pMapPar.emplace("s1_lp_pp", [&](const int val){ s1_lp_pp = val;});
	pMapTrig.emplace("s1_lp_pp", [&](const int val){ trig_s1_lp_pp = val;});
	pMapPar.emplace("s1_lp_pos", [&](const int val){ s1_lp_pos = val;});
	pMapCv.emplace("s1_lp_pos", [&](const int val){ cv_s1_lp_pos = val;});
	pMapPar.emplace("s1_atk", [&](const int val){ s1_atk = val;});
	pMapCv.emplace("s1_atk", [&](const int val){ cv_s1_atk = val;});
	pMapPar.emplace("s1_dcy", [&](const int val){ s1_dcy = val;});
	pMapCv.emplace("s1_dcy", [&](const int val){ cv_s1_dcy = val;});
	pMapPar.emplace("s1_eg2fm", [&](const int val){ s1_eg2fm = val;});
	pMapCv.emplace("s1_eg2fm", [&](const int val){ cv_s1_eg2fm = val;});
	pMapPar.emplace("s1_brr", [&](const int val){ s1_brr = val;});
	pMapCv.emplace("s1_brr", [&](const int val){ cv_s1_brr = val;});
	pMapPar.emplace("s1_ft", [&](const int val){ s1_ft = val;});
	pMapCv.emplace("s1_ft", [&](const int val){ cv_s1_ft = val;});
	pMapPar.emplace("s1_fc", [&](const int val){ s1_fc = val;});
	pMapCv.emplace("s1_fc", [&](const int val){ cv_s1_fc = val;});
	pMapPar.emplace("s1_fq", [&](const int val){ s1_fq = val;});
	pMapCv.emplace("s1_fq", [&](const int val){ cv_s1_fq = val;});
	isStereo = true;
	id = "DrumRackSampler";
	// sectionCpp0
}
