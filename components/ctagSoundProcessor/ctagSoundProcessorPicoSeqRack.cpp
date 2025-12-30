#include "ctagSoundProcessorPicoSeqRack.hpp"
#include "braids/quantizer_scales.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_cpu.h"
#include "esp_timer.h"
// #include "freertos/FreeRTOS.h"

using namespace CTAG::SP;

// TODOs: fx return before compressor, stereo panning with delay -> when panned right, levels are lower, metallic sound of reverb.

#define maxFXSendLevelRev 1.5f

void ctagSoundProcessorPicoSeqRack::mixRenderOutputMono(float *source, float level, float pan, float fx1, float fx2) {
    float mL = (1.0f - pan) * level;
    float mR = (1.0f + pan) * level;

    CONSTRAIN(mL, 0.0f, 1.f);
    CONSTRAIN(mR, 0.0f, 1.f);

    float sL1 = mL * fx1;
    float sR1 = mR * fx1;
    float sL2 = mL * fx2;
    float sR2 = mR * fx2;

    for (int i = 0; i < bufSz; i++) {
        combined_out[i*2+0] += source[i] * mL;
        combined_out[i*2+1] += source[i] * mR;
        send1_out[i*2+0] += source[i] * sL1;
        send1_out[i*2+1] += source[i] * sR1;
        send2_out[i*2+0] += source[i] * sL2;
        send2_out[i*2+1] += source[i] * sR2;
    }
}

void ctagSoundProcessorPicoSeqRack::mixRenderOutputStereo(float *source, float level, float pan, float fx1, float fx2) {
    float mL = (1.0f - pan) * level;
    float mR = (1.0f + pan) * level;

    CONSTRAIN(mL, 0.0f, 1.f);
    CONSTRAIN(mR, 0.0f, 1.f);

    float sL1 = mL * fx1;
    float sR1 = mR * fx1;
    float sL2 = mL * fx2;
    float sR2 = mR * fx2;

    for (int i = 0; i < bufSz; i++) {
        combined_out[i*2+0] += source[i*2+0] * mL;
        combined_out[i*2+1] += source[i*2+1] * mR;
        send1_out[i*2+0] += source[i*2+0] * sL1;
        send1_out[i*2+1] += source[i*2+1] * sR1;
        send2_out[i*2+0] += source[i*2+0] * sL2;
        send2_out[i*2+1] += source[i*2+1] * sR2;
    }
}

void ctagSoundProcessorPicoSeqRack::preprocessFX1(const ProcessData& data) {
    // int global_bpm_lo2 = global_bpm_lo / 32;
    // int global_bpm_hi2 = global_bpm_hi / 32;
    // int scaledbpm = global_bpm_lo2 + (global_bpm_hi2 << 7);
    int scaledbpm = data.sequencer_tempo / 10;
    if (scaledbpm < 32) scaledbpm = 32;
    if (scaledbpm != last_scaledbpm) {
        last_scaledbpm = scaledbpm;
        printf("Scaled BPM set to %1.1f\n",
            (float)scaledbpm/10.0f);
		last_msPerBeat = 60000.0f / ((float)(scaledbpm) / 10.0f);
    }

    // fDelayTime = fx1_time_ms / 10.0f; // TODO: Better calculation here
    float dt = fx1_time_ms / 32.0f; // 0-127
    dt *= last_msPerBeat;
    dt /= 8.0; // 8 per "1/16th note"
    dt *= 44100.0f;
    dt /= 1000.0f;
    CONSTRAIN(dt, 4.0f, 88200.f);
    int idt = (int)dt;
    if (idt != delaySamples) {
        delaySamples = idt;
        printf("Delay time set to %d samples (scaled BPM: %d)\n", idt, scaledbpm);
    }

    MK_FLT_PAR_ABS_NOCV(fBase, fx1_base, 4095.f, 1.f)
    MK_FLT_PAR_ABS_NOCV(fWidth, fx1_width, 4095.f, 1.f)
    bool bSync = fx1_sync;
    bool bSyncTrig {false};
    // if(trig_fx1_sync != -1) bSyncTrig = data.trig[trig_fx1_sync] == 1 ? false : true;
    // if(!bSync){
    // if(cv_fx1_time_ms != -1) fDelayTime = fabsf(data.cv[cv_fx1_time_ms]) * 2000.f;
    // }

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
    timer++;
}

void ctagSoundProcessorPicoSeqRack::preprocessFX2(const ProcessData& data) {
    MK_FLT_PAR_ABS_NOCV(fRevTime, fx2_time, 4095.f, 1.f)
    MK_FLT_PAR_ABS_NOCV(fReverbLPF, fx2_lp, 4095.f, 1.f)
    reverb.set_time(fRevTime);
    reverb.set_lp(fReverbLPF);
}

void ctagSoundProcessorPicoSeqRack::preprocessMaster(const ProcessData& data) {
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(fCompThresdB, c_thres, 4095.f, -80.f, 0.f)
    sumCompressor.setThresh(fCompThresdB);
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(fCompAtk, c_atk, 4095.f, 0.3f, 30.f)
    sumCompressor.setAttack(fCompAtk);
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(fCompRel, c_rel, 4095.f, 40.f, 2000.f)
    sumCompressor.setRelease(fCompRel);
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(fCompRatio, c_ratio, 4095.f, 0.0001f, 1.25f)
    sumCompressor.setRatio(fCompRatio);
}

