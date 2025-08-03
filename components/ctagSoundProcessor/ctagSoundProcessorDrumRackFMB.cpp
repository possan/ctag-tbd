#include "ctagSoundProcessorDrumRackFMB.hpp"

using namespace CTAG::SP;

// TODOs: fx return before compressor, stereo panning with delay -> when panned right, levels are lower, metallic sound of reverb.

void ctagSoundProcessorDrumRackFMB::Process(const ProcessData& data){
    const float maxFXSendLevelDly {4.f};
    const float maxFXSendLevelRev {2.f};
    
	// FM Bass Drum
    MK_BOOL_PAR(bFMBMute, fmb_mute)
    MK_BOOL_PAR(bFMBTrig, fmb_trigger)
    if (bFMBTrig != fmb_trig_prev && bFMBTrig){
	    fmb_trig_prev = true;
	    fmb.Trigger();
    }
    else if (!bFMBTrig){
	    fmb_trig_prev = false;
    }

    MK_FLT_PAR_ABS(fFMBLev, fmb_lev, 4095.f, 2.f)
    fFMBLev *= fFMBLev;
    MK_FLT_PAR_ABS_PAN(fFMBPan, fmb_pan, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fFMBFX1Send, fmb_fx1, 4095.f, maxFXSendLevelDly)
    fFMBFX1Send *= fFMBFX1Send;
    MK_FLT_PAR_ABS(fFMBFX2Send, fmb_fx2, 4095.f, maxFXSendLevelRev)
    fFMBFX2Send *= fFMBFX2Send;

    if (!bFMBMute){
	    MK_BOOL_PAR(bFMBUseRatioMode, fmb_use_ratio_mode)
	    MK_BOOL_PAR(bFMBModEnvSync, fmb_mod_env_sync)
	    float fFMBF0 = fmb_f_b/4095.f * (200.f-20.f)+20.f;
    	if(cv_fmb_f_b != -1){
    		float fMod = data.cv[cv_fmb_f_b] * 5.f;
    		fMod = CTAG::SP::HELPERS::fastpow2(fMod);
    		fFMBF0 *= fMod;
    	}
	    MK_FLT_PAR_ABS_MIN_MAX(fFMBDecayBase, fmb_d_b, 4095.f, 0.001f, 1.f)
	    MK_FLT_PAR_ABS_MIN_MAX(fFMBFMod, fmb_f_m, 4095.f, 40.f, 2000.f)
    	MK_FLT_PAR_ABS(fFMBRatioModIndex, fmb_f_m, 4095.f, 63.f)
    	int iModIndex = static_cast<int>(fFMBRatioModIndex);
    	CONSTRAIN(iModIndex, 0, 63)
	    MK_FLT_PAR_ABS_MIN_MAX(fFMBI, fmb_I, 4095.f, 0.f, 10.f)
	    MK_FLT_PAR_ABS_MIN_MAX(fFMBDecayMod, fmb_d_m, 4095.f, 0.001f, .5f)
		MK_INT_PAR(iModFeedback, fmb_b_m, 16.f)
	    MK_FLT_PAR_ABS_MIN_MAX(fFMBAmpFreq, fmb_A_f, 4095.f, 0.f, 1000.f)
	    MK_FLT_PAR_ABS_MIN_MAX(fFMBDecayFreq, fmb_d_f, 4095.f, 0.001f, .1f)

	    fmb.params.use_ratio_mode = bFMBUseRatioMode;
	    fmb.params.mod_env_sync = bFMBModEnvSync;
	    fmb.params.f_b = fFMBF0;
	    fmb.params.d_b = fFMBDecayBase;
	    fmb.params.f_m = fFMBFMod;
    	fmb.params.mod_ratio_index = iModIndex;
	    fmb.params.I = fFMBI;
	    fmb.params.d_m = fFMBDecayMod;
	    fmb.params.b_m = static_cast<float>(iModFeedback);
	    fmb.params.A_f = fFMBAmpFreq;
	    fmb.params.d_f = fFMBDecayFreq;

	    fmb.Process(fmb_out, 32);
	    // data_ptrs[8] = fmb_out;
    }
    else{
	    // data_ptrs[8] = silence;
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

void ctagSoundProcessorDrumRackFMB::Init(std::size_t blockSize, void* blockPtr){
    // construct internal data model
    knowYourself();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);

    fmb.Init();
    std::fill_n(silence, 32, 0.f);

    // check if blockMem is large enough
    // blockMem is used just like larger blocks of heap memory
    // assert(blockSize >= memLen);
    // if memory larger than blockMem is needed, use heap_caps_malloc() instead with MALLOC_CAPS_SPIRAM
}

// no ctor, use Init() instead, is called from factory after successful creation
// dtor
ctagSoundProcessorDrumRackFMB::~ctagSoundProcessorDrumRackFMB(){
    // no explicit freeing for blockMem needed, done by ctagSPAllocator
    // explicit free is only needed when using heap_caps_malloc() with MALLOC_CAPS_SPIRAM
}

void ctagSoundProcessorDrumRackFMB::knowYourself(){
    // autogenerated code here
    // sectionCpp0
	pMapPar.emplace("fmb_trigger", [&](const int val){ fmb_trigger = val;});
	pMapTrig.emplace("fmb_trigger", [&](const int val){ trig_fmb_trigger = val;});
	pMapPar.emplace("fmb_mute", [&](const int val){ fmb_mute = val;});
	pMapTrig.emplace("fmb_mute", [&](const int val){ trig_fmb_mute = val;});
	// pMapPar.emplace("fmb_lev", [&](const int val){ fmb_lev = val;});
	// pMapCv.emplace("fmb_lev", [&](const int val){ cv_fmb_lev = val;});
	// pMapPar.emplace("fmb_pan", [&](const int val){ fmb_pan = val;});
	// pMapCv.emplace("fmb_pan", [&](const int val){ cv_fmb_pan = val;});
	// pMapPar.emplace("fmb_fx1", [&](const int val){ fmb_fx1 = val;});
	// pMapCv.emplace("fmb_fx1", [&](const int val){ cv_fmb_fx1 = val;});
	// pMapPar.emplace("fmb_fx2", [&](const int val){ fmb_fx2 = val;});
	// pMapCv.emplace("fmb_fx2", [&](const int val){ cv_fmb_fx2 = val;});
	pMapPar.emplace("fmb_use_ratio_mode", [&](const int val){ fmb_use_ratio_mode = val;});
	pMapTrig.emplace("fmb_use_ratio_mode", [&](const int val){ trig_fmb_use_ratio_mode = val;});
	pMapPar.emplace("fmb_mod_env_sync", [&](const int val){ fmb_mod_env_sync = val;});
	pMapTrig.emplace("fmb_mod_env_sync", [&](const int val){ trig_fmb_mod_env_sync = val;});
	pMapPar.emplace("fmb_f_b", [&](const int val){ fmb_f_b = val;});
	pMapCv.emplace("fmb_f_b", [&](const int val){ cv_fmb_f_b = val;});
	pMapPar.emplace("fmb_d_b", [&](const int val){ fmb_d_b = val;});
	pMapCv.emplace("fmb_d_b", [&](const int val){ cv_fmb_d_b = val;});
	pMapPar.emplace("fmb_f_m", [&](const int val){ fmb_f_m = val;});
	pMapCv.emplace("fmb_f_m", [&](const int val){ cv_fmb_f_m = val;});
	pMapPar.emplace("fmb_I", [&](const int val){ fmb_I = val;});
	pMapCv.emplace("fmb_I", [&](const int val){ cv_fmb_I = val;});
	pMapPar.emplace("fmb_d_m", [&](const int val){ fmb_d_m = val;});
	pMapCv.emplace("fmb_d_m", [&](const int val){ cv_fmb_d_m = val;});
	pMapPar.emplace("fmb_b_m", [&](const int val){ fmb_b_m = val;});
	pMapCv.emplace("fmb_b_m", [&](const int val){ cv_fmb_b_m = val;});
	pMapPar.emplace("fmb_A_f", [&](const int val){ fmb_A_f = val;});
	pMapCv.emplace("fmb_A_f", [&](const int val){ cv_fmb_A_f = val;});
	pMapPar.emplace("fmb_d_f", [&](const int val){ fmb_d_f = val;});
	pMapCv.emplace("fmb_d_f", [&](const int val){ cv_fmb_d_f = val;});
	isStereo = true;
	id = "DrumRackFMB";
	// sectionCpp0
}
