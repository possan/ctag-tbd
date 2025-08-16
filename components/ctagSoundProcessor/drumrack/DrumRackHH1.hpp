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

class DrumRackHH1 {
public:
    void Process(const DrumRackProcessData &data);
    void Init(const DrumRackInitData *initdata);
	bool enabled;
	float hh1_out[BUF_SZ];

private:
	plaits::HiHat<plaits::SquareNoise, plaits::SwingVCA, true, false> hh1;
	bool hh1_trig_prev {false};
	float temp1_[BUF_SZ];
	float temp2_[BUF_SZ];
	atomic<int16_t> hh1_trigger, trig_hh1_trigger;
	atomic<int16_t> hh1_accent, cv_hh1_accent;
	atomic<int16_t> hh1_f0, cv_hh1_f0;
	atomic<int16_t> hh1_tone, cv_hh1_tone;
	atomic<int16_t> hh1_decay, cv_hh1_decay;
	atomic<int16_t> hh1_noise, cv_hh1_noise;
};
