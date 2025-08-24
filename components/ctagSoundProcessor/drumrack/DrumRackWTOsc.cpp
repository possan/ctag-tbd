#include "DrumRackSynth.hpp"
#include "DrumRackWTOsc.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"
#include "helpers/ctagNumUtil.hpp"
#include "plaits/dsp/engine/engine.h"
#include "braids/quantizer_scales.h"

using namespace CTAG::SP;

#define td3_kAccentDecay 0.5f
#define td3_kAccentVCAFactor 1.5f

void DrumRackWTOsc::Init(const DrumRackInitData *initdata) {
    lfo.SetSampleRate(44100.f / BUF_SZ);
    lfo.SetFrequency(1.f);

    buffer = static_cast<int16_t*>(heap_caps_malloc(260 * 64 * 2, MALLOC_CAP_SPIRAM));
    fbuffer = static_cast<float*>(heap_caps_malloc(512 * 4, MALLOC_CAP_SPIRAM));
    memset(buffer, 0, 260 * 64 * 2);
    memset(fbuffer, 0, 512 * 4);

    oscillator.Init();
    svf.Init();
    adsr.SetModeExp();
    adsr.SetSampleRate(44100.f / BUF_SZ);
    adsr.Reset();
    pitchQuantizer.Init();

    this->enabled = false;

	initdata->rack->registerParam(initdata->prefix, "gain", [&](const int val){ gain = val;});
	initdata->rack->registerCv(initdata->prefix, "gain", [&](const int val){ cv_gain = val;});
	initdata->rack->registerParam(initdata->prefix, "gate", [&](const int val){ gate = val;});
	initdata->rack->registerTrig(initdata->prefix, "gate", [&](const int val){ trig_gate = val;});
	initdata->rack->registerParam(initdata->prefix, "pitch", [&](const int val){ pitch = val;});
	initdata->rack->registerCv(initdata->prefix, "pitch", [&](const int val){ cv_pitch = val;});
	initdata->rack->registerParam(initdata->prefix, "q_scale", [&](const int val){ q_scale = val;});
	initdata->rack->registerCv(initdata->prefix, "q_scale", [&](const int val){ cv_q_scale = val;});
	initdata->rack->registerParam(initdata->prefix, "tune", [&](const int val){ tune = val;});
	initdata->rack->registerCv(initdata->prefix, "tune", [&](const int val){ cv_tune = val;});
	initdata->rack->registerParam(initdata->prefix, "wavebank", [&](const int val){ wavebank = val;});
	initdata->rack->registerCv(initdata->prefix, "wavebank", [&](const int val){ cv_wavebank = val;});
	initdata->rack->registerParam(initdata->prefix, "wave", [&](const int val){ wave = val;});
	initdata->rack->registerCv(initdata->prefix, "wave", [&](const int val){ cv_wave = val;});
	initdata->rack->registerParam(initdata->prefix, "fmode", [&](const int val){ fmode = val;});
	initdata->rack->registerCv(initdata->prefix, "fmode", [&](const int val){ cv_fmode = val;});
	initdata->rack->registerParam(initdata->prefix, "fcut", [&](const int val){ fcut = val;});
	initdata->rack->registerCv(initdata->prefix, "fcut", [&](const int val){ cv_fcut = val;});
	initdata->rack->registerParam(initdata->prefix, "freso", [&](const int val){ freso = val;});
	initdata->rack->registerCv(initdata->prefix, "freso", [&](const int val){ cv_freso = val;});
	initdata->rack->registerParam(initdata->prefix, "lfo2wave", [&](const int val){ lfo2wave = val;});
	initdata->rack->registerCv(initdata->prefix, "lfo2wave", [&](const int val){ cv_lfo2wave = val;});
	initdata->rack->registerParam(initdata->prefix, "lfo2am", [&](const int val){ lfo2am = val;});
	initdata->rack->registerCv(initdata->prefix, "lfo2am", [&](const int val){ cv_lfo2am = val;});
	initdata->rack->registerParam(initdata->prefix, "lfo2fm", [&](const int val){ lfo2fm = val;});
	initdata->rack->registerCv(initdata->prefix, "lfo2fm", [&](const int val){ cv_lfo2fm = val;});
	initdata->rack->registerParam(initdata->prefix, "lfo2filtfm", [&](const int val){ lfo2filtfm = val;});
	initdata->rack->registerCv(initdata->prefix, "lfo2filtfm", [&](const int val){ cv_lfo2filtfm = val;});
	initdata->rack->registerParam(initdata->prefix, "eg2wave", [&](const int val){ eg2wave = val;});
	initdata->rack->registerCv(initdata->prefix, "eg2wave", [&](const int val){ cv_eg2wave = val;});
	initdata->rack->registerParam(initdata->prefix, "eg2am", [&](const int val){ eg2am = val;});
	initdata->rack->registerCv(initdata->prefix, "eg2am", [&](const int val){ cv_eg2am = val;});
	initdata->rack->registerParam(initdata->prefix, "eg2fm", [&](const int val){ eg2fm = val;});
	initdata->rack->registerCv(initdata->prefix, "eg2fm", [&](const int val){ cv_eg2fm = val;});
	initdata->rack->registerParam(initdata->prefix, "eg2filtfm", [&](const int val){ eg2filtfm = val;});
	initdata->rack->registerCv(initdata->prefix, "eg2filtfm", [&](const int val){ cv_eg2filtfm = val;});
	initdata->rack->registerParam(initdata->prefix, "lfospeed", [&](const int val){ lfospeed = val;});
	initdata->rack->registerCv(initdata->prefix, "lfospeed", [&](const int val){ cv_lfospeed = val;});
	initdata->rack->registerParam(initdata->prefix, "lfosync", [&](const int val){ lfosync = val;});
	initdata->rack->registerTrig(initdata->prefix, "lfosync", [&](const int val){ trig_lfosync = val;});
	initdata->rack->registerParam(initdata->prefix, "egfasl", [&](const int val){ egfasl = val;});
	initdata->rack->registerTrig(initdata->prefix, "egfasl", [&](const int val){ trig_egfasl = val;});
	initdata->rack->registerParam(initdata->prefix, "attack", [&](const int val){ attack = val;});
	initdata->rack->registerCv(initdata->prefix, "attack", [&](const int val){ cv_attack = val;});
	initdata->rack->registerParam(initdata->prefix, "decay", [&](const int val){ decay = val;});
	initdata->rack->registerCv(initdata->prefix, "decay", [&](const int val){ cv_decay = val;});
	initdata->rack->registerParam(initdata->prefix, "sustain", [&](const int val){ sustain = val;});
	initdata->rack->registerCv(initdata->prefix, "sustain", [&](const int val){ cv_sustain = val;});
	initdata->rack->registerParam(initdata->prefix, "release", [&](const int val){ release = val;});
	initdata->rack->registerCv(initdata->prefix, "release", [&](const int val){ cv_release = val;});
}

