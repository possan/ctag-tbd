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

class DrumRackHH2 {
public:
    void Process(const DrumRackProcessData &data);
    void Init(const DrumRackInitData *initdata);
	bool enabled;
	float out[BUF_SZ];

private:
	plaits::HiHat<plaits::RingModNoise, plaits::LinearVCA, false, true> hh2;
	bool trig_prev {false};
	float temp1[BUF_SZ];
	float temp2[BUF_SZ];
	atomic<int16_t> trigger, trig_trigger;
	atomic<int16_t> accent, cv_accent;
	atomic<int16_t> f0, cv_f0;
	atomic<int16_t> tone, cv_tone;
	atomic<int16_t> decay, cv_decay;
	atomic<int16_t> noise, cv_noise;
};
