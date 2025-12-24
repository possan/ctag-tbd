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

class RackDSD {
public:
    void Process(const PicoSeqRackProcessData &data);
    void Init(const PickSeqRackInitData *initdata);
	bool enabled;
	float out[BUF_SZ];
	
	void handleMidiNoteOn();
	// void handleMidiCC(uint8_t control, uint8_t value);

private:
	bool midi_trig {false};
	bool trig_prev {false};
	plaits::SyntheticSnareDrum dsd;
	// atomic<int16_t> trigger, trig_trigger;
	atomic<int16_t> accent; //, cv_accent;
	atomic<int16_t> f0; //, cv_f0;
	atomic<int16_t> fm_amt; //, cv_fm_amt;
	atomic<int16_t> decay; //, cv_decay;
	atomic<int16_t> spy; //, cv_spy;
};
