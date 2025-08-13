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

class DrumRackHH2 : DrumRackSynth {
public:
    void Process(const DrumRackProcessData &data) override;
    void Init(const DrumRackInitData *initdata) override;

private:
	plaits::HiHat<plaits::RingModNoise, plaits::LinearVCA, false, true> hh2;
	bool hh2_trig_prev {false};
	float hh2_out[32];
	float temp1_[32];
	float temp2_[32];
	atomic<int16_t> hh2_trigger, trig_hh2_trigger;
	atomic<int16_t> hh2_mute, trig_hh2_mute;
	atomic<int16_t> hh2_lev, cv_hh2_lev;
	atomic<int16_t> hh2_pan, cv_hh2_pan;
	atomic<int16_t> hh2_fx1, cv_hh2_fx1;
	atomic<int16_t> hh2_fx2, cv_hh2_fx2;
	atomic<int16_t> hh2_accent, cv_hh2_accent;
	atomic<int16_t> hh2_f0, cv_hh2_f0;
	atomic<int16_t> hh2_tone, cv_hh2_tone;
	atomic<int16_t> hh2_decay, cv_hh2_decay;
	atomic<int16_t> hh2_noise, cv_hh2_noise;
};