void DrumRackWTOsc::Process(const DrumRackProcessData &data) {
    std::fill_n(out, BUF_SZ, 0.f);

    if (!this->enabled) {
        return;
    }

    // wave select
    currentBank = wavebank;
	if(cv_wave != -1) ONE_POLE(fWave, fabsf(data.cv[cv_wave]), 0.1f)
	else fWave = wave / 4095.f;


    if(lastBank != currentBank) { // this is slow, hence not modulated by CV
        prepareWavetables(data.sampleRom);
        lastBank = currentBank;
    }

    // gain
    MK_FLT_PAR_ABS(fGain, gain, 4095.f, 2.f)

    // adsr + adsr modulation
    MK_BOOL_PAR(bGate, gate)
    adsr.Gate(bGate);
    MK_BOOL_PAR(bEGSlow, egfasl)
    MK_FLT_PAR_ABS(fAttack, attack, 4095.f, 10.f)
    MK_FLT_PAR_ABS(fDecay, decay, 4095.f, 10.f)
    MK_FLT_PAR_ABS(fSustain, sustain, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fRelease, release, 4095.f, 10.f)
    if(bEGSlow){
        fAttack *= 30.f;
        fDecay *= 30.f;
        fRelease *= 30.f;
    }
    adsr.SetAttack(fAttack);
    adsr.SetDecay(fDecay);
    adsr.SetSustain(fSustain);
    adsr.SetRelease(fRelease);

    // adsr modulation
    MK_FLT_PAR_ABS_SFT(fEGAM, eg2am, 4095.f, 1.f)
    MK_FLT_PAR_ABS_SFT(fEGFM, eg2fm, 4095.f, 12.f)
    MK_FLT_PAR_ABS_SFT(fEGFMFilt, eg2filtfm, 4095.f, 1.f)
    MK_FLT_PAR_ABS_SFT(fEGWave, eg2wave, 4095.f, 1.f)
    valADSR = adsr.Process();

    // modulation LFO
    MK_FLT_PAR_ABS(fLFOSpeed, lfospeed, 4095.f, 20.f)
    MK_BOOL_PAR(bLFOSync, lfosync)
    bool trigger = preGate != bGate && bGate;

    if (trigger) {
        printf("WTOSC\n");
        if (bLFOSync) {
            lfo.SetFrequencyPhase(fLFOSpeed, 0.f);
        } else {
            lfo.SetFrequency(fLFOSpeed);
        }
    }

    preGate = bGate;

    MK_FLT_PAR_ABS(fLFOAM, lfo2am, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fLFOFM, lfo2fm, 4095.f, 12.f)
    MK_FLT_PAR_ABS(fLFOFMFilt, lfo2filtfm, 4095.f, 1.f);
    MK_FLT_PAR_ABS(fLFOWave, lfo2wave, 4095.f, 1.f)
    valLFO = lfo.Process();

    // // pitch / tuning / FM
    int32_t ipitch = pitch;
    ipitch += 48; // midi note * resolution
    ipitch *= 128;
    int32_t ipitch_root = ipitch;
    if (cv_pitch != -1) {
        ipitch += static_cast<int32_t>(data.cv[cv_pitch] * 12.f * 5.f * 128.f); // five octaves
    }
    int32_t sc = q_scale;
    if (cv_q_scale != -1) {
        sc = static_cast<int32_t>(fabsf(data.cv[cv_q_scale]) * 48.f);
        CONSTRAIN(sc, 0, 47);
    }
    //ESP_LOGE("WTOSC", "Scale %d", sc);
    pitchQuantizer.Configure(braids::scales[sc]);
    ipitch = pitchQuantizer.Process(ipitch, ipitch_root);

    float fPitch = static_cast<float>(ipitch);
    fPitch /= 128.f;
    MK_FLT_PAR_ABS_SFT(fTune, tune, 2048.f, 1.f)
    const float f0 = plaits::NoteToFrequency(fPitch + fTune * 12.f + fLFOFM * valLFO + fEGFM * valADSR) * 0.998f;

    // filter
    MK_FLT_PAR_ABS(fCut, fcut, 4095.f, 1.f)
    MK_FLT_PAR_ABS(fReso, freso, 4095.f, 20.f)
    // filter modulation
    fCut = fCut + fEGFMFilt * valADSR + fLFOFMFilt * valLFO; // TODO: Pitch tracking
    // limit values
    CONSTRAIN(fCut, 0.f, 1.f)
    CONSTRAIN(fReso, 1.f, 20.f)
    fCut = 20.f * stmlib::SemitonesToRatio(fCut * 120.f);
    svf.set_f_q<stmlib::FREQUENCY_FAST>(fCut / 44100.f, fReso);
    MK_INT_PAR_ABS(iFType, fmode, 4.f)
    CONSTRAIN(iFType, 0, 3);

    // calculate modulation params
    float fAM = valADSR * fEGAM; // adsr
    if (fEGAM < 0.f) fAM -= fEGAM; // adsr
    fAM = ((1.f - fabsf(fEGAM)) + fAM); // adsr
    fAM *= (1.f - (valLFO + 1.f) * 0.5f * fLFOAM); // lfo
    fAM *= fGain * fGain; // gain (quadratic)
    CONSTRAIN(fAM, 0.f, 1.f)

    float fWt = fWave + valADSR * fEGWave + valLFO * fLFOWave * 2.f;
    CONSTRAIN(fWt, 0.f, 1.f)

    // detect very fast modulations and filter wave for respective frame
    // float deltaWt = fabsf(pre_fWt - fWt);
    // if(deltaWt > 0.1f){
    //     trigger = true;
    // }
    // pre_fWt = fWt;

    // calc wave and apply filter
    if(isWaveTableGood){
        oscillator.Render(trigger, f0, fAM, fWt, wavetables, out, BUF_SZ);

        switch(iFType){
            case 1:
                svf.Process<stmlib::FILTER_MODE_LOW_PASS>(out, out, BUF_SZ);
                break;
            case 2:
                svf.Process<stmlib::FILTER_MODE_BAND_PASS>(out, out, BUF_SZ);
                break;
            case 3:
                svf.Process<stmlib::FILTER_MODE_HIGH_PASS>(out, out, BUF_SZ);
            default:
                break;
        }
    }
}

