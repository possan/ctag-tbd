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

class DrumRackRimshot : DrumRackSynth {
public:
    void Process(const DrumRackProcessData &data) override;
    void Init(const DrumRackInitData *initdata) override;

private:
	CTAG::SYNTHESIS::Rimshot rs;
	float rs_out[32];
	bool rs_trig_prev {false};
	atomic<int32_t> rs_trigger, trig_rs_trigger;
	atomic<int32_t> rs_mute, trig_rs_mute;
	atomic<int32_t> rs_lev, cv_rs_lev;
	atomic<int32_t> rs_pan, cv_rs_pan;
	atomic<int32_t> rs_fx1, cv_rs_fx1;
	atomic<int32_t> rs_fx2, cv_rs_fx2;
	atomic<int32_t> rs_accent, cv_rs_accent;
	atomic<int32_t> rs_f0, cv_rs_f0;
	atomic<int32_t> rs_tone, cv_rs_tone;
	atomic<int32_t> rs_decay, cv_rs_decay;
	atomic<int32_t> rs_noise, cv_rs_noise;
};
