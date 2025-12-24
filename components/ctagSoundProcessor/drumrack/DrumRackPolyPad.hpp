#pragma once
#include "DrumRackSynth.hpp"
#include "plaits/dsp/drums/analog_bass_drum.h"
#include "plaits/dsp/drums/analog_snare_drum.h"
#include "plaits/dsp/drums/synthetic_bass_drum.h"
#include "plaits/dsp/drums/synthetic_snare_drum.h"
#include "plaits/dsp/drums/hi_hat.h"
#include "braids/analog_oscillator.h"
#include "braids/signature_waveshaper.h"
#include "braids/macro_oscillator.h"
#include "braids/settings.h"
#include "braids/quantizer.h"
#include "filters/ctagDiodeLadderFilter.hpp"
#include "filters/ctagDiodeLadderFilter2.hpp"
#include "filters/ctagDiodeLadderFilter3.hpp"
#include "filters/ctagDiodeLadderFilter4.hpp"
#include "filters/ctagDiodeLadderFilter5.hpp"
#include "filters/ctagFilterBase.hpp"
#include "synthesis/RomplerVoiceMinimal.hpp"
#include "synthesis/Clap.hpp"
#include "synthesis/Rimshot.hpp"
#include "synthesis/FmKick.hpp"
#include "helpers/ctagSampleRom.hpp"
#include "helpers/ctagADEnv.hpp"
#include "SimpleComp/SimpleComp.h"
#include "mifx/reverb.h"
#include "polypad/ChordSynth.hpp"

using namespace CTAG::SP;

class DrumRackPolyPad {
public:
    void Process(const PicoSeqRackProcessData &data);
    void Init(const PickSeqRackInitData *initdata);
	bool enabled;
    // float pp_out[BUF_SZ];
    float pp_out_stereo[BUF_SZ * 2];

	void handleMidiNoteOn(uint8_t note, uint8_t vel);
	void handleMidiNoteOff(uint8_t note, uint8_t vel);
	// void handleMidiCC(uint8_t control, uint8_t value);

private:
    bool pp_trig_prev {false};
	array<ChordSynth, 1> pp_v_voices;
	bool pp_latchVoice = false;
	bool pp_latched = false;
	bool pp_toggle = false;
	int32_t pp_preNCVoices = 0;
	braids::Quantizer pp_quantizer;
	bool trig_prev {false};
	float midi_freq {0.0f};
	int midi_note {0};
	bool midi_trig {false};

	// atomic<int16_t> pp_pitch; //, cv_pp_pitch;
	atomic<int16_t> pp_q_scale; //, cv_pp_q_scale;
	atomic<int16_t> pp_chord; //, cv_pp_chord;
	atomic<int16_t> pp_inversion; //, cv_pp_inversion;
	atomic<int16_t> pp_detune; //, cv_pp_detune;
	atomic<int16_t> pp_nnotes; //, cv_pp_nnotes;
	// atomic<int16_t> pp_ncvoices; //, cv_pp_ncvoices;
	atomic<int16_t> pp_voicehold; //, trig_pp_voicehold;
	atomic<int16_t> pp_lfo1_freq; //, cv_pp_lfo1_freq;
	atomic<int16_t> pp_lfo1_amt; //, cv_pp_lfo1_amt;
	atomic<int16_t> pp_filter_type; //, cv_pp_filter_type;
	atomic<int16_t> pp_cutoff; //, cv_pp_cutoff;
	atomic<int16_t> pp_resonance; //, cv_pp_resonance;
	atomic<int16_t> pp_lfo2_freq; //, cv_pp_lfo2_freq;
	atomic<int16_t> pp_lfo2_amt; //, cv_pp_lfo2_amt;
	atomic<int16_t> pp_lfo2_rphase; //, trig_pp_lfo2_rphase;
	atomic<int16_t> pp_eg_filt_amt; //, cv_pp_eg_filt_amt;
	// atomic<int16_t> pp_enableEG; //, trig_pp_enableEG;
	// atomic<int16_t> pp_latchEG; //, trig_pp_latchEG;
	// atomic<int16_t> pp_eg_slow_fast; //, trig_pp_eg_slow_fast;
	atomic<int16_t> pp_attack; //, cv_pp_attack;
	atomic<int16_t> pp_decay; //, cv_pp_decay;
	atomic<int16_t> pp_sustain; //, cv_pp_sustain;
	atomic<int16_t> pp_release; //, cv_pp_release;
};
