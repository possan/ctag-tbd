#include "ctagSoundProcessorDrumRackCL.hpp"

using namespace CTAG::SP;

// TODOs: fx return before compressor, stereo panning with delay -> when panned right, levels are lower, metallic sound of reverb.

void ctagSoundProcessorDrumRackCL::Process(const ProcessData& data){
    // const float maxFXSendLevelDly {4.f};
    // const float maxFXSendLevelRev {2.f};
     

    // clap
    MK_BOOL_PAR(bCLMute, cl_mute)
    MK_FLT_PAR_ABS(fCLLev, cl_lev, 4095.f, 2.f)
    fCLLev *= fCLLev;
    MK_FLT_PAR_ABS_PAN(fCLPan, cl_pan, 4095.f, 1.f)
    // MK_FLT_PAR_ABS(fCLFX1Send, cl_fx1, 4095.f, maxFXSendLevelDly)
    // fCLFX1Send *= fCLFX1Send;
    // MK_FLT_PAR_ABS(fCLFX2Send, cl_fx2, 4095.f, maxFXSendLevelRev)
    // fCLFX2Send *= fCLFX2Send;

    if (bCLMute) {
        memset(data.buf, 0, 32 * sizeof(float));
        // data_ptrs[7] = silence;
        return;
    }

    MK_FLT_PAR_ABS_MIN_MAX(cl_pitch1_, cl_f0, 4095.f, 350.f, 4000.f)
    MK_FLT_PAR_ABS_MIN_MAX(cl_pitch2_, cl_f0, 4095.f, 300.f, 3000.f)
    MK_FLT_PAR_ABS_MIN_MAX(cl_reso1_, cl_tone, 4095.f, 1.f, 2.5f)
    MK_FLT_PAR_ABS_MIN_MAX(cl_reso2_, cl_tone, 4095.f, 0.75f, 6.5f)
    MK_FLT_PAR_ABS_MIN_MAX(cl_decay1_, cl_decay, 4095.f, 0.05f, 0.3f)
    MK_FLT_PAR_ABS_MIN_MAX(cl_decay2_, cl_decay, 4095.f, 0.05f, 2.f)
    MK_FLT_PAR_ABS_MIN_MAX(cl_scale_attack_, cl_scale, 4095.f, 0.f, 0.1f)
    MK_FLT_PAR_ABS_MIN_MAX(cl_scale_trans, cl_scale, 4095.f, 1.f, 3.f)
    MK_INT_PAR_ABS(cl_trans_, cl_transient, 16)

    cl.params.pitch1 = cl_pitch1_ / 44100.f;
    cl.params.pitch2 = cl_pitch2_ / 44100.f;
    cl.params.reso1 = cl_reso1_;
    cl.params.reso2 = cl_reso2_;
    cl.params.decay1 = cl_decay1_;
    cl.params.decay2 = cl_decay2_;
    cl.params.attack = cl_scale_attack_;
    cl.params.scale = cl_scale_trans;
    cl.params.transient = cl_trans_ % 16;

    MK_BOOL_PAR(bCLTrig, cl_trigger)
    if (bCLTrig != cl_trig_prev && bCLTrig){
        cl_trig_prev = true;
        cl.Trigger();
    }
    else if (!bCLTrig){
        cl_trig_prev = false;
    }

    cl.Process(cl_out, 32);
    //     // data_ptrs[7] = cl_out;
    // }
    // else{
    //     // data_ptrs[7] = silence;
    // }

    memcpy(data.buf, cl_out, 32 * sizeof(float));
 
    // // delay
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

void ctagSoundProcessorDrumRackCL::Init(){
    // construct internal data model
    knowYourself();
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);

    cl.Init();
    std::fill_n(silence, 32, 0.f);

    // check if blockMem is large enough
    // blockMem is used just like larger blocks of heap memory
    // assert(blockSize >= memLen);
    // if memory larger than blockMem is needed, use heap_caps_malloc() instead with MALLOC_CAPS_SPIRAM
}

// no ctor, use Init() instead, is called from factory after successful creation
// dtor
ctagSoundProcessorDrumRackCL::~ctagSoundProcessorDrumRackCL(){
    // no explicit freeing for blockMem needed, done by ctagSPAllocator
    // explicit free is only needed when using heap_caps_malloc() with MALLOC_CAPS_SPIRAM
}

void ctagSoundProcessorDrumRackCL::knowYourself(){
    // autogenerated code here
    // sectionCpp0
	pMapPar.emplace("cl_trigger", [&](const int val){ cl_trigger = val;});
	pMapTrig.emplace("cl_trigger", [&](const int val){ trig_cl_trigger = val;});
	pMapPar.emplace("cl_mute", [&](const int val){ cl_mute = val;});
	pMapTrig.emplace("cl_mute", [&](const int val){ trig_cl_mute = val;});
	// pMapPar.emplace("cl_lev", [&](const int val){ cl_lev = val;});
	// pMapCv.emplace("cl_lev", [&](const int val){ cv_cl_lev = val;});
	// pMapPar.emplace("cl_pan", [&](const int val){ cl_pan = val;});
	// pMapCv.emplace("cl_pan", [&](const int val){ cv_cl_pan = val;});
	// pMapPar.emplace("cl_fx1", [&](const int val){ cl_fx1 = val;});
	// pMapCv.emplace("cl_fx1", [&](const int val){ cv_cl_fx1 = val;});
	// pMapPar.emplace("cl_fx2", [&](const int val){ cl_fx2 = val;});
	// pMapCv.emplace("cl_fx2", [&](const int val){ cv_cl_fx2 = val;});
	pMapPar.emplace("cl_f0", [&](const int val){ cl_f0 = val;});
	pMapCv.emplace("cl_f0", [&](const int val){ cv_cl_f0 = val;});
	pMapPar.emplace("cl_tone", [&](const int val){ cl_tone = val;});
	pMapCv.emplace("cl_tone", [&](const int val){ cv_cl_tone = val;});
	pMapPar.emplace("cl_decay", [&](const int val){ cl_decay = val;});
	pMapCv.emplace("cl_decay", [&](const int val){ cv_cl_decay = val;});
	pMapPar.emplace("cl_scale", [&](const int val){ cl_scale = val;});
	pMapCv.emplace("cl_scale", [&](const int val){ cv_cl_scale = val;});
	pMapPar.emplace("cl_transient", [&](const int val){ cl_transient = val;});
	pMapCv.emplace("cl_transient", [&](const int val){ cv_cl_transient = val;});
	isStereo = false;
	id = "DrumRackCL";
	// sectionCpp0
}
