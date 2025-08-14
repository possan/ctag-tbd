#include "ctagSoundProcessorDrumRack.hpp"
#include "braids/quantizer_scales.h"

using namespace CTAG::SP;

// TODOs: fx return before compressor, stereo panning with delay -> when panned right, levels are lower, metallic sound of reverb.

#define maxFXSendLevelRev 1.5f

void ctagSoundProcessorDrumRack::mixRenderOutputMono(float *source, float level, float pan, float fx1, float fx2) {
    float mL = (1.0f - pan) * level;
    float mR = pan * level;
    float sL1 = mL * fx1;
    float sR1 = mR * fx1;
    float sL2 = mL * fx2;
    float sR2 = mR * fx2;

    for (int i = 0; i < 32; i++) {
        combined_out[i*2+0] += source[i] * mL;
        combined_out[i*2+1] += source[i] * mR;
        send1_out[i*2+0] += source[i] * sL1;
        send1_out[i*2+1] += source[i] * sR1;
        send2_out[i*2+0] += source[i] * sL2;
        send2_out[i*2+1] += source[i] * sR2;
    }
}

void ctagSoundProcessorDrumRack::mixRenderOutputStereo(float *source, float level, float pan, float fx1, float fx2) {
    float mL = (1.0f - pan) * level;
    float mR = pan * level;
    float sL1 = mL * fx1;
    float sR1 = mR * fx1;
    float sL2 = mL * fx2;
    float sR = mR * fx2;

    for (int i = 0; i < 32; i++) {
        combined_out[i*2+0] += source[i*2+0] * mL;
        combined_out[i*2+1] += source[i*2+1] * mR;
        send1_out[i*2+0] += source[i*2+0] * sL1;
        send1_out[i*2+1] += source[i*2+1] * sR1;
        send2_out[i*2+0] += source[i*2+0] * sL2;
        send2_out[i*2+1] += source[i*2+1] * sR;
    }
}

void ctagSoundProcessorDrumRack::preprocessFX1(const ProcessData& data) {
    MK_FLT_PAR_ABS(fBase, fx1_base, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fWidth, fx1_width, 4095.f, 1.f)
    bool bSync = fx1_sync;
    bool bSyncTrig {false};
    if(trig_fx1_sync != -1) bSyncTrig = data.trig[trig_fx1_sync] == 1 ? false : true;
    if(!bSync){
        fDelayTime = fx1_time_ms;
        if(cv_fx1_time_ms != -1) fDelayTime = fabsf(data.cv[cv_fx1_time_ms]) * 2000.f;
    }

    fBase = 20.f * stmlib::SemitonesToRatio(fBase * 120.f);
    fWidth = 20.f * stmlib::SemitonesToRatio(fWidth * 120.f);
    CONSTRAIN(fBase, 20.f, 20000.f)
    CONSTRAIN(fWidth, 50.f, 20000.f)
    float hp_cut = fBase;
    float lp_cut = fBase + fWidth;
    CONSTRAIN(lp_cut, 20.f, 20000.f)
    CONSTRAIN(hp_cut, 20.f, 20000.f)
    lp_l.set_f<stmlib::FREQUENCY_ACCURATE>(lp_cut / 44100.f);
    hp_l.set_f<stmlib::FREQUENCY_ACCURATE>(hp_cut / 44100.f);
    lp_r.copy_f(lp_l);
    hp_r.copy_f(hp_l);

    // sync mechanism
    if(bSyncTrig != pre_sync){
        pre_sync = bSyncTrig;
        if(bSyncTrig && bSync){
            int delta = timer - pre_timer;
            if(std::abs(delta) > 1){
                fDelayTime = static_cast<float>(timer) * 32.f / 44.1f;
            }
            pre_timer = timer;
            timer = 0;
        }
    }
    timer++;
}

