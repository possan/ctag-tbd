#pragma once

#include "RackSynth.hpp"
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

class RackRompler {
public:
    void Process(const PicoSeqRackProcessData &data);
    void Init(const PickSeqRackInitData *initdata);
	bool enabled;
    float s1_out[BUF_SZ];

	void handleMidiNoteOn(uint8_t note, uint8_t vel);
	void handleMidiNoteOff(uint8_t note, uint8_t vel);
	// void handleMidiCC(uint8_t control, uint8_t value);

private:
	CTAG::SYNTHESIS::RomplerVoiceMinimal rompler;
	bool trig_prev {false};
	float midi_freq {0.0f};
	int midi_note {0};
	bool midi_trig {false};
	atomic<int16_t> s1_speed; //, cv_s1_speed;
	atomic<int16_t> s1_pitch; //, cv_s1_pitch;
	atomic<int16_t> s1_bank; //, cv_s1_bank;
	atomic<int16_t> s1_slice; //, cv_s1_slice;
	atomic<int16_t> s1_start; //, cv_s1_start;
	atomic<int16_t> s1_end; //, cv_s1_end;
	atomic<int16_t> s1_lp; //, trig_s1_lp;
	atomic<int16_t> s1_lp_pp; //, trig_s1_lp_pp;
	atomic<int16_t> s1_lp_pos; //, cv_s1_lp_pos;
	atomic<int16_t> s1_atk; //, cv_s1_atk;
	atomic<int16_t> s1_dcy; //, cv_s1_dcy;
	atomic<int16_t> s1_eg2fm; //, cv_s1_eg2fm;
	atomic<int16_t> s1_brr; //, cv_s1_brr;
	atomic<int16_t> s1_ft; //, cv_s1_ft;
	atomic<int16_t> s1_fc; //, cv_s1_fc;
	atomic<int16_t> s1_fq; //, cv_s1_fq;
	atomic<int16_t> s1_tsmode;
	atomic<int16_t> s1_tsamount;
	atomic<int16_t> s1_tssteps;
};
