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
#include "plaits/dsp/oscillator/wavetable_oscillator.h"

using namespace CTAG::SP;

class DrumRackWTOsc {
public:
    void Process(const DrumRackProcessData &data);
    void Init(const DrumRackInitData *initdata);
	bool enabled;
    float out[BUF_SZ];

	void handleMidiNoteOn(uint8_t note, uint8_t vel);
	void handleMidiNoteOff(uint8_t note, uint8_t vel);
	// void handleMidiCC(uint8_t control, uint8_t value);

private:
	void prepareWavetables(HELPERS::ctagSampleRom *samplerom);
	plaits::WavetableOscillator<256, 64> oscillator;
	ctagSineSource lfo;
	ctagADSREnv adsr;
	stmlib::Svf svf;
	int16_t *buffer = NULL;
	float *fbuffer = NULL;
	float fWave = 0.f;
	const int16_t *wavetables[64];
	int currentBank = 0;
	int lastBank = -1;
	bool isWaveTableGood = false;
	float valADSR = 0.f, valLFO = 0.f;
	bool preGate = false;
	braids::Quantizer pitchQuantizer;
	float pre_fWt = 0.f;
	float midi_freq {0.0f};
	bool midi_trig {false};

	atomic<int32_t> gain, cv_gain;
	// atomic<int32_t> gate, trig_gate;
	atomic<int32_t> pitch, cv_pitch;
	atomic<int32_t> q_scale, cv_q_scale;
	atomic<int32_t> tune, cv_tune;
	atomic<int32_t> wavebank, cv_wavebank;
	atomic<int32_t> wave, cv_wave;
	atomic<int32_t> fmode, cv_fmode;
	atomic<int32_t> fcut, cv_fcut;
	atomic<int32_t> freso, cv_freso;
	atomic<int32_t> lfo2wave, cv_lfo2wave;
	atomic<int32_t> lfo2am, cv_lfo2am;
	atomic<int32_t> lfo2fm, cv_lfo2fm;
	atomic<int32_t> lfo2filtfm, cv_lfo2filtfm;
	atomic<int32_t> eg2wave, cv_eg2wave;
	atomic<int32_t> eg2am, cv_eg2am;
	atomic<int32_t> eg2fm, cv_eg2fm;
	atomic<int32_t> eg2filtfm, cv_eg2filtfm;
	atomic<int32_t> lfospeed, cv_lfospeed;
	atomic<int32_t> lfosync, trig_lfosync;
	atomic<int32_t> egfasl, trig_egfasl;
	atomic<int32_t> attack, cv_attack;
	atomic<int32_t> decay, cv_decay;
	atomic<int32_t> sustain, cv_sustain;
	atomic<int32_t> release, cv_release;
};