void ctagSoundProcessorDrumRack::preprocessFX2(const ProcessData& data) {
    MK_FLT_PAR_ABS(fRevTime, fx2_time, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fReverbLPF, fx2_lp, 4095.f, 1.f)
    reverb.set_time(fRevTime);
    reverb.set_lp(fReverbLPF);
}

void ctagSoundProcessorDrumRack::preprocessMaster(const ProcessData& data) {
    MK_FLT_PAR_ABS_MIN_MAX(fCompThresdB, c_thres, 4095.f, -80.f, 0.f)
    sumCompressor.setThresh(fCompThresdB);
    MK_FLT_PAR_ABS_MIN_MAX(fCompAtk, c_atk, 4095.f, 0.3f, 30.f)
    sumCompressor.setAttack(fCompAtk);
    MK_FLT_PAR_ABS_MIN_MAX(fCompRel, c_rel, 4095.f, 40.f, 2000.f)
    sumCompressor.setRelease(fCompRel);
    MK_FLT_PAR_ABS_MIN_MAX(fCompRatio, c_ratio, 4095.f, 0.0001f, 1.25f)
    sumCompressor.setRatio(fCompRatio);
}
 
void ctagSoundProcessorDrumRack::renderMasterOutput(const ProcessData& data) {
    // delay
    MK_BOOL_PAR(bFreeze, fx1_freeze)
    MK_FLT_PAR_ABS(fDelayStereoWidth, fx1_st_width, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fDelayReverbSend, fx1_fx_send, 4095.f, maxFXSendLevelRev)
    fDelayReverbSend *= fDelayReverbSend;
    MK_FLT_PAR_ABS(fFeedback, fx1_feedback, 4095.f, 1.5f)
    MK_FLT_PAR_ABS(fDelayAmount, fx1_amount, 4095.f, 2.f)

    // reverb
    MK_FLT_PAR_ABS(fRevAmount, fx2_amount, 4095.f, 2.f)

    // sum compressor
    float buf_fx1_l[32], buf_fx1_r[32], buf_fx2[32];
    MK_BOOL_PAR(bTapeDigital, fx1_tape_digital)
    MK_BOOL_PAR(bSideChainLPF, c_lpf)
    MK_FLT_PAR_ABS_MIN_MAX(fCompMUPGain, c_gain, 4095.f, 0.f, 60.f) // in dB
    if (fCompMUPGain != fCompMUPGain_pre){
        fCompMUPGain = chunkware_simple::dB2lin(fCompMUPGain);
        fCompMUPGain_pre = fCompMUPGain;
    }
    MK_FLT_PAR_ABS_PAN(fCompMix, c_mix, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fCompDlyLevel, c_dly_level, 4095.f, 2.f)
    fCompDlyLevel *= fCompDlyLevel;
    MK_FLT_PAR_ABS(fCompRevLevel, c_rev_level, 4095.f, 2.f)
    fCompRevLevel *= fCompRevLevel;

    // overall mix
    MK_FLT_PAR_ABS(fMixLevel, sum_lev, 4095.f, 3.f)
    fMixLevel *= fMixLevel;

    // Render final buffer
    for (int i = 0; i < 32; i++){
        float fVal_l = combined_out[i * 2 + 0];
        float fVal_r = combined_out[i * 2 + 1];

        // FX1 models
        buf_fx1_l[i] = send1_out[i * 2 + 0];
        buf_fx1_r[i] = send1_out[i * 2 + 1];

        // FX2 models, reverb is mono in stereo out, but input buffer is stereo
        buf_fx2[i] = send2_out[i * 2 + 0];

        float dry_l = fVal_l;
        float dry_r = fVal_r;
        if (bSideChainLPF){
            ONE_POLE(side_l, fVal_l, 0.0005f);
            ONE_POLE(side_r, fVal_r, 0.0005f);
        }
        else{
            side_l = fVal_l;
            side_r = fVal_r;
        }
        side_l = fabsf(side_l);
        side_r = fabsf(side_r);
        float side = std::max(side_l, side_r);
        sumCompressor.process(fVal_l, fVal_r, side);
        fVal_l = fVal_l * fCompMUPGain * fCompMix + dry_l * (1.f - fCompMix);
        fVal_r = fVal_r * fCompMUPGain * fCompMix + dry_r * (1.f - fCompMix);
        data.buf[i * 2] = fVal_l * fMixLevel;
        data.buf[i * 2 + 1] = fVal_r * fMixLevel;
    }

    // fx buffers
    float dly_buf_l[32], dly_buf_r[32];
    float rev_buf_l[32], rev_buf_r[32];

    // delay
    CONSTRAIN(fDelayTime, 0.0001, 2000.f)
    float ofs = fDelayTime * 44.1f;
    if(fabsf(ofs - delayOffset) < 16) ofs = delayOffset;
    for(int i=0; i<32; i++){
        // Calculate the delay offset in samples
        if(delayOffset != ofs){
            if(bTapeDigital){
                if(ofs != delayOffset){
                    duck = 1.f;
                }
                delayOffset = ofs;
            } else {
                float temp = delayOffset;
                delayOffset = ONE_POLE(temp, ofs, 0.0001f);
            }
            readPos = static_cast<float>(writeIndex) - delayOffset;
            if(readPos < 0.f) readPos += float(delayBufferSizeMax);
            if(readPos >= float(delayBufferSizeMax)) readPos -= float(delayBufferSizeMax);
        }

        float inputSample_l = buf_fx1_l[i];
        float inputSample_r = buf_fx1_r[i];
        float outputSample_l, outputSample_r;

        outputSample_l = HELPERS::InterpolateWaveLinearWrap(delayBuffer_l, readPos, delayBufferSizeMax);
        outputSample_r = HELPERS::InterpolateWaveLinearWrap(delayBuffer_r, readPos, delayBufferSizeMax);
        readPos += 1.f;
        readPos > float(delayBufferSizeMax) ? readPos -= float(delayBufferSizeMax) : readPos;

        float temp = duck;
        duck = ONE_POLE(temp, 0.f, 0.35f)
        outputSample_l = outputSample_l * (1.f - duck);
        outputSample_r = outputSample_r * (1.f - duck);
        // Write the input sample to the delay buffer
        float out_l, out_r;
        if(!bFreeze){
            out_l = inputSample_l + fFeedback * ((1.f - fDelayStereoWidth) * outputSample_l + fDelayStereoWidth * outputSample_r);
            out_l = lp_l.Process<stmlib::FILTER_MODE_LOW_PASS>(out_l);
            out_l = hp_l.Process<stmlib::FILTER_MODE_HIGH_PASS>(out_l);
            out_r = (1.f - fDelayStereoWidth) * inputSample_r + fFeedback * ((1.f - fDelayStereoWidth) * outputSample_r + fDelayStereoWidth * outputSample_l);
            out_r = lp_r.Process<stmlib::FILTER_MODE_LOW_PASS>(out_r);
            out_r = hp_r.Process<stmlib::FILTER_MODE_HIGH_PASS>(out_r);
        }
        else{
            out_l = ((1.f - fDelayStereoWidth) * outputSample_l + fDelayStereoWidth * outputSample_r);
            out_r = ((1.f - fDelayStereoWidth) * outputSample_r + fDelayStereoWidth * outputSample_l);
        }

        delayBuffer_l[writeIndex] = stmlib::SoftLimit(out_l);
        delayBuffer_r[writeIndex] = stmlib::SoftLimit(out_r);
        writeIndex = (writeIndex + 1) % delayBufferSizeMax;

        // Mix the dry (input) and wet (delayed) signal
        dly_buf_l[i] = outputSample_l;
        dly_buf_r[i] = outputSample_r;
        rev_buf_l[i] = buf_fx2[i] + dly_buf_l[i] * fDelayReverbSend;
        rev_buf_r[i] = buf_fx2[i] + dly_buf_r[i] * fDelayReverbSend;
    }

    // reverb
    reverb.Process(rev_buf_l, rev_buf_r, 32);

    // add fx to sum
    fRevAmount *= fRevAmount;
    fDelayAmount *= fDelayAmount;
    for (int i = 0; i < 32; i++) {
        data.buf[i * 2] += rev_buf_l[i] * fRevAmount + dly_buf_l[i] * fDelayAmount;
        data.buf[i * 2 + 1] += rev_buf_r[i] * fRevAmount + dly_buf_r[i] * fDelayAmount;
    }
}

