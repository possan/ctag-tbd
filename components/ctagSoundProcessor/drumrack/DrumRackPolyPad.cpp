#include "DrumRackSynth.hpp"
#include "DrumRackPolyPad.hpp"
#include "../ctagSoundProcessorDrumRack.hpp"
#include "braids/quantizer_scales.h"

using namespace CTAG::SP;

void DrumRackPolyPad::Init(const DrumRackInitData *initdata) {
    // uint8_t *privatedata = initdata->allocator(1000);

    for(auto &s:pp_v_voices){
        s.Reset();
    }
    pp_quantizer.Init();

    initdata->rack->registerParam(initdata->prefix, "pitch", [&](const int val) { pp_pitch = val; });
    initdata->rack->registerCv(initdata->prefix, "pitch", [&](const int val) { cv_pp_pitch = val; });
    initdata->rack->registerParam(initdata->prefix, "q_scale", [&](const int val) { pp_q_scale = val; });
    initdata->rack->registerCv(initdata->prefix, "q_scale", [&](const int val) { cv_pp_q_scale = val; });
    initdata->rack->registerParam(initdata->prefix, "chord", [&](const int val) { pp_chord = val; });
    initdata->rack->registerCv(initdata->prefix, "chord", [&](const int val) { cv_pp_chord = val; });
    initdata->rack->registerParam(initdata->prefix, "inversion", [&](const int val) { pp_inversion = val; });
    initdata->rack->registerCv(initdata->prefix, "inversion", [&](const int val) { cv_pp_inversion = val; });
    initdata->rack->registerParam(initdata->prefix, "detune", [&](const int val) { pp_detune = val; });
    initdata->rack->registerCv(initdata->prefix, "detune", [&](const int val) { cv_pp_detune = val; });
    initdata->rack->registerParam(initdata->prefix, "nnotes", [&](const int val) { pp_nnotes = val; });
    initdata->rack->registerCv(initdata->prefix, "nnotes", [&](const int val) { cv_pp_nnotes = val; });
    initdata->rack->registerParam(initdata->prefix, "ncvoices", [&](const int val) { pp_ncvoices = val; });
    initdata->rack->registerCv(initdata->prefix, "ncvoices", [&](const int val) { cv_pp_ncvoices = val; });
    initdata->rack->registerParam(initdata->prefix, "voicehold", [&](const int val) { pp_voicehold = val; });
    initdata->rack->registerTrig(initdata->prefix, "voicehold", [&](const int val) { trig_pp_voicehold = val; });
    initdata->rack->registerParam(initdata->prefix, "lfo1_freq", [&](const int val) { pp_lfo1_freq = val; });
    initdata->rack->registerCv(initdata->prefix, "lfo1_freq", [&](const int val) { cv_pp_lfo1_freq = val; });
    initdata->rack->registerParam(initdata->prefix, "lfo1_amt", [&](const int val) { pp_lfo1_amt = val; });
    initdata->rack->registerCv(initdata->prefix, "lfo1_amt", [&](const int val) { cv_pp_lfo1_amt = val; });
    initdata->rack->registerParam(initdata->prefix, "filter_type", [&](const int val) { pp_filter_type = val; });
    initdata->rack->registerCv(initdata->prefix, "filter_type", [&](const int val) { cv_pp_filter_type = val; });
    initdata->rack->registerParam(initdata->prefix, "cutoff", [&](const int val) { pp_cutoff = val; });
    initdata->rack->registerCv(initdata->prefix, "cutoff", [&](const int val) { cv_pp_cutoff = val; });
    initdata->rack->registerParam(initdata->prefix, "resonance", [&](const int val) { pp_resonance = val; });
    initdata->rack->registerCv(initdata->prefix, "resonance", [&](const int val) { cv_pp_resonance = val; });
    initdata->rack->registerParam(initdata->prefix, "lfo2_freq", [&](const int val) { pp_lfo2_freq = val; });
    initdata->rack->registerCv(initdata->prefix, "lfo2_freq", [&](const int val) { cv_pp_lfo2_freq = val; });
    initdata->rack->registerParam(initdata->prefix, "lfo2_amt", [&](const int val) { pp_lfo2_amt = val; });
    initdata->rack->registerCv(initdata->prefix, "lfo2_amt", [&](const int val) { cv_pp_lfo2_amt = val; });
    initdata->rack->registerParam(initdata->prefix, "lfo2_rphase", [&](const int val) { pp_lfo2_rphase = val; });
    initdata->rack->registerTrig(initdata->prefix, "lfo2_rphase", [&](const int val) { trig_pp_lfo2_rphase = val; });
    initdata->rack->registerParam(initdata->prefix, "eg_filt_amt", [&](const int val) { pp_eg_filt_amt = val; });
    initdata->rack->registerCv(initdata->prefix, "eg_filt_amt", [&](const int val) { cv_pp_eg_filt_amt = val; });
    initdata->rack->registerParam(initdata->prefix, "enableEG", [&](const int val) { pp_enableEG = val; });
    initdata->rack->registerTrig(initdata->prefix, "enableEG", [&](const int val) { trig_pp_enableEG = val; });
    initdata->rack->registerParam(initdata->prefix, "latchEG", [&](const int val) { pp_latchEG = val; });
    initdata->rack->registerTrig(initdata->prefix, "latchEG", [&](const int val) { trig_pp_latchEG = val; });
    initdata->rack->registerParam(initdata->prefix, "eg_slow_fast", [&](const int val) { pp_eg_slow_fast = val; });
    initdata->rack->registerTrig(initdata->prefix, "eg_slow_fast", [&](const int val) { trig_pp_eg_slow_fast = val; });
    initdata->rack->registerParam(initdata->prefix, "attack", [&](const int val) { pp_attack = val; });
    initdata->rack->registerCv(initdata->prefix, "attack", [&](const int val) { cv_pp_attack = val; });
    initdata->rack->registerParam(initdata->prefix, "decay", [&](const int val) { pp_decay = val; });
    initdata->rack->registerCv(initdata->prefix, "decay", [&](const int val) { cv_pp_decay = val; });
    initdata->rack->registerParam(initdata->prefix, "sustain", [&](const int val) { pp_sustain = val; });
    initdata->rack->registerCv(initdata->prefix, "sustain", [&](const int val) { cv_pp_sustain = val; });
    initdata->rack->registerParam(initdata->prefix, "release", [&](const int val) { pp_release = val; });
    initdata->rack->registerCv(initdata->prefix, "release", [&](const int val) { cv_pp_release = val; });

    this->enabled = false;
    pp_preNCVoices = 99;
};

