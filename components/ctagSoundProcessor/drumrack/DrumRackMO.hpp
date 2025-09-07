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

class DrumRackMO {
public:
    void Process(const DrumRackProcessData &data);
    void Init(const DrumRackInitData *initdata);
	bool enabled;
    float mo_out[BUF_SZ];

	void handleMidiNoteOn(uint8_t note, uint8_t vel);
	void handleMidiNoteOff(uint8_t note, uint8_t vel);
	// void handleMidiCC(uint8_t control, uint8_t value);

private:
	braids::MacroOscillator mo_osc;
	braids::SignatureWaveshaper mo_ws;
	braids::Quantizer mo_quantizer;
	CTAG::SP::HELPERS::ctagADEnv mo_envelope;
	const uint8_t mo_sync[BUF_SZ] = {0};
	bool mo_prevTrigger = false;
	const uint16_t mo_bit_reduction_masks[7] = {
			0xc000,
			0xe000,
			0xf000,
			0xf800,
			0xff00,
			0xfff0,
			0xffff};
	float midi_freq {0.0f};
	int midi_note {0};
	bool midi_trig {false};

	atomic<int16_t> mo_shape; //, cv_mo_shape;
	atomic<int16_t> mo_pitch; //, cv_mo_pitch;	
	atomic<int16_t> mo_decimation; //, cv_mo_decimation;
	atomic<int16_t> mo_bit_reduction; //, cv_mo_bit_reduction;
	atomic<int16_t> mo_q_scale; //, cv_mo_q_scale;
	atomic<int16_t> mo_param_0; //, cv_mo_param_0;
	atomic<int16_t> mo_param_1; //, cv_mo_param_1;
	atomic<int16_t> mo_waveshaping; //, cv_mo_waveshaping;
	atomic<int16_t> mo_fm_amt; //, cv_mo_fm_amt;
	atomic<int16_t> mo_p0_amt; //, cv_mo_p0_amt;
	atomic<int16_t> mo_p1_amt; //, cv_mo_p1_amt;
	// atomic<int16_t> mo_enableEG, trig_mo_enableEG;
	atomic<int16_t> mo_loopEG, trig_mo_loopEG;
	atomic<int16_t> mo_attack; //, cv_mo_attack;
	atomic<int16_t> mo_decay; //, cv_mo_decay;
};