void ctagSoundProcessorDrumRack::Process(const ProcessData& data){
    memset(combined_out, 0, 32 * 2 * sizeof(float));
    memset(send1_out, 0, 32 * 2 * sizeof(float));
    memset(send2_out, 0, 32 * 2 * sizeof(float));

	struct DrumRackProcessData idata;
	idata.cv = data.cv;
	idata.trig = data.trig;
    idata.firstNonWtSlice = sampleRom.GetFirstNonWaveTableSlice();

    // process input first

    ch16.PreProcess(idata);
    if (ch16.enabled) {
        ch16_in.enabled = ch16.enabled && ch16.device == 0;
        // ch16_in.Process(idata); - it does nothing...
        if (ch1_db.enabled) {
            mixRenderOutputStereo(data.buf, ch16.level, ch16.pan, ch16.send1, ch16.send2);
        }
    }

    ch1.PreProcess(idata);
    if (ch1.enabled) {
        ch1_db.enabled = ch1.enabled && ch1.device == 0;
        ch1_db.Process(idata);
        if (ch1_db.enabled) {
            mixRenderOutputMono(ch1_db.dbd_out, ch1.level, ch1.pan, ch1.send1, ch1.send2);
        }

        ch1_ab.enabled = ch1.enabled && ch1.device == 1;
        ch1_ab.Process(idata);
        if (ch1_ab.enabled) {
            mixRenderOutputMono(ch1_ab.abd_out, ch1.level, ch1.pan, ch1.send1, ch1.send2);
        }
    }

    ch2.PreProcess(idata);
    if (ch2.enabled) {
        ch2_fmb1.enabled = ch2.enabled && ch2.device == 0;
        ch2_fmb1.Process(idata);
        if (ch2_fmb1.enabled) {
            mixRenderOutputMono(ch2_fmb1.fmb_out, ch2.level, ch2.pan, ch2.send1, ch2.send2);
        }

        ch2_fmb2.enabled = ch2.enabled && ch2.device == 1;
        ch2_fmb2.Process(idata);
        if (ch2_fmb2.enabled) {
            mixRenderOutputMono(ch2_fmb2.fmb_out, ch2.level, ch2.pan, ch2.send1, ch2.send2);
        }
    }

    ch3.PreProcess(idata);
    if (ch3.enabled) {
        ch3_ds.enabled = ch3.enabled && ch3.device == 0;
        ch3_ds.Process(idata);
        if (ch3_ds.enabled) {
            mixRenderOutputMono(ch3_ds.dsd_out, ch3.level, ch3.pan, ch3.send1, ch3.send2);
        }

        ch3_as.enabled = ch3.enabled && ch3.device == 1;
        ch3_as.Process(idata);
        if (ch3_as.enabled) {
            mixRenderOutputMono(ch3_as.asd_out, ch3.level, ch3.pan, ch3.send1, ch3.send2);
        }
    }

    ch4.PreProcess(idata);
    if (ch4.enabled) {
        ch4_hh1.enabled = ch4.enabled && ch4.device == 0;
        ch4_hh1.Process(idata);
        if (ch4_hh1.enabled) {
            mixRenderOutputMono(ch4_hh1.hh1_out, ch4.level, ch4.pan, ch4.send1, ch4.send2);
        }

        ch4_hh2.enabled = ch4.enabled && ch4.device == 1;
        ch4_hh2.Process(idata);
        if (ch4_hh2.enabled) {
            mixRenderOutputMono(ch4_hh2.hh2_out, ch4.level, ch4.pan, ch4.send1, ch4.send2);
        }
    }

    ch5.PreProcess(idata);
    if (ch5.enabled) {
        ch5_rs.enabled = ch5.enabled && ch5.device == 0;
        ch5_rs.Process(idata);
        if (ch5_rs.enabled) {
            mixRenderOutputMono(ch5_rs.rs_out, ch5.level, ch5.pan, ch5.send1, ch5.send2);
        }
    }

    ch6.PreProcess(idata);
    if (ch6.enabled) {
        ch6_cl.enabled = ch6.enabled && ch6.device == 0;
        ch6_cl.Process(idata);
        if (ch6_cl.enabled) {
            mixRenderOutputMono(ch6_cl.cl_out, ch6.level, ch6.pan, ch6.send1, ch6.send2);
        }
    }

    ch7.PreProcess(idata);
    if (ch7.enabled) {
        ch7_ro.enabled = ch7.enabled && ch7.device == 0;
        ch7_ro.Process(idata);
        if (ch7_ro.enabled) {
            mixRenderOutputMono(ch7_ro.s1_out, ch7.level, ch7.pan, ch7.send1, ch7.send2);
        }
    }

    ch8.PreProcess(idata);
    if (ch8.enabled) {
        ch8_ro.enabled = ch8.enabled && ch8.device == 0;
        ch8_ro.Process(idata);
        if (ch8_ro.enabled) {
            mixRenderOutputMono(ch8_ro.s1_out, ch8.level, ch8.pan, ch8.send1, ch8.send2);
        }
    }

    ch9.PreProcess(idata);
    if (ch9.enabled) {
        ch9_td3.enabled = ch9.enabled && ch9.device == 0;
        ch9_td3.Process(idata);
        if (ch9_td3.enabled) {
            mixRenderOutputMono(ch9_td3.td3_out, ch9.level, ch9.pan, ch9.send1, ch9.send2);
        }
    }

    ch10.PreProcess(idata);
    if (ch10.enabled) {
        ch10_td3.enabled = ch10.enabled && ch10.device == 0;
        ch10_td3.Process(idata);
        if (ch10_td3.enabled) {
            mixRenderOutputMono(ch10_td3.td3_out, ch10.level, ch10.pan, ch10.send1, ch10.send2);
        }
    }

    ch11.PreProcess(idata);
    if (ch11.enabled) {
        ch11_mo.enabled = ch11.enabled && ch11.device == 0;
        ch11_mo.Process(idata);
        if (ch11_mo.enabled) {
            mixRenderOutputMono(ch11_mo.mo_out, ch11.level, ch11.pan, ch11.send1, ch11.send2);
        }
    }

    ch12.PreProcess(idata);
    if (ch12.enabled) {
        ch12_mo.enabled = ch12.enabled && ch12.device == 0;
        ch12_mo.Process(idata);
        if (ch12_mo.enabled) {
            mixRenderOutputMono(ch12_mo.mo_out, ch12.level, ch12.pan, ch12.send1, ch12.send2);
        }
    }

    ch13.PreProcess(idata);
    if (ch13.enabled) {
        ch13_ro.enabled = ch13.enabled && ch13.device == 0;
        ch13_ro.Process(idata);
        if (ch13_ro.enabled) {
            mixRenderOutputMono(ch13_ro.s1_out, ch13.level, ch13.pan, ch13.send1, ch13.send2);
        }
    }

    ch14.PreProcess(idata);
    if (ch14.enabled) {
        ch14_ro.enabled = ch14.enabled && ch14.device == 0;
        ch14_ro.Process(idata);
        if (ch14_ro.enabled) {
            mixRenderOutputMono(ch14_ro.s1_out, ch14.level, ch14.pan, ch14.send1, ch14.send2);
        }
    }

    ch15.PreProcess(idata);
    if (ch15.enabled) {
        ch15_pp.enabled = ch15.enabled && ch15.device == 0;
        ch15_pp.Process(idata);
        if (ch15_pp.enabled) {
            mixRenderOutputStereo(ch15_pp.pp_out_stereo, ch15.level, ch15.pan, ch15.send1, ch15.send2);
        }
    }

    // Process effects
    preprocessFX1(data); // delay
    preprocessFX2(data); // reverb
    preprocessMaster(data); // sum compressor

    MK_BOOL_PAR(bSumMute, sum_mute)
    if (bSumMute){
        memset(data.buf, 0, 32 * 2 * sizeof(float));
        return;
    }

    renderMasterOutput(data);
}