void DrumRackPolyPad::Process(const DrumRackProcessData &data) {
    // std::fill_n(pp_out, BUF_SZ, 0.f);
    std::fill_n(pp_out_stereo, BUF_SZ * 2, 0.f);

    if (!this->enabled) {
        return;
    }

    int32_t NCVoices = pp_ncvoices;
    CONSTRAIN(NCVoices, 1, 8)
    if(pp_preNCVoices != NCVoices){
        for(auto &s:pp_v_voices){
            s.Reset();
        }
        pp_preNCVoices = NCVoices;
    }

    // start chord
    bool shouldTrigger = pp_enableEG;
    if (trig_pp_enableEG != -1) shouldTrigger = data.trig[trig_pp_enableEG] == 1 ? 0 : 1; // inverted logic
    if (shouldTrigger != trig_prev) {
        if (shouldTrigger) {
            printf("PP1\n");
        }
        trig_prev = shouldTrigger;
    } else {
        shouldTrigger = false;
    }

    // if (pp_latchEG) {
    //     if (!pp_toggle && shouldTrigger) {
    //         pp_latched = !pp_latched;
    //         pp_toggle = true;
    //     } else if (!shouldTrigger) {
    //         pp_toggle = false;
    //     }
    //     if (pp_latched && shouldTrigger) {
    //         shouldTrigger = false;
    //     }
    // } else {
    //     pp_latched = true;
    //     pp_toggle = false;
    // }
    // shouldTrigger = shouldTrigger && (pp_latchVoice == false);
    // start processing voices

    if (shouldTrigger) {
        printf("PP2\n");
        // check if voice needs to be killed because too many are active

        // sort array according to voice time to live, last in array has shortest TTL
        sort(begin(pp_v_voices), end(pp_v_voices) - (8-NCVoices),
             [](ChordSynth &a, ChordSynth &b) { return a.GetTTL() > b.GetTTL(); }
        );

        // hold voices
        bool shouldHold = pp_voicehold;
        if (trig_pp_voicehold != -1) { shouldHold = data.trig[trig_pp_voicehold] == 0 ? 1 : 0; } // inverted logic
        if (shouldHold) {
            for (int i=0;i<NCVoices-1;i++) {
                if(!pp_v_voices[i].IsDead())
                    pp_v_voices[i].Hold();
            }
        }

        // start new voice with current parameter settings including cv mod capture
        ChordSynth::ChordParams params;

        // pitch calculation and quantization + fm
        params.pitch = pp_pitch;
        if (cv_pp_pitch != -1) { params.pitch += static_cast<int16_t>(data.cv[cv_pp_pitch] * 5.f * 12.f * 128.f); }
        int32_t sc = pp_q_scale;
        if (cv_pp_q_scale != -1) {
            sc = static_cast<int32_t>(fabsf(data.cv[cv_pp_q_scale]) * 48.f);
        }
        CONSTRAIN(sc, 0, 47);
        pp_quantizer.Configure(braids::scales[sc]);
        params.pitch = pp_quantizer.Process(params.pitch, pp_pitch);
        CONSTRAIN(params.pitch, 0, 16383);

        // which chord
        params.chord = pp_chord;
        if (cv_pp_chord != -1) { params.chord = static_cast<int16_t>(fabsf(data.cv[cv_pp_chord]) * kChordNumChords); }
        CONSTRAIN(params.chord, 0, kChordNumChords - 1)
        params.nnotes = pp_nnotes;
        if (cv_pp_nnotes != -1) { params.nnotes = static_cast<int16_t>(fabsf(data.cv[cv_pp_nnotes]) * 4.f) + 1; }
        CONSTRAIN(params.nnotes, 1, 4)

        params.detune = pp_detune;
        if (cv_pp_detune != -1) { params.detune = static_cast<int16_t>(fabsf(data.cv[cv_pp_detune]) * 32767.f); }
        CONSTRAIN(params.detune, 0, 32767)
        params.inversion = pp_inversion;
        if (cv_pp_inversion != -1) { params.inversion = static_cast<int16_t>(fabsf(data.cv[cv_pp_inversion]) * 6.f - 3.f); }
        CONSTRAIN(params.inversion, -2, 2)
        float maxA, maxD, maxR;
        if (pp_eg_slow_fast) {
            maxA = 60.f;
            maxD = 40.f;
            maxR = 40.f;
        } else {
            maxA = 10.f;
            maxD = 10.f;
            maxR = 10.f;
        }
        params.attack = static_cast<float>(pp_attack) / 4095.f * maxA;
        if (cv_pp_attack != -1) { params.attack = fabsf(data.cv[cv_pp_attack]) * maxA; }
        CONSTRAIN(params.attack, 0.f, maxA)
        params.decay = static_cast<float>(pp_decay) / 4095.f * maxD;
        if (cv_pp_decay != -1) { params.decay = fabsf(data.cv[cv_pp_decay]) * maxD; }
        CONSTRAIN(params.decay, 0.f, maxD)
        params.sustain = static_cast<float>(pp_sustain) / 4095.f;
        if (cv_pp_sustain != -1) { params.sustain = fabsf(data.cv[cv_pp_sustain]); }
        CONSTRAIN(params.sustain, 0.f, 1.f)
        params.release = static_cast<float>(pp_release) / 4095.f * maxR;
        if (cv_pp_release != -1) { params.release = fabsf(data.cv[cv_pp_release]) * maxR; }
        CONSTRAIN(params.release, 0.f, maxR)

        // vibrato
        params.lfo1_freq = static_cast<float>(pp_lfo1_freq) / 4095.f * 5.f;
        if (cv_pp_lfo1_freq != -1) { params.lfo1_freq = fabsf(data.cv[cv_pp_lfo1_freq]) * 5.f; }
        CONSTRAIN(params.lfo1_freq, 0.f, 5.f)
        params.lfo1_amt = static_cast<float>(pp_lfo1_amt) / 4095.f * 5.f;
        if (cv_pp_lfo1_amt != -1) { params.lfo1_amt = fabsf(data.cv[cv_pp_lfo1_amt]) * 5.f; }
        CONSTRAIN(params.lfo1_amt, 0.f, 5.f)

        // filter fm chopper
        params.lfo2_freq = static_cast<float>(pp_lfo2_freq) / 4095.f * 5.f;
        if (cv_pp_lfo2_freq != -1) { params.lfo2_freq = fabsf(data.cv[cv_pp_lfo2_freq]) * 5.f; }
        CONSTRAIN(params.lfo2_freq, 0.f, 5.f)
        params.lfo2_amt = static_cast<float>(pp_lfo2_amt) / 4095.f;
        if (cv_pp_lfo2_amt != -1) { params.lfo2_amt = fabsf(data.cv[cv_pp_lfo2_amt]); }
        CONSTRAIN(params.lfo2_amt, 0.f, 1.f)
        params.lfo2_random_phase = pp_lfo2_rphase;
        params.eg_filt_amt = static_cast<float>(pp_eg_filt_amt) / 4095.f;
        if (cv_pp_eg_filt_amt != -1) { params.eg_filt_amt = data.cv[cv_pp_eg_filt_amt]; }
        CONSTRAIN(params.eg_filt_amt, -1.f, 1.f)
        params.filter_type = pp_filter_type;
        if (cv_pp_filter_type != -1) { params.filter_type = fabsf(data.cv[cv_pp_filter_type]) * 3.f; }
        CONSTRAIN(params.filter_type, 0, 2)

        // find a silent voice and activate
        for(int i=0;i<NCVoices;i++){
            // find a dead voice
            if(pp_v_voices[i].IsDead()){
                pp_v_voices[i].Init(params);
                break;
            }
            // if none found, activate the last one
            if(i == NCVoices-1){
                pp_v_voices[i].Init(params);
            }
        }

        pp_latchVoice = true;
    }

    // render buffers with updated cutoff, resonance and detune
    uint32_t c = pp_cutoff;
    if (cv_pp_cutoff != -1) {
        c = static_cast<int32_t>(1750.f + fabsf(data.cv[cv_pp_cutoff]) * (16384.f - 1750.f));
        CONSTRAIN(c, 1750, 16384)
    }
    int32_t r = pp_resonance;
    if (cv_pp_resonance != -1) {
        r = static_cast<int32_t>(fabsf(data.cv[cv_pp_resonance]) * 32767.f);
        CONSTRAIN(r, 0, 32767)
    }
    int32_t d = pp_detune;
    if (cv_pp_detune != -1) {
        d = static_cast<int32_t>(fabsf(data.cv[cv_pp_detune]) * 32767.f);
        CONSTRAIN(d, 0, 32767)
    }

    for (int i=0;i<NCVoices;i++) {
        if(pp_v_voices[i].IsDead()) continue;
        pp_v_voices[i].SetCutoff(c);
        pp_v_voices[i].SetResonance(r);
        pp_v_voices[i].SetDetune(d);
        pp_v_voices[i].Process(pp_out_stereo, 0);
    }

    // it isn't stereo after all...
    for(int i=0;i<BUF_SZ;i++) {
        pp_out_stereo[i*2+1] = pp_out_stereo[i*2+0];
    }

    // note off including latched mode
    bool shouldNoteOff = !pp_enableEG;
    if (trig_pp_enableEG != -1) shouldNoteOff = data.trig[trig_pp_enableEG]; // already inverted
    if (pp_latchEG) {
        shouldNoteOff = !shouldNoteOff;
        if (!pp_latched && shouldNoteOff)
            shouldNoteOff = false;
    }
    shouldNoteOff = shouldNoteOff && (pp_latchVoice == true);

    if (shouldNoteOff) {
        for (auto &v:pp_v_voices) {
            v.NoteOff();
        }
        pp_latchVoice = false;
    }

    if (pp_out_stereo[0] != pp_out_stereo[0]) {
        printf("DrumRackPolyPad: NaN detected!\n");
        // hh2.Init();
        for (auto &v:pp_v_voices) {
            v.NoteOff();
        }
    }
}