void DrumRackWTOsc::prepareWavetables(HELPERS::ctagSampleRom *sampleRom) {
    ESP_LOGI("DrumRackWTOsc", "prepareWavetables bank=%d\n", currentBank);

    // precalculates wavetable data according to https://www.dafx12.york.ac.uk/papers/dafx12_submission_69.pdf
    // plaits uses integrated wavetable synthesis, i.e. integrated wavetables, order K=1 (one integration), N=1 (linear interpolation)
    // check if sample rom seems to have current bank
    if(!sampleRom->HasSliceGroup(currentBank * 64, currentBank * 64 + 63)){
        isWaveTableGood = false;
        return;
    }
    int size = sampleRom->GetSliceGroupSize(currentBank * 64, currentBank * 64 + 63);
    if(size != 256*64){
        isWaveTableGood = false;
        return;
    }
    int bankOffset = currentBank*64*256;
    int bufferOffset = 4*64; // load sample data into buffer at offset, due to pre-calculation each wave will be 260 words long
    sampleRom->Read(&buffer[bufferOffset], bankOffset, 256*64);
    // start conversion of data
    // 64 wavetables per bank
    int c = 0;
    for(int i=0;i<64;i++){ // iterate all waves
        int startOffset = bufferOffset + i*256; // which wave
        // prepare long array, i.e. x = numpy.array(list(wave) * 2 + wave[0] + wave[1] + wave[2] + wave[3])
        float sum4 = buffer[startOffset] + buffer[startOffset+1] + buffer[startOffset+2] + buffer[startOffset+3]; // add dc
        for(int j=0;j<512;j++){
            fbuffer[j] = buffer[startOffset + (j%256)] + sum4;
        }
        // x -= x.mean()
        removeMeanOfFloatArray(fbuffer, 512);
        // x /= numpy.abs(x).max()
        scaleFloatArrayToAbsMax(fbuffer, 512);
        // x = numpy.cumsum(x)
        accumulateFloatArray(fbuffer, 512);
        // x -= x.mean()
        removeMeanOfFloatArray(fbuffer, 512);
        // create pointer map
        wavetables[i] = &buffer[c];
        // x = numpy.round(x * (4 * 32768.0 / WAVETABLE_SIZE)
        for(int j=512-256-4;j<512;j++){
            int16_t v = static_cast<int16_t >(roundf(fbuffer[j] * 4.f * 32768.f / 256.f));
            buffer[c++] = v;
        }
    }
    isWaveTableGood = true;
}
