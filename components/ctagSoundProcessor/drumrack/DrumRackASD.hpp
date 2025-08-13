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

class DrumRackASD : DrumRackSynth {
public:
    void Process(const DrumRackProcessData &data) override;
    void Init(const DrumRackInitData *initdata) override;

private:
	plaits::AnalogSnareDrum asd;
	float asd_out[32];
	bool asd_trig_prev {false};
	atomic<int32_t> as_trigger, trig_as_trigger;
	atomic<int32_t> as_mute, trig_as_mute;
	atomic<int32_t> as_lev, cv_as_lev;
	atomic<int32_t> as_pan, cv_as_pan;
	atomic<int32_t> as_fx1, cv_as_fx1;
	atomic<int32_t> as_fx2, cv_as_fx2;
	atomic<int32_t> as_accent, cv_as_accent;
	atomic<int32_t> as_f0, cv_as_f0;
	atomic<int32_t> as_tone, cv_as_tone;
	atomic<int32_t> as_decay, cv_as_decay;
	atomic<int32_t> as_a_spy, cv_as_a_spy;
};