void ctagSoundProcessorDrumRack::registerCv(const char *prefix, const char *suffix, function<DrumRackParameterSetter> setter){
    string fullId = string(prefix) + string(suffix);
    pMapCv.emplace(fullId, setter);
}

void ctagSoundProcessorDrumRack::registerParam(const char *prefix, const char *suffix, function<DrumRackParameterSetter> setter){
    string fullId = string(prefix) + string(suffix);
    pMapPar.emplace(fullId, setter);
}

void ctagSoundProcessorDrumRack::registerTrig(const char *prefix, const char *suffix, function<DrumRackParameterSetter> setter){
    string fullId = string(prefix) + string(suffix);
    pMapTrig.emplace(fullId, setter);
}


void ctagSoundProcessorDrumRack::Init(std::size_t blockSize, void* blockPtr){
    // construct internal data model

	printf("ctagSoundProcessorDrumRack::Init(%zu, %x)\n", blockSize, (uintptr_t) blockPtr);

    knowYourself();

    DrumRackInitData dri;
    dri.rack = this;
    // dri.allocator = [blockPtr, blockSize](std::size_t size) -> void* {
    //     void *ptr = static_cast<float*>(heap_caps_malloc(size, MALLOC_CAP_SPIRAM));
    //     return ptr;
    // };

    dri.prefix = "ch1_"; ch1.Init(&dri);
    dri.prefix = "ch1_db_"; ch1_db.Init(&dri);
    dri.prefix = "ch1_ab_"; ch1_ab.Init(&dri);

    dri.prefix = "ch2_"; ch2.Init(&dri);
    dri.prefix = "ch2_fmb1_"; ch2_fmb1.Init(&dri);
    dri.prefix = "ch2_fmb2_"; ch2_fmb2.Init(&dri);

    dri.prefix = "ch3_"; ch3.Init(&dri);
    dri.prefix = "ch3_ds_"; ch3_ds.Init(&dri);
    dri.prefix = "ch3_as_"; ch3_as.Init(&dri);

    dri.prefix = "ch4_"; ch4.Init(&dri);
    dri.prefix = "ch4_hh1_"; ch4_hh1.Init(&dri);
    dri.prefix = "ch4_hh2_"; ch4_hh2.Init(&dri);

    dri.prefix = "ch5_"; ch5.Init(&dri);
    dri.prefix = "ch5_rs_"; ch5_rs.Init(&dri);

    dri.prefix = "ch6_"; ch6.Init(&dri);
    dri.prefix = "ch6_cl_"; ch6_cl.Init(&dri);

    dri.prefix = "ch7_"; ch7.Init(&dri);
    dri.prefix = "ch7_smp_"; ch7_ro.Init(&dri);

    dri.prefix = "ch8_"; ch8.Init(&dri);
    dri.prefix = "ch8_smp_"; ch8_ro.Init(&dri);

    dri.prefix = "ch9_"; ch9.Init(&dri);
    dri.prefix = "ch9_tbd03_"; ch9_td3.Init(&dri);

    dri.prefix = "ch10_"; ch10.Init(&dri);
    dri.prefix = "ch10_tbd03_"; ch10_td3.Init(&dri);

    dri.prefix = "ch11_"; ch11.Init(&dri);
    dri.prefix = "ch11_mo_"; ch11_mo.Init(&dri);

    dri.prefix = "ch12_"; ch12.Init(&dri);
    dri.prefix = "ch12_mo_"; ch12_mo.Init(&dri); // WT OSC DUO later

    dri.prefix = "ch13_"; ch13.Init(&dri);
    dri.prefix = "ch13_smp_"; ch13_ro.Init(&dri);

    dri.prefix = "ch14_"; ch14.Init(&dri);
    dri.prefix = "ch14_smp_";ch14_ro.Init(&dri);

    dri.prefix = "ch15_"; ch15.Init(&dri);
    dri.prefix = "ch15_pp_"; ch15_pp.Init(&dri);

    dri.prefix = "ch16_"; ch16.Init(&dri);
    dri.prefix = "ch16_in_"; ch16_in.Init(&dri); // audio input, no prefix

    // dri.prefix = "fx1_";
    // fx_delay.Init(&dri);

    // dri.prefix = "fx2_";
    // fx_reverb.Init(&dri);

    // dri.prefix = "mmm_";
    // fx_master.Init(&dri);

    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);

    // delay
    delayBuffer_l = static_cast<float*>(heap_caps_malloc(delayBufferSizeMax * sizeof(float), MALLOC_CAP_SPIRAM));
    ESP_LOGI("ctagSoundProcessorDrumRack", "Allocate: delayBuffer_l=0x%x", (unsigned int)delayBuffer_l);
    assert(delayBuffer_l != nullptr);
    std::fill_n(delayBuffer_l, delayBufferSizeMax, 0.f);

    delayBuffer_r = static_cast<float*>(heap_caps_malloc(delayBufferSizeMax * sizeof(float), MALLOC_CAP_SPIRAM));
    ESP_LOGI("ctagSoundProcessorDrumRack", "Allocate: delayBuffer_r=0x%x", (unsigned int)delayBuffer_r);
    assert(delayBuffer_r != nullptr);
    std::fill_n(delayBuffer_r, delayBufferSizeMax, 0.f);

    // reverb
    reverbBuffer = static_cast<float*>(heap_caps_malloc(32768 * sizeof(float), MALLOC_CAP_SPIRAM));
    ESP_LOGI("ctagSoundProcessorDrumRack", "Allocate: reverbBuffer=0x%x", (unsigned int)reverbBuffer);
    assert(reverbBuffer != nullptr);
    // assert(blockSize >= 32768 * 4);
    reverb.Init(reverbBuffer); // requires 32768*4 bytes = 128KB
    reverb.Clear();
    // blockPtr = static_cast<void*>(static_cast<uint8_t*>(blockPtr) + 32768 * 4);
    // blockSize -= 32768 * 4;
    reverb.set_diffusion(0.7f);
    reverb.set_input_gain(.5f); // left and right are summed
    reverb.set_amount(1.f);
    reverb.set_lp(0.5f);
    reverb.set_time(0.4f);

    // preload samples
    sampleRom.BufferInSPIRAM();

    // init compressor
    sumCompressor.setSampleRate(44100.f);
    sumCompressor.initRuntime();
}

