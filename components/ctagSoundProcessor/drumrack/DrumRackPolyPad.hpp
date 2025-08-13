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

class DrumRackPolyPad : DrumRackSynth {
public:
    void Process(const DrumRackProcessData &data) override;
    void Init(const DrumRackInitData *initdata) override;

private:
    bool pp_trig_prev {false};
	array<ChordSynth, 8> pp_v_voices;
	bool pp_latchVoice = false;
	bool pp_latched = false;
	bool pp_toggle = false;
	int32_t pp_preNCVoices = 0;
	braids::Quantizer pp_quantizer;

	atomic<int32_t> pp_pitch, cv_pp_pitch;
	atomic<int32_t> pp_q_scale, cv_pp_q_scale;
	atomic<int32_t> pp_chord, cv_pp_chord;
	atomic<int32_t> pp_inversion, cv_pp_inversion;
	atomic<int32_t> pp_detune, cv_pp_detune;
	atomic<int32_t> pp_nnotes, cv_pp_nnotes;
	atomic<int32_t> pp_ncvoices, cv_pp_ncvoices;
	atomic<int32_t> pp_voicehold, trig_pp_voicehold;
	atomic<int32_t> pp_lfo1_freq, cv_pp_lfo1_freq;
	atomic<int32_t> pp_lfo1_amt, cv_pp_lfo1_amt;
	atomic<int32_t> pp_filter_type, cv_pp_filter_type;
	atomic<int32_t> pp_cutoff, cv_pp_cutoff;
	atomic<int32_t> pp_resonance, cv_pp_resonance;
	atomic<int32_t> pp_lfo2_freq, cv_pp_lfo2_freq;
	atomic<int32_t> pp_lfo2_amt, cv_pp_lfo2_amt;
	atomic<int32_t> pp_lfo2_rphase, trig_pp_lfo2_rphase;
	atomic<int32_t> pp_eg_filt_amt, cv_pp_eg_filt_amt;
	atomic<int32_t> pp_enableEG, trig_pp_enableEG;
	atomic<int32_t> pp_latchEG, trig_pp_latchEG;
	atomic<int32_t> pp_eg_slow_fast, trig_pp_eg_slow_fast;
	atomic<int32_t> pp_attack, cv_pp_attack;
	atomic<int32_t> pp_decay, cv_pp_decay;
	atomic<int32_t> pp_sustain, cv_pp_sustain;
	atomic<int32_t> pp_release, cv_pp_release;
	atomic<int32_t> pp_lev, cv_pp_lev;
	atomic<int32_t> pp_pan, cv_pp_pan;
	atomic<int32_t> pp_fx1, cv_pp_fx1;
	atomic<int32_t> pp_fx2, cv_pp_fx2;
};