void ctagSoundProcessorPicoSeqRack::renderMasterOutput(const ProcessData& data) {
    // delay
    MK_BOOL_PAR_NOCV(bFreeze, fx1_freeze)
    MK_FLT_PAR_ABS_NOCV(fDelayStereoWidth, fx1_st_width, 4095.f, 1.f)
    MK_FLT_PAR_ABS_NOCV(fDelayReverbSend, fx1_fx_send, 4095.f, maxFXSendLevelRev)
    fDelayReverbSend *= fDelayReverbSend;
    MK_FLT_PAR_ABS_NOCV(fFeedback, fx1_feedback, 4095.f, 1.5f)
    MK_FLT_PAR_ABS_NOCV(fDelayAmount, fx1_amount, 4095.f, 2.f)

    // reverb
    MK_FLT_PAR_ABS_NOCV(fRevAmount, fx2_amount, 4095.f, 2.f)

    // sum compressor
    float buf_fx1_l[BUF_SZ], buf_fx1_r[BUF_SZ], buf_fx2[BUF_SZ];
    MK_BOOL_PAR_NOCV(bTapeDigital, fx1_tape_digital)
    MK_BOOL_PAR_NOCV(bSideChainLPF, c_lpf)
    MK_FLT_PAR_ABS_MIN_MAX_NOCV(fCompMUPGain, c_gain, 4095.f, 0.f, 60.f) // in dB
    if (fCompMUPGain != fCompMUPGain_pre){
        fCompMUPGain = chunkware_simple::dB2lin(fCompMUPGain);
        fCompMUPGain_pre = fCompMUPGain;
    }
    MK_FLT_PAR_ABS_PAN_NOCV(fCompMix, c_mix, 4095.f, 1.f)
    MK_FLT_PAR_ABS_NOCV(fCompDlyLevel, c_dly_level, 4095.f, 2.f)
    fCompDlyLevel *= fCompDlyLevel;
    MK_FLT_PAR_ABS_NOCV(fCompRevLevel, c_rev_level, 4095.f, 2.f)
    fCompRevLevel *= fCompRevLevel;

    // overall mix
    MK_FLT_PAR_ABS_NOCV(fMixLevel, sum_lev, 4095.f, 3.f)
    fMixLevel *= fMixLevel;

    // Render final buffer
    for (int i = 0; i < bufSz; i++){
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
    float dly_buf_l[BUF_SZ], dly_buf_r[BUF_SZ];
    float rev_buf_l[BUF_SZ], rev_buf_r[BUF_SZ];

    // delay
    // CONSTRAIN(delaySamples, 4.0f, 88200.f);
    float ofs = delaySamples;
    if(fabsf(ofs - delayOffset) < 16) ofs = delayOffset;
    // if (obs < 16) {
    //     obs = 16;
    // }
    for(int i=0; i<bufSz; i++){
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
    reverb.Process(rev_buf_l, rev_buf_r, bufSz);

    // add fx to sum
    fRevAmount *= fRevAmount;
    fDelayAmount *= fDelayAmount;
    for (int i = 0; i < bufSz; i++) {
        data.buf[i * 2] += rev_buf_l[i] * fRevAmount + dly_buf_l[i] * fDelayAmount;
        data.buf[i * 2 + 1] += rev_buf_r[i] * fRevAmount + dly_buf_r[i] * fDelayAmount;
    }
}

void ctagSoundProcessorPicoSeqRack::Process(const ProcessData& data){
    framecounter ++;

    // TODO: process midi in data.midibytes here
    parseIncomingMidiMessages(data.midibytes, N_MIDIBYTES);

    std::fill_n(combined_out, bufSz * 2, 0.f);
    std::fill_n(send1_out, bufSz * 2, 0.f);
    std::fill_n(send2_out, bufSz * 2, 0.f);

	struct PicoSeqRackProcessData idata;
    idata.firstNonWtSlice = sampleRom.GetFirstNonWaveTableSlice();
    idata.sampleRom = &sampleRom;
    idata.tempo = data.sequencer_tempo;
    idata.quantum = data.sequencer_quantum;
    idata.msPerBeat = last_msPerBeat;

    // process input first


    int16_t T2 = esp_timer_get_time();
    // ch16.PreProcess(idata);
    // if (ch16.enabled) {
    //     ch16_in.enabled = ch16.enabled && ch16.device == 0;
    //     // ch16_in.Process(idata); - it does nothing...
    //     if (ch16_in.enabled) {
    //         mixRenderOutputStereo(data.buf, ch16.level, ch16.pan, ch16.send1, ch16.send2);
    //     }
    // }

    int16_t T = esp_timer_get_time();
    ch16_render_time = T2 - T;

    ch1.PreProcess(idata);
    if (ch1.enabled) {
        ch1_db.enabled = ch1.enabled && ch1.device == 0;
        ch1_db.Process(idata);
        if (ch1_db.enabled) {
            mixRenderOutputMono(ch1_db.out, ch1.level, ch1.pan, ch1.send1, ch1.send2);
        }

        ch1_ab.enabled = ch1.enabled && ch1.device == 1;
        ch1_ab.Process(idata);
        if (ch1_ab.enabled) {
            mixRenderOutputMono(ch1_ab.out, ch1.level, ch1.pan, ch1.send1, ch1.send2);
        }
    }

    T2 = esp_timer_get_time();
    ch1_render_time = T2 - T;

    ch2.PreProcess(idata);
    if (ch2.enabled) {
        ch2_fmb1.enabled = ch2.enabled && ch2.device == 0;
        ch2_fmb1.Process(idata);
        if (ch2_fmb1.enabled) {
            mixRenderOutputMono(ch2_fmb1.out, ch2.level, ch2.pan, ch2.send1, ch2.send2);
        }

        ch2_fmb2.enabled = ch2.enabled && ch2.device == 1;
        ch2_fmb2.Process(idata);
        if (ch2_fmb2.enabled) {
            mixRenderOutputMono(ch2_fmb2.out, ch2.level, ch2.pan, ch2.send1, ch2.send2);
        }
    }

    T = esp_timer_get_time();
    ch2_render_time = T - T2;


    ch3.PreProcess(idata);
    if (ch3.enabled) {
        ch3_ds.enabled = ch3.enabled && ch3.device == 0;
        ch3_ds.Process(idata);
        if (ch3_ds.enabled) {
            mixRenderOutputMono(ch3_ds.out, ch3.level, ch3.pan, ch3.send1, ch3.send2);
        }

        ch3_as.enabled = ch3.enabled && ch3.device == 1;
        ch3_as.Process(idata);
        if (ch3_as.enabled) {
            mixRenderOutputMono(ch3_as.out, ch3.level, ch3.pan, ch3.send1, ch3.send2);
        }
    }

    T2 = esp_timer_get_time();
    ch3_render_time = T2 - T;

    ch4.PreProcess(idata);
    if (ch4.enabled) {
        ch4_hh1.enabled = ch4.enabled && ch4.device == 0;
        ch4_hh1.Process(idata);
        if (ch4_hh1.enabled) {
            mixRenderOutputMono(ch4_hh1.out, ch4.level, ch4.pan, ch4.send1, ch4.send2);
        }

        ch4_hh2.enabled = ch4.enabled && ch4.device == 1;
        ch4_hh2.Process(idata);
        if (ch4_hh2.enabled) {
            mixRenderOutputMono(ch4_hh2.out, ch4.level, ch4.pan, ch4.send1, ch4.send2);
        }
    }

    T = esp_timer_get_time();
    ch4_render_time = T - T2;


    ch5.PreProcess(idata);
    if (ch5.enabled) {
        ch5_rs.enabled = ch5.enabled && ch5.device == 0;
        ch5_rs.Process(idata);
        if (ch5_rs.enabled) {
            mixRenderOutputMono(ch5_rs.rs_out, ch5.level, ch5.pan, ch5.send1, ch5.send2);
        }
    }

    T2 = esp_timer_get_time();
    ch5_render_time = T2 - T;

    ch6.PreProcess(idata);
    if (ch6.enabled) {
        ch6_cl.enabled = ch6.enabled && ch6.device == 0;
        ch6_cl.Process(idata);
        if (ch6_cl.enabled) {
            mixRenderOutputMono(ch6_cl.out, ch6.level, ch6.pan, ch6.send1, ch6.send2);
        }
    }

    T = esp_timer_get_time();
    ch6_render_time = T - T2;

    ch7.PreProcess(idata);
    if (ch7.enabled) {
        ch7_ro.enabled = ch7.enabled && ch7.device == 0;
        ch7_ro.track_length = ch7.track_length;
        ch7_ro.Process(idata);
        if (ch7_ro.enabled) {
            mixRenderOutputMono(ch7_ro.s1_out, ch7.level, ch7.pan, ch7.send1, ch7.send2);
        }
    }

    T2 = esp_timer_get_time();
    ch7_render_time = T2 - T;

    ch8.PreProcess(idata);
    if (ch8.enabled) {
        ch8_ro.enabled = ch8.enabled && ch8.device == 0;
        ch8_ro.track_length = ch8.track_length;
        ch8_ro.Process(idata);
        if (ch8_ro.enabled) {
            mixRenderOutputMono(ch8_ro.s1_out, ch8.level, ch8.pan, ch8.send1, ch8.send2);
        }
    }

    T = esp_timer_get_time();
    ch8_render_time = T - T2;



    ch9.PreProcess(idata);
    // if (ch9.enabled) {
    //     ch9_td3.enabled = ch9.enabled && ch9.device == 0;
    //     ch9_td3.Process(idata);
    //     if (ch9_td3.enabled) {
    //         mixRenderOutputMono(ch9_td3.td3_out, ch9.level, ch9.pan, ch9.send1, ch9.send2);
    //     }
    // }

    T2 = esp_timer_get_time();
    ch9_render_time = T2 - T;

    ch10.PreProcess(idata);
    // if (ch10.enabled) {
    //     ch10_td3.enabled = ch10.enabled && ch10.device == 0;
    //     ch10_td3.Process(idata);
    //     if (ch10_td3.enabled) {
    //         mixRenderOutputMono(ch10_td3.td3_out, ch10.level, ch10.pan, ch10.send1, ch10.send2);
    //     }
    // }

    T = esp_timer_get_time();
    ch10_render_time = T - T2;

    ch11.PreProcess(idata);
    if (ch11.enabled) {
        ch11_mo.enabled = ch11.enabled && ch11.device == 0;
        ch11_mo.Process(idata);
        if (ch11_mo.enabled) {
            mixRenderOutputMono(ch11_mo.mo_out, ch11.level, ch11.pan, ch11.send1, ch11.send2);
        }
    }

    T2 = esp_timer_get_time();
    ch11_render_time = T2 - T;

    ch12.PreProcess(idata);
    if (ch12.enabled) {
        ch12_wtosc.enabled = ch12.enabled && ch12.device == 0;
        ch12_wtosc.Process(idata);
        if (ch12_wtosc.enabled) {
            mixRenderOutputMono(ch12_wtosc.out, ch12.level, ch12.pan, ch12.send1, ch12.send2);
        }

        ch12_mo.enabled = ch12.enabled && ch12.device == 1;
        ch12_mo.Process(idata);
        if (ch12_mo.enabled) {
            mixRenderOutputMono(ch12_mo.mo_out, ch12.level, ch12.pan, ch12.send1, ch12.send2);
        }
    }

    T = esp_timer_get_time();
    ch12_render_time = T - T2;

    ch13.PreProcess(idata);
    if (ch13.enabled) {
        ch13_ro.enabled = ch13.enabled && ch13.device == 0;
        ch13_ro.track_length = ch13.track_length;
        ch13_ro.Process(idata);
        if (ch13_ro.enabled) {
            mixRenderOutputMono(ch13_ro.s1_out, ch13.level, ch13.pan, ch13.send1, ch13.send2);
        }
    }

    T2 = esp_timer_get_time();
    ch13_render_time = T2 - T;

    ch14.PreProcess(idata);
    if (ch14.enabled) {
        ch14_ro.enabled = ch14.enabled && ch14.device == 0;
        ch14_ro.track_length = ch14.track_length;
        ch14_ro.Process(idata);
        if (ch14_ro.enabled) {
            mixRenderOutputMono(ch14_ro.s1_out, ch14.level, ch14.pan, ch14.send1, ch14.send2);
        }
    }

    T = esp_timer_get_time();
    ch14_render_time = T - T2;

    ch15.PreProcess(idata);
    if (ch15.enabled) {
        ch15_pp.enabled = ch15.enabled && ch15.device == 0;
        ch15_pp.Process(idata);
        if (ch15_pp.enabled) {
            mixRenderOutputStereo(ch15_pp.pp_out_stereo, ch15.level, ch15.pan, ch15.send1, ch15.send2);
        }
    }

    T2 = esp_timer_get_time();
    ch15_render_time = T2 - T;

    // Process effects
    preprocessFX1(data); // delay

    T = esp_timer_get_time(); 
    fx_delay_render_time = T - T2; 
    
    preprocessFX2(data); // reverb

    T2 = esp_timer_get_time();
    fx_reverb_render_time = T2 - T;

    preprocessMaster(data); // sum compressor

    T = esp_timer_get_time(); 
    fx_master_render_time = T - T2;

    MK_BOOL_PAR_NOCV(bSumMute, sum_mute)
    if (bSumMute){
        memset(data.buf, 0, bufSz * 2 * sizeof(float));
        return;
    }

    T = esp_timer_get_time();

    renderMasterOutput(data);

    // if audio somehow ends up in NaN, reset buffers...
    if (data.buf[0] != data.buf[0]) {
        std::fill_n(data.buf, bufSz * 2, 0.f);
        std::fill_n(combined_out, bufSz * 2, 0.f);
        std::fill_n(send1_out, bufSz * 2, 0.f);
        std::fill_n(send2_out, bufSz * 2, 0.f);
        std::fill_n(delayBuffer_l, delayBufferSizeMax, 0.f);
        std::fill_n(delayBuffer_r, delayBufferSizeMax, 0.f);
        std::fill_n(reverbBuffer, 32768, 0.f);
    }

    if (framecounter % 2000 == 0) {
        printf("PicoSeqRack CPU times (us): Ch1:%d Ch2:%d Ch3:%d Ch4:%d Ch5:%d Ch6:%d Ch7:%d Ch8:%d Ch9:%d Ch10:%d Ch11:%d Ch12:%d Ch13:%d Ch14:%d Ch15:%d Ch16:%d FX1:%d FX2:%d Master:%d\n",
        (int)ch1_render_time, (int)ch2_render_time, (int)ch3_render_time, (int)ch4_render_time,
        (int)ch5_render_time, (int)ch6_render_time, (int)ch7_render_time, (int)ch8_render_time,
        (int)ch9_render_time, (int)ch10_render_time, (int)ch11_render_time, (int)ch12_render_time,
        (int)ch13_render_time, (int)ch14_render_time, (int)ch15_render_time, (int)ch16_render_time,
        (int)fx_delay_render_time, (int)fx_reverb_render_time, (int)fx_master_render_time);
    }
}

void ctagSoundProcessorPicoSeqRack::registerParam(const char *prefix, const char *suffix, function<DrumRackParameterSetter> setter){
    string fullId = string(prefix) + string(suffix);
    pMapPar.emplace(fullId, setter);
}

void ctagSoundProcessorPicoSeqRack::registerParamAndCC(const PickSeqRackInitData *initdata, const char *suffix, int cc, function<DrumRackParameterSetter> setter){
    string fullId = string(initdata->prefix) + string(suffix);
    pMapPar.emplace(fullId, setter);
    if (cc != -1) {
        pMapCC.emplace(CC_TO_MAP_KEY(initdata->midi_channel, initdata->cc_base + cc), fullId);
    }
}

void ctagSoundProcessorPicoSeqRack::registerParam(const PickSeqRackInitData *initdata, const char *suffix, function<DrumRackParameterSetter> setter){
    string fullId = string(initdata->prefix) + string(suffix);
    pMapPar.emplace(fullId, setter);
}

void ctagSoundProcessorPicoSeqRack::handleMidiNoteOff(const uint8_t channel, const uint8_t note, const uint8_t vel) {
    // ESP_LOGI("ctagSoundProcessorPicoSeqRack", "MIDI: note off %d, %d, %d", channel, note, vel);

    if (channel == 0) {
        if (ch9_td3.enabled) {
            ch9_td3.noteOff(note, 0);
        }
    }

    if (channel == 1) {
        if (ch10_td3.enabled) {
            ch10_td3.noteOff(note, 0);
        }
    }

    if (channel == 2) {
        if (ch11_mo.enabled) {
            ch11_mo.noteOff(note, 0);
        }
    }

    if (channel == 3) {
        if (ch12_mo.enabled) {
            ch12_mo.noteOff(note, 0);
        }
        if (ch12_wtosc.enabled) {
            ch12_wtosc.noteOff(note, 0);
        }
    }

    if (channel == 4) {
        if (ch13_ro.enabled) {
            ch13_ro.noteOff(note, 0);
        }
    }

    if (channel == 5) {
        if (ch14_ro.enabled) {
            ch14_ro.noteOff(note, 0);
        }
    }

    if (channel == 6) {
        if (ch15_pp.enabled) {
            ch15_pp.noteOff(note, 0);
        }
    }

    if (channel == 9) {
        // most drum rack doesn't care about note offs'
    }

    if (channel == 11) {
        ch7_ro.noteOff(note, 0);
    }

    if (channel == 12) {
        ch8_ro.noteOff(note, 0);
    }
};

void ctagSoundProcessorPicoSeqRack::handleMidiNoteOn(const uint8_t channel, const uint8_t note, const uint8_t vel) {
    // ESP_LOGI("ctagSoundProcessorPicoSeqRack", "MIDI: note on %d, %d, %d", channel, note, vel);

    if (channel == 0) {
        if (ch9_td3.enabled) {
            if (vel == 0) {
                ch9_td3.noteOff(note, 0);
            } else {
                ch9_td3.noteOn(note, vel);
            }
        }
    }

    if (channel == 1) {
        if (ch10_td3.enabled) {
            if (vel == 0) {
                ch10_td3.noteOff(note, 0);
            } else {
                ch10_td3.noteOn(note, vel);
            }
        }
    }

    if (channel == 2) {
        if (ch11_mo.enabled) {
            if (vel == 0) {
                ch11_mo.noteOff(note, 0);
            } else {
                ch11_mo.noteOn(note, vel);
            }
        }
    }

    if (channel == 3) {
        if (ch12_mo.enabled) {
            if (vel == 0) {
                ch12_mo.noteOff(note, 0);
            } else {
                ch12_mo.noteOn(note, vel);
            }
        }
        if (ch12_wtosc.enabled) {
            if (vel == 0) {
                ch12_wtosc.noteOff(note, 0);
            } else {
                ch12_wtosc.noteOn(note, vel);
            }
        }
    }

    if (channel == 4) {
        if (ch13_ro.enabled) {
            if (vel == 0) {
                ch13_ro.noteOff(note, 0);
            } else {
                ch13_ro.noteOn(note, vel);
            }
        }
    }

    if (channel == 5) {
        if (ch14_ro.enabled) {
            if (vel == 0) {
                ch14_ro.noteOff(note, 0);
            } else {
                ch14_ro.noteOn(note, vel);
            }
        }
    }

    if (channel == 6) {
        if (ch15_pp.enabled) {
            if (vel == 0) {
                ch15_pp.noteOff(note, 0);
            } else {
                ch15_pp.noteOn(note, vel);
            }
        }
    }

    if (channel == 7) {
        // ch16 has no notes
    }

    if (channel == 9) {
        // drum rack doesn't care about note offs'

        if (note == 36) { // kick 1
            if (ch1_ab.enabled) {
                if (vel > 0) {
                    ch1_ab.trigger();
                }
            }
            if (ch1_db.enabled) {
                if (vel > 0) {
                    ch1_db.trigger();
                }
            }
        }
        else if (note == 37) { // kick 2
            if (ch2_fmb1.enabled) {
                if (vel > 0) {
                    ch2_fmb1.trigger();
                }
            }
        }
        else if (note == 38) { // snare
            if (ch3_as.enabled) {
                if (vel > 0) {
                    ch3_as.trigger();
                }
            }
            if (ch3_ds.enabled) {
                if (vel > 0) {
                    ch3_ds.trigger();
                }
            }
        }
        else if (note == 39) { // hat
            if (ch4_hh1.enabled) {
                if (vel > 0) {
                    ch4_hh1.trigger();
                }
            }
            if (ch4_hh2.enabled) {
                if (vel > 0) {
                    ch4_hh2.trigger();
                }
            }
        }
        else if (note == 40) { // rs
            if (ch5_rs.enabled) {
                if (vel > 0) {
                    ch5_rs.trigger();
                }
            }
        }
        else if (note == 41) { // clap
            if (ch6_cl.enabled) {
                if (vel > 0) {
                    ch6_cl.trigger();
                }
            }
        }
    }

    if (channel == 11) {
        if (ch7_ro.enabled) {
            if (vel == 0) {
                ch7_ro.noteOff(note, 0);
            } else {
                ch7_ro.noteOn(note, 127);
            }
        }
    }

    if (channel == 12) {
        if (ch8_ro.enabled) {
            if (vel == 0) {
                ch8_ro.noteOff(note, 0);
            } else {
                ch8_ro.noteOn(note, 127);
            }
        }
    }
};

void ctagSoundProcessorPicoSeqRack::handleMidiAftertouch(const uint8_t channel, const uint8_t note, const uint8_t vel) {
    ESP_LOGI("ctagSoundProcessorPicoSeqRack", "MIDI: aftertouch %d, %d, %d", channel, note, vel);
};

void ctagSoundProcessorPicoSeqRack::handleMidiControlChange(const uint8_t channel, const uint8_t control, const uint8_t value) {
    // ESP_LOGI("ctagSoundProcessorPicoSeqRack", "MIDI: CC %d, %d, %d", channel, control, value);

    int cv_value = value * 4096 / 128;
    int key = CC_TO_MAP_KEY(channel, control);
    auto it = pMapCC.find(key);
    if (it != pMapCC.end()) {
        // printf("CC%d, CH%d map to %s = %d (%d)\n", control, channel, it->second.c_str(), cv_value, value);
        auto it2 = pMapPar.find(it->second.c_str());
        if (it2 != pMapPar.end()) {
            (it2->second)(cv_value);
        }
    } else {
        printf("No CC mapping for CC %d, CH %d\n", control, channel);
    }
};

void ctagSoundProcessorPicoSeqRack::handleMidiPatchChange(const uint8_t channel, const uint8_t patch) {
    // override if needed
    ESP_LOGI("ctagSoundProcessorPicoSeqRack", "MIDI: patch change %d, %d", channel, patch);
};

void ctagSoundProcessorPicoSeqRack::handleMidiPitchBend(const uint8_t channel, const uint16_t bend) {
    // override if needed
    ESP_LOGI("ctagSoundProcessorPicoSeqRack", "MIDI: pitch bend %d, %d", channel, bend);
};

void ctagSoundProcessorPicoSeqRack::Init(std::size_t blockSize, void* blockPtr){
    // construct internal data model

	printf("ctagSoundProcessorPicoSeqRack::Init(%zu, %x)\n", blockSize, (uintptr_t) blockPtr);

    ESP_LOGI("ctagSoundProcessorPicoSeqRack", "Before know yourself");
    knowYourself();
    ESP_LOGI("ctagSoundProcessorPicoSeqRack", "After know yourself");

    framecounter = 0;

    PickSeqRackInitData dri;
    dri.rack = this;

    dri.midi_channel = 9;
    dri.cc_base = 0;
    dri.prefix = "ch1_"; ch1.Init(&dri);
    dri.prefix = "ch1_db_"; ch1_db.Init(&dri);
    dri.prefix = "ch1_ab_"; ch1_ab.Init(&dri);
    ch1_render_time = 0;

    dri.midi_channel = 9;
    dri.cc_base = 20;
    dri.prefix = "ch2_"; ch2.Init(&dri);
    dri.prefix = "ch2_fmb1_"; ch2_fmb1.Init(&dri);
    dri.prefix = "ch2_fbm2_"; ch2_fmb2.Init(&dri);
    ch2_render_time = 0;

    dri.midi_channel = 9;
    dri.cc_base = 40;
    dri.prefix = "ch3_"; ch3.Init(&dri);
    dri.prefix = "ch3_ds_"; ch3_ds.Init(&dri);
    dri.prefix = "ch3_as_"; ch3_as.Init(&dri);
    ch3_render_time = 0;

    dri.midi_channel = 9;
    dri.cc_base = 60;
    dri.prefix = "ch4_"; ch4.Init(&dri);
    dri.prefix = "ch4_hh1_"; ch4_hh1.Init(&dri);
    dri.prefix = "ch4_hh2_"; ch4_hh2.Init(&dri);
    ch4_render_time = 0;

    dri.midi_channel = 9;
    dri.cc_base = 80;
    dri.prefix = "ch5_"; ch5.Init(&dri);
    dri.prefix = "ch5_rs_"; ch5_rs.Init(&dri);
    ch5_render_time = 0;

    dri.midi_channel = 9;
    dri.cc_base = 100;
    dri.prefix = "ch6_"; ch6.Init(&dri);
    dri.prefix = "ch6_cl_"; ch6_cl.Init(&dri);
    ch6_render_time = 0;

    dri.midi_channel = 11;
    dri.cc_base = 0;
    dri.prefix = "ch7_"; ch7.Init(&dri);
    dri.prefix = "ch7_smp_"; ch7_ro.Init(&dri);
    ch7_render_time = 0;

    dri.midi_channel = 12;
    dri.cc_base = 0;
    dri.prefix = "ch8_"; ch8.Init(&dri);
    dri.prefix = "ch8_smp_"; ch8_ro.Init(&dri);
    ch8_render_time = 0;

    dri.midi_channel = 0;
    dri.cc_base = 0;
    dri.prefix = "ch9_"; ch9.Init(&dri);
    dri.prefix = "ch9_tbd03_"; ch9_td3.Init(&dri);
    ch9_render_time = 0;

    dri.midi_channel = 1;
    dri.cc_base = 0;
    dri.prefix = "ch10_"; ch10.Init(&dri);
    dri.prefix = "ch10_tbd03_"; ch10_td3.Init(&dri);
    ch10_render_time = 0;

    dri.midi_channel = 2;
    dri.cc_base = 0;
    dri.prefix = "ch11_"; ch11.Init(&dri);
    dri.prefix = "ch11_mo_"; ch11_mo.Init(&dri);
    ch11_render_time = 0;

    dri.midi_channel = 3;
    dri.cc_base = 0;
    dri.prefix = "ch12_"; ch12.Init(&dri);
    dri.prefix = "ch12_wtosc_"; ch12_wtosc.Init(&dri);
    dri.prefix = "ch12_mo_"; ch12_mo.Init(&dri);
    ch12_render_time = 0;

    dri.midi_channel = 4;
    dri.cc_base = 0;
    dri.prefix = "ch13_"; ch13.Init(&dri);
    dri.prefix = "ch13_smp_"; ch13_ro.Init(&dri);
    ch13_render_time = 0;

    dri.midi_channel = 5;
    dri.cc_base = 0;
    dri.prefix = "ch14_"; ch14.Init(&dri);
    dri.prefix = "ch14_smp_"; ch14_ro.Init(&dri);
    ch14_render_time = 0;

    dri.midi_channel = 6;
    dri.cc_base = 0;
    dri.prefix = "ch15_"; ch15.Init(&dri);
    dri.prefix = "ch15_pp_"; ch15_pp.Init(&dri);
    ch15_render_time = 0;

    dri.midi_channel = 7;
    dri.cc_base = 0;
    dri.prefix = "ch16_"; ch16.Init(&dri);
    dri.prefix = "ch16_in_"; ch16_in.Init(&dri); // audio input, no prefix
    ch16_render_time = 0;

    dri.prefix = "fx1_";
    fx_delay.Init(&dri);
    fx_delay_render_time = 0;

    dri.prefix = "fx2_";
    fx_reverb.Init(&dri);
    fx_reverb_render_time = 0;

    dri.prefix = "mmm_";
    fx_master.Init(&dri);
    fx_master_render_time = 0;


    // print out some stats.
    ESP_LOGI("ctagSoundProcessorPicoSeqRack", "DrumRack: number of parameters registered %d", pMapPar.size());
    int nn = 0;
    // auto it2 = pMapPar.begin();
    // do {
    //     ESP_LOGI("ctagSoundProcessorPicoSeqRack", "  %s", it2->first.c_str());
    //     ++it2;

    //     if (nn++ % 50 == 0) {
    //         taskYIELD();
    //     }
    // } while (it2 != pMapPar.end());

    ESP_LOGI("ctagSoundProcessorPicoSeqRack", "DrumRack: number of CC's registered %d", pMapCC.size());
    // auto it = pMapCC.begin();
    // do {
    //     int ch = (it->first >> 8) & 0xFF;
    //     int co = it->first & 0xFF;
    //     ESP_LOGI("ctagSoundProcessorPicoSeqRack", "  CH %d, CC %d => %s", ch, co, it->second.c_str());
    //     ++it;

    //     if (nn++ % 50 == 0) {
    //         taskYIELD();
    //     }
    // } while (it != pMapCC.end());

#ifdef TBD_SIM
    // do not load a preset 
#else
    model = std::make_unique<ctagSPDataModel>(id, isStereo);
    LoadPreset(0);
#endif

    // delay
    delayBuffer_l = static_cast<float*>(heap_caps_malloc(delayBufferSizeMax * sizeof(float), MALLOC_CAP_SPIRAM));
    ESP_LOGI("ctagSoundProcessorPicoSeqRack", "Allocate: delayBuffer_l=0x%x", (unsigned int)delayBuffer_l);
    assert(delayBuffer_l != nullptr);
    std::fill_n(delayBuffer_l, delayBufferSizeMax, 0.f);

    delayBuffer_r = static_cast<float*>(heap_caps_malloc(delayBufferSizeMax * sizeof(float), MALLOC_CAP_SPIRAM));
    ESP_LOGI("ctagSoundProcessorPicoSeqRack", "Allocate: delayBuffer_r=0x%x", (unsigned int)delayBuffer_r);
    assert(delayBuffer_r != nullptr);
    std::fill_n(delayBuffer_r, delayBufferSizeMax, 0.f);

    // reverb
    reverbBuffer = static_cast<float*>(heap_caps_malloc(32768 * sizeof(float), MALLOC_CAP_SPIRAM));
    ESP_LOGI("ctagSoundProcessorPicoSeqRack", "Allocate: reverbBuffer=0x%x", (unsigned int)reverbBuffer);
    assert(reverbBuffer != nullptr);
    std::fill_n(reverbBuffer, 32768, 0.f);

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

    // init compressor
    sumCompressor.setSampleRate(44100.f);
    sumCompressor.initRuntime();
}

ctagSoundProcessorPicoSeqRack::~ctagSoundProcessorPicoSeqRack(){
}

void ctagSoundProcessorPicoSeqRack::knowYourself(){
    // autogenerated code here
    // sectionCpp0

    pMapPar.emplace("fx1_time_ms", [&](const int val){ fx1_time_ms = val;});
    pMapCC.emplace(CC_TO_MAP_KEY(13, 20), "fx1_time_ms");

    pMapPar.emplace("fx1_sync", [&](const int val){ fx1_sync = val;});
    pMapCC.emplace(CC_TO_MAP_KEY(13, 21), "fx1_sync");

    pMapPar.emplace("fx1_freeze", [&](const int val){ fx1_freeze = val;});
    pMapCC.emplace(CC_TO_MAP_KEY(13, 22), "fx1_freeze");

    pMapPar.emplace("fx1_tape_digital", [&](const int val){ fx1_tape_digital = val;});
    pMapCC.emplace(CC_TO_MAP_KEY(13, 23), "fx1_tape_digital");

    pMapPar.emplace("fx1_st_width", [&](const int val){ fx1_st_width = val;});
    pMapCC.emplace(CC_TO_MAP_KEY(13, 24), "fx1_st_width");

    pMapPar.emplace("fx1_fx_send", [&](const int val){ fx1_fx_send = val;});
    pMapCC.emplace(CC_TO_MAP_KEY(13, 25), "fx1_fx_send");

    pMapPar.emplace("fx1_feedback", [&](const int val){ fx1_feedback = val;});
    pMapCC.emplace(CC_TO_MAP_KEY(13, 26), "fx1_feedback");

    pMapPar.emplace("fx1_base", [&](const int val){ fx1_base = val;});
    pMapCC.emplace(CC_TO_MAP_KEY(13, 27), "fx1_base");

    pMapPar.emplace("fx1_width", [&](const int val){ fx1_width = val;});
    pMapCC.emplace(CC_TO_MAP_KEY(13, 28), "fx1_width");

    pMapPar.emplace("fx1_amount", [&](const int val){ fx1_amount = val;});
    pMapCC.emplace(CC_TO_MAP_KEY(13, 29), "fx1_amount");



    pMapPar.emplace("fx2_time", [&](const int val){ fx2_time = val;});
    pMapCC.emplace(CC_TO_MAP_KEY(13, 40), "fx2_time");

    pMapPar.emplace("fx2_lp", [&](const int val){ fx2_lp = val;});
    pMapCC.emplace(CC_TO_MAP_KEY(13, 41), "fx2_lp");

    pMapPar.emplace("fx2_amount", [&](const int val){ fx2_amount = val;});
    pMapCC.emplace(CC_TO_MAP_KEY(13, 42), "fx2_amount");




    pMapPar.emplace("c_thres", [&](const int val){ c_thres = val;});
    pMapCC.emplace(CC_TO_MAP_KEY(13, 60), "c_thres");

    pMapPar.emplace("c_ratio", [&](const int val){ c_ratio = val;});
    pMapCC.emplace(CC_TO_MAP_KEY(13, 61), "c_ratio");

    pMapPar.emplace("c_atk", [&](const int val){ c_atk = val;});
    pMapCC.emplace(CC_TO_MAP_KEY(13, 62), "c_atk");

    pMapPar.emplace("c_rel", [&](const int val){ c_rel = val;});
    pMapCC.emplace(CC_TO_MAP_KEY(13, 63), "c_rel");

    pMapPar.emplace("c_lpf", [&](const int val){ c_lpf = val;});
    pMapCC.emplace(CC_TO_MAP_KEY(13, 64), "c_lpf");

    pMapPar.emplace("c_gain", [&](const int val){ c_gain = val;});
    pMapCC.emplace(CC_TO_MAP_KEY(13, 65), "c_gain");

    pMapPar.emplace("c_mix", [&](const int val){ c_mix = val;});
    pMapCC.emplace(CC_TO_MAP_KEY(13, 66), "c_mix");

    pMapPar.emplace("c_dly_level", [&](const int val){ c_dly_level = val;});
    pMapCC.emplace(CC_TO_MAP_KEY(13, 67), "c_dly_level");

    pMapPar.emplace("c_rev_level", [&](const int val){ c_rev_level = val;});
    pMapCC.emplace(CC_TO_MAP_KEY(13, 68), "c_rev_level");



    pMapPar.emplace("sum_mute", [&](const int val){ sum_mute = val;});
    pMapCC.emplace(CC_TO_MAP_KEY(13, 80), "sum_mute");

    pMapPar.emplace("sum_lev", [&](const int val){ sum_lev = val;});
    pMapCC.emplace(CC_TO_MAP_KEY(13, 81), "sum_lev");



    isStereo = true;
	id = "PicoSeqRack";
	// sectionCpp0
}


void ctagSoundProcessorPicoSeqRack::parseIncomingMidiMessages(const uint8_t *buf, const size_t len) {
    // ESP_LOGI("ctagSoundProcessorPicoSeqRack", "parseIncomingMidiMessages: 0x%x, %d", buf, len);
    // ESP_LOGI("ctagSoundProcessorPicoSeqRack", "parseIncomingMidiMessages: %02X %02X %02X %02X %02X %02X %02X %02X %02X (%d)",
    //     buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8], len);

    if (buf == nullptr || len < 1)  {
        return;
    }

    int left = len;
    int o = 0;

    while(left > 3) {
        uint8_t b0 = buf[o++];
        left --;

        if (b0 == 0) {
            // probably end of event stream
            return;
        }

        uint8_t channel = (b0 & 0x0F);
        uint8_t cmd = (b0 & 0xF0);

        switch(cmd) {
            case 0x80: // note off
            {
                if (left < 2) return; // not enough data

                uint8_t b1 = buf[o++];
                uint8_t b2 = buf[o++];
                left -= 2;
                handleMidiNoteOff(channel, b1, b2);
                break;
            }
            case 0x90: // note on
            {
                if (left < 2) return; // not enough data

                uint8_t b1 = buf[o++];
                uint8_t b2 = buf[o++];
                left -= 2;
                handleMidiNoteOn(channel, b1, b2);
                break;
            }
            case 0xA0: // aftertouch
            {
                if (left < 2) return; // not enough data

                uint8_t b1 = buf[o++];
                uint8_t b2 = buf[o++];
                left -= 2;
                handleMidiAftertouch(channel, b1, b2);
                break;
            }
            case 0xB0: // control change
            {
                if (left < 2) return; // not enough data

                uint8_t b1 = buf[o++];
                uint8_t b2 = buf[o++];
                left -= 2;
                handleMidiControlChange(channel, b1, b2);
                break;
            }
            case 0xC0: // program change
            {
                if (left < 2) return; // not enough data

                uint8_t b1 = buf[o++];
                uint8_t b2 = buf[o++]; // not used?
                left -= 2;
                handleMidiPatchChange(channel, b1);
                break;
            }
            case 0xE0: // pitch bend
            {
                if (left < 2) return; // not enough data

                uint8_t b1 = buf[o++];
                uint8_t b2 = buf[o++];
                left -= 2;
                handleMidiPitchBend(channel, b2 * 128 + b1);
                break;
            }
            case 0xF0: // system common / real time
                // TODO handle sysex, clock, start, stop, continue, ...
                switch(b0) {
                    case 0xF0: // sysex start
                        return; // just ignore and bail out for now

                    case 0xF8: // timing clock
                    case 0xFA: // start
                    case 0xFB: // continue
                    case 0xFC: // stop
                    case 0xFE: // active sensing
                    case 0xFF: // system reset
                        // ignore for now
                        break;
                }
                break; // fail for now
            default:
                // for anything else, just stop parsing.
                return;
        }
    }
}