ctagSoundProcessorDrumRack::~ctagSoundProcessorDrumRack(){
}

void ctagSoundProcessorDrumRack::knowYourself(){
    // autogenerated code here
    // sectionCpp0
    
    pMapPar.emplace("fx1_time_ms", [&](const int val){ fx1_time_ms = val;});
	pMapCv.emplace("fx1_time_ms", [&](const int val){ cv_fx1_time_ms = val;});
	pMapPar.emplace("fx1_sync", [&](const int val){ fx1_sync = val;});
	pMapTrig.emplace("fx1_sync", [&](const int val){ trig_fx1_sync = val;});
	pMapPar.emplace("fx1_freeze", [&](const int val){ fx1_freeze = val;});
	pMapTrig.emplace("fx1_freeze", [&](const int val){ trig_fx1_freeze = val;});
	pMapPar.emplace("fx1_tape_digital", [&](const int val){ fx1_tape_digital = val;});
	pMapTrig.emplace("fx1_tape_digital", [&](const int val){ trig_fx1_tape_digital = val;});
	pMapPar.emplace("fx1_st_width", [&](const int val){ fx1_st_width = val;});
	pMapCv.emplace("fx1_st_width", [&](const int val){ cv_fx1_st_width = val;});
	pMapPar.emplace("fx1_fx_send", [&](const int val){ fx1_fx_send = val;});
	pMapCv.emplace("fx1_fx_send", [&](const int val){ cv_fx1_fx_send = val;});
	pMapPar.emplace("fx1_feedback", [&](const int val){ fx1_feedback = val;});
	pMapCv.emplace("fx1_feedback", [&](const int val){ cv_fx1_feedback = val;});
	pMapPar.emplace("fx1_base", [&](const int val){ fx1_base = val;});
	pMapCv.emplace("fx1_base", [&](const int val){ cv_fx1_base = val;});
	pMapPar.emplace("fx1_width", [&](const int val){ fx1_width = val;});
	pMapCv.emplace("fx1_width", [&](const int val){ cv_fx1_width = val;});

    pMapPar.emplace("fx2_time", [&](const int val){ fx2_time = val;});
	pMapCv.emplace("fx2_time", [&](const int val){ cv_fx2_time = val;});
	pMapPar.emplace("fx2_lp", [&](const int val){ fx2_lp = val;});
	pMapCv.emplace("fx2_lp", [&](const int val){ cv_fx2_lp = val;});

    pMapPar.emplace("c_thres", [&](const int val){ c_thres = val;});
	pMapCv.emplace("c_thres", [&](const int val){ cv_c_thres = val;});
	pMapPar.emplace("c_ratio", [&](const int val){ c_ratio = val;});
	pMapCv.emplace("c_ratio", [&](const int val){ cv_c_ratio = val;});
	pMapPar.emplace("c_atk", [&](const int val){ c_atk = val;});
	pMapCv.emplace("c_atk", [&](const int val){ cv_c_atk = val;});
	pMapPar.emplace("c_rel", [&](const int val){ c_rel = val;});
	pMapCv.emplace("c_rel", [&](const int val){ cv_c_rel = val;});
	pMapPar.emplace("c_lpf", [&](const int val){ c_lpf = val;});
	pMapTrig.emplace("c_lpf", [&](const int val){ trig_c_lpf = val;});
	pMapPar.emplace("c_gain", [&](const int val){ c_gain = val;});
	pMapCv.emplace("c_gain", [&](const int val){ cv_c_gain = val;});
	pMapPar.emplace("c_mix", [&](const int val){ c_mix = val;});
	pMapCv.emplace("c_mix", [&](const int val){ cv_c_mix = val;});
	pMapPar.emplace("c_dly_level", [&](const int val){ c_dly_level = val;});
	pMapCv.emplace("c_dly_level", [&](const int val){ cv_c_dly_level = val;});
	pMapPar.emplace("c_rev_level", [&](const int val){ c_rev_level = val;});
	pMapCv.emplace("c_rev_level", [&](const int val){ cv_c_rev_level = val;});

    pMapPar.emplace("sum_mute", [&](const int val){ sum_mute = val;});
	pMapTrig.emplace("sum_mute", [&](const int val){ trig_sum_mute = val;});
	pMapPar.emplace("sum_lev", [&](const int val){ sum_lev = val;});
	pMapCv.emplace("sum_lev", [&](const int val){ cv_sum_lev = val;});

    pMapPar.emplace("fx1_amount", [&](const int val){ fx1_amount = val;});
	pMapCv.emplace("fx1_amount", [&](const int val){ cv_fx1_amount = val;});

    pMapPar.emplace("fx2_amount", [&](const int val){ fx2_amount = val;});
	pMapCv.emplace("fx2_amount", [&](const int val){ cv_fx2_amount = val;});

    isStereo = true;
	id = "DrumRack";
	// sectionCpp0
}
